#ifndef LOADING_UTIL_H
#define LOADING_UTIL_H

// defined in m_accel.h
typedef struct accel_settings accel_settings_t;

typedef struct mouse_dev {
  libusb_context *usb_ctx;
  libusb_device_handle *usb_handle;
  bool usb_detached;
  bool usb_claimed;
  uint16_t vendor_id;
  uint16_t product_id;
  uint16_t endpoint_in;
  uint16_t interface;
  uint16_t buf_size;
} mouse_dev_t;

int load_config(accel_settings_t *, const char *);

/*
 * functions to manage the device with libusb
 */
int dev_setup(mouse_dev_t *dev);
void dev_close(mouse_dev_t *dev);

/*
 * Fucuntions to handle uinput for the device.
 */
int create_input_device(uint16_t, uint16_t);
void close_input_device(int);

#endif
