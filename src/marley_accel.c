#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libusb-1.0/libusb.h>
#include <linux/uinput.h>

#include "errmsg.h"
#include "find_mouse.h"
#include "loading_util.h"
#include "m_accel.h"
#include "m_driver.h"

int main(int argc, char *argv[]) {
  int err;
  char *config_path;

  // default accel settings
  accel_settings_t as = {.accel = quake_accel,
                         .overflow_lim = 10,
                         .base = 1,
                         .offset = 0,
                         .upper_bound = 8,
                         .accel_rate = 2,
                         .power = 2,
                         .game_sens = 1,
                         .carry_dx = 0,
                         .carry_dy = 0,
                         .pre_scalar_x = 1,
                         .pre_scalar_y = 1,
                         .post_scalar_x = 1,
                         .post_scalar_y = 1};

  if (argc == 2) {
    config_path = argv[1];
    printf("Loading config at %s\n", config_path);
    err = load_config(&as, config_path);
    if (err != 0) {
      errmsg("There was an error loading the config\n", err);
      return err;
    }
  } else {
    printf("You did not pass a configuration file\n");
    return 0;
  }

  printf("Accel Config Settings:\n");
  printf("  > overflow_lim=%d\n", as.overflow_lim);
  printf("  > base=%.4f\n", as.base);
  printf("  > offset=%.4f\n", as.offset);
  printf("  > upper_bound=%.4f\n", as.upper_bound);
  printf("  > accel_rate=%.4f\n", as.accel_rate);
  printf("  > power=%.4f\n", as.power);
  printf("  > game_sens=%.4f\n", as.game_sens);
  printf("  > pre_scalar_x=%.4f\n", as.pre_scalar_x);
  printf("  > pre_scalar_y=%.4f\n", as.pre_scalar_y);
  printf("  > post_scalar_x=%.4f\n", as.post_scalar_x);
  printf("  > post_scalar_y=%.4f\n", as.post_scalar_y);

  mouse_info_t mouse_info = find_mouse();
  if (!mouse_info.found) {
    printf("Marley-Accel: No USB mouse was found.\n");
    return 0;
  }

  mouse_dev_t md = {.usb_ctx = NULL,
                    .usb_handle = NULL,
                    .usb_detached = false,
                    .usb_claimed = false,
                    .vendor_id = mouse_info.vendor_id,
                    .product_id = mouse_info.product_id,
                    .endpoint_in = mouse_info.endpoint_in,
                    .interface = mouse_info.interface,
                    .buf_size = mouse_info.buf_size};

  err = dev_setup(&md);
  if (err) {
    libusb_errmsg("Error during device setup", err);
    dev_close(&md);
    return err;
  }

  int fd = create_input_device(md.vendor_id, md.product_id);

  printf("\nStop with Ctrl-c.\n");

  err = accel_driver(fd, &md, &as);
  if (err) {
    libusb_errmsg("Error during device execution", err);
    dev_close(&md);
    return err;
  }

  dev_close(&md);

  if (fd)
    close_input_device(fd);

  printf("Reattaching kernel driver.\n");

  return 0;
}
