#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <libusb-1.0/libusb.h>
#include <linux/uinput.h>

#include "errmsg.h"
#include "loading_util.h"
#include "marley_map.h"
#include "mouse_accel.h"

#define CONFIG_LINE_LENGTH 1024

static void make_lowercase(char *);
static void remove_spaces(char *);
static void remove_comments(char *);
static int assign_settings(const char *, accel_settings_t *);
static void create_bindings(int);
static int initialize_device(int, uint16_t, uint16_t);

/**
 * Load users accel settings from the specified configuration file.
 * This will automatically perform some cleaning to the config file and remove
 * comments
 */
int load_config(accel_settings_t *as, const char *config_path) {
  FILE *config = fopen(config_path, "r");
  char line[CONFIG_LINE_LENGTH];
  while (fgets(line, sizeof(line), config) != NULL) {
    make_lowercase(line);
    remove_spaces(line);
    remove_comments(line);
    int err = assign_settings(line, as);
    if (err) {
      return err;
    }
  }
  // default to quake accel if nothing specified.
  if (!as->accel) {
    as->accel = quake_accel;
  }
  fclose(config);
  return 0;
}

/**
 * Device setup. This connects to usb mouse device using libusb. Detaches the
 * kernel driver and claims the device.
 */
int dev_setup(mouse_dev_t *dev) {
  int err = libusb_init(&dev->usb_ctx);
  if (err) {
    return err;
  }

  // find the device using the vendor and product ids.
  // These can be found using the "usb-devices" command in the terminal.
  dev->usb_handle = libusb_open_device_with_vid_pid(
      dev->usb_ctx, dev->vendor_id, dev->product_id);
  if (!dev->usb_handle) {
    printf("Marley-Accel: Device not found, try running with sudo.\n");
    return -1;
  }
  // after an error, on restart the driver may not be attached. So,
  // we make sure that it is attached.
  // If driver is already attached, this has no effect.
  libusb_attach_kernel_driver(dev->usb_handle, dev->interface);

  if (libusb_kernel_driver_active(dev->usb_handle, dev->interface) == 1) {
    err = libusb_detach_kernel_driver(dev->usb_handle, dev->interface);
    if (err != 0) {
      libusb_errmsg("Failed to detach kernel driver.\n", err);
      return err;
    }
    dev->usb_detached = true;
  } else {
    printf("Marley-Accel: Device does not have an attached driver.\n");
    printf("Marley-Accel: May not have been reattached after prior run.\n");
    return -1;
  }

  if ((err = libusb_claim_interface(dev->usb_handle, dev->interface)) != 0) {
    libusb_errmsg("Marley-Accel: Failed to claim device interface.\n", err);
    return err;
  }
  dev->usb_claimed = true;

  return 0;
}

/**
 * undoes dev_setup. Releases the device and reattaches the kernel driver.
 */
void dev_close(mouse_dev_t *dev) {
  if (dev->usb_handle) {
    if (dev->usb_claimed)
      libusb_release_interface(dev->usb_handle, dev->interface);
    if (dev->usb_detached)
      libusb_attach_kernel_driver(dev->usb_handle, dev->interface);
    libusb_close(dev->usb_handle);
  }
  if (dev->usb_ctx)
    libusb_exit(dev->usb_ctx);
  printf("\nMarley-Accel: Device closed\n");
}

/**
 * Creates uinput driver for the mouse with vendor_id and product_id.
 * These values are part of the mouse settings.
 */
int create_input_device(uint16_t vendor_id, uint16_t product_id) {
  int err;
  const int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (fd < 0) {
    errmsg("Failed to open uinput", fd);
    return fd;
  }
  create_bindings(fd);
  err = initialize_device(fd, vendor_id, product_id);
  if (err < 0) {
    errmsg("Failed to write uinput device description", err);
    return err;
  }
  err = ioctl(fd, UI_DEV_CREATE);
  if (err < 0) {
    errmsg("Failed to ioctl UI_DEV_CREATE", err);
    return err;
  }
  return fd;
}

void close_input_device(int fd) {
  ioctl(fd, UI_DEV_DESTROY);
  close(fd);
}

static void make_lowercase(char *line) {
  for (int idx = 0; idx < CONFIG_LINE_LENGTH && line[idx] != '\0'; ++idx) {
    line[idx] = tolower(line[idx]);
  }
}

/**
 * Remove all spaces from the line string. The config files are small. This can
 * be done efficiently enough by shifting text to cover spaces in-place.
 */
static void remove_spaces(char *line) {
  for (int idx = 0; idx < CONFIG_LINE_LENGTH && line[idx] != '\0'; ++idx) {
    // shift text over to remove space.
    if (isspace(line[idx])) {
      for (int shift = idx; shift < CONFIG_LINE_LENGTH && line[shift] != '\0';
           ++shift) {
        line[shift] = line[shift + 1];
      }
    }
  }
}

/**
 * Remove comments from a line in a config file
 * This is done by Replace '#' with end of string.
 */
static void remove_comments(char *line) {
  for (int idx = 0; idx < CONFIG_LINE_LENGTH; ++idx) {
    if (line[idx] == '#') {
      line[idx] = '\0';
    }
  }
}

marley_map *name_to_func_map() {
  marley_map *map = marley_map_alloc(4);
  marley_map_set(map, "quake", (void *)quake_accel);
  marley_map_set(map, "quake_accel", (void *)quake_accel);
  marley_map_set(map, "pow", (void *)pow_accel);
  marley_map_set(map, "pow_accel", (void *)pow_accel);
  return map;
}

static int assign_settings(const char *line, accel_settings_t *as) {
  char *eq_ptr = strchr(line, '=');
  if (eq_ptr == NULL) {
    return -1;
  }
  *eq_ptr = '\0';

  marley_map *map = name_to_func_map();
  accel_func accel = (accel_func)marley_map_lookup(map, eq_ptr + 1);
  marley_map_free(map);
  if (!accel) {
    return -1;
  }

  if (accel) {
    as->accel = accel;
  } else if (strcmp(line, "base") == 0) {
    as->base = strtof(eq_ptr + 1, NULL);
  } else if (strcmp(line, "offset") == 0) {
    as->offset = strtof(eq_ptr + 1, NULL);
  } else if (strcmp(line, "upper_bound") == 0) {
    as->upper_bound = strtof(eq_ptr + 1, NULL);
  } else if (strcmp(line, "accel_rate") == 0) {
    as->accel_rate = strtof(eq_ptr + 1, NULL);
  } else if (strcmp(line, "power") == 0) {
    as->power = strtof(eq_ptr + 1, NULL);
  } else if (strcmp(line, "game_sens") == 0) {
    as->game_sens = strtof(eq_ptr + 1, NULL);
  } else if (strcmp(line, "overflow_lim") == 0) {
    as->overflow_lim = strtof(eq_ptr + 1, NULL);
  } else if (strcmp(line, "pre_scalar_x") == 0) {
    as->pre_scalar_x = strtof(eq_ptr + 1, NULL);
  } else if (strcmp(line, "pre_scalar_y") == 0) {
    as->pre_scalar_y = strtof(eq_ptr + 1, NULL);
  } else if (strcmp(line, "post_scalar_x") == 0) {
    as->post_scalar_x = strtof(eq_ptr + 1, NULL);
  } else if (strcmp(line, "post_scalar_y") == 0) {
    as->post_scalar_y = strtof(eq_ptr + 1, NULL);
  } else {
    return -1;
  }
  return 0;
}

static void create_bindings(int fd) {
  // mouse buttons
  ioctl(fd, UI_SET_EVBIT, EV_KEY);
  ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
  ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
  ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);
  ioctl(fd, UI_SET_KEYBIT, BTN_SIDE);
  ioctl(fd, UI_SET_KEYBIT, BTN_EXTRA);

  // mouse movement and scroll wheel
  ioctl(fd, UI_SET_EVBIT, EV_REL);
  ioctl(fd, UI_SET_RELBIT, REL_X);
  ioctl(fd, UI_SET_RELBIT, REL_Y);
  ioctl(fd, UI_SET_RELBIT, REL_WHEEL);
}

static int initialize_device(int fd, uint16_t vendor_id, uint16_t product_id) {
  struct uinput_user_dev uidev;
  memset(&uidev, 0x0, sizeof(uidev));
  strcpy(uidev.name, "Marley Accel Driver");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor = vendor_id;
  uidev.id.product = product_id;
  int err = ioctl(fd, UI_DEV_SETUP, &uidev);
  return err;
}
