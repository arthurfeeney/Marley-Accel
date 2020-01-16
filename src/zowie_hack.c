#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <libusb-1.0/libusb.h>
#include <linux/uinput.h>

#include "m_driver.h"     /* accel_driver */
#include "m_accel.h"      /* accel_settings_t and accel functions */
#include "find_mouse.h"   /* find_mouse */
#include "loading_util.h" /* load device and uinput. mouse_dev_t */

void libusb_errmsg(const char *str, int ret) {
  /*
   * Helper function to print libusb error messages
   */
  printf("Marley-Accel: %s [%d: %s / %s]\n",
      str, ret, 
      libusb_error_name(ret),
      libusb_strerror(ret));
}

int main(int argc, char *argv[]) {
  int err;
  char *config_path;

  accel_settings_t as = {
    .accel = quake_accel,
    .overflow_lim = 10,
    .base = 1,
    .offset = 0,
    .upper_bound = 8,
    .accel_rate = 2,
    .power = 2,
    .game_sens = 1,
    .carry_dx = 0,
    .carry_dy = 0
  };

  if(argc == 2) {
    config_path = argv[1];
    err = load_config(&as, config_path);
    if(err != 0) { printf("There was an error loading the config\n"); }
  }

  printf("base=%.4f\n", as.base);
  printf("offset=%.4f\n", as.offset);
  printf("upper_bound=%.4f\n", as.upper_bound);
  printf("accel_rate=%.4f\n", as.accel_rate);
  printf("power=%.4f\n", as.power);
  printf("game_sens=%.4f\n", as.game_sens);
  printf("overflow_lim=%d\n", as.overflow_lim);

  mouse_info_t mouse_info = find_mouse();
  if(!mouse_info.found) {
    printf("Marley-Accel: No USB mouse was found.\n");
    return 0;
  }

  mouse_dev_t md = {
    .usb_ctx = NULL,
    .usb_handle = NULL,
    .usb_detached = false,
    .usb_claimed = false,
    .vendor_id = mouse_info.vendor_id,
    .product_id = mouse_info.product_id,
    .endpoint_in = mouse_info.endpoint_in,
    .interface = mouse_info.interface
  };

  err = dev_setup(&md);
  if(err) {
    libusb_errmsg("Error during device setup", err);
    dev_close(&md);
    return err;
  }

  int fd = create_input_device(md.vendor_id, md.product_id);

  err = accel_driver(fd, &md, &as);
  if(err) {
    libusb_errmsg("Error during device execution", err);
    dev_close(&md);
    return err;
  }

  dev_close(&md);
  if(fd)
    close_input_device(fd);

  return 0;
}
