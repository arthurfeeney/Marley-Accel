#include <stdio.h>

#include <libusb-1.0/libusb.h>

void errmsg(const char *str, int ret) {
  printf("Marley-Accel: %s [%d]", str, ret);
}

void libusb_errmsg(const char *str, int ret) {
  /*
   * Helper function to print libusb error messages
   */
  printf("Marley-Accel (libusb): %s [%d: %s / %s]\n", str, ret,
         libusb_error_name(ret), libusb_strerror(ret));
}
