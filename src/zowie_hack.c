#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <libusb-1.0/libusb.h>
#include <linux/uinput.h>

#include "loading_util.h" /* load device and uinput. mouse_dev_t */
#include "m_driver.h"     /* accel_driver */
#include "m_accel.h"      /* accel_settings_t and accel functions */
#include "find_mouse.h"   /* find_mouse */

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

  accel_settings_t as = {
    .accel = quake_accel,
    .base = 1,
    .offset = 3,
    .upper_bound = 5,
    .accel_rate = 2,
    .power = 2
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
