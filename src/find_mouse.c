#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <usb.h>

#include "find_mouse.h"

#define MOUSE_INTERFACE_PROTOCOL 2

mouse_info_t find_mouse(void) {
  /*
   * Searches for a mouse and returns info on the first one that it finds.
   * It goes through all devices in all busses until it finds a mouse.
   * Documentation for device description pointers at
   * https://www.kernel.org/doc/html/v4.17/driver-api/usb/usb.html
   */
  usb_init(); // there does not seem to be a usb_exit
  usb_find_busses();
  usb_find_devices();

  for (struct usb_bus *bus = usb_busses; bus; bus = bus->next) {
    for (struct usb_device *dev = bus->devices; dev; dev = dev->next) {
      const struct usb_interface *interface = dev->config->interface;
      const struct usb_interface_descriptor *uid = interface->altsetting;
      if (uid->bInterfaceProtocol == MOUSE_INTERFACE_PROTOCOL) {
        const mouse_info_t info = {.found = true,
                                   .vendor_id = dev->descriptor.idVendor,
                                   .product_id = dev->descriptor.idProduct,
                                   .endpoint_in =
                                       uid->endpoint->bEndpointAddress,
                                   .interface = uid->iInterface};
        return info;
      }
    }
  }
  const mouse_info_t fail = {.found = false};
  return fail;
}
