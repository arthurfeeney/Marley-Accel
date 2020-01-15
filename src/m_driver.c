#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#include <libusb-1.0/libusb.h>
#include <linux/uinput.h>
#include <linux/input-event-codes.h>

#include "m_driver.h"
#include "loading_util.h"
#include "m_accel.h"

static void intrmsg(const unsigned char *buf, int len) {
  for(int i = 0; i < len; ++i) {
    printf("%X ", buf[i]);
  }
  printf("\n");
}

int accel_driver(int fd, mouse_dev_t *dev, accel_settings_t *as) {
  /*
   * The actual acceleration driver.
   * Gets interrupt, applies acceleration functions to the relative change
   * in mouse position, and writes to uinput.
   */
  int err;
  int iterations = 3000;
  unsigned char mouse_interrupt_buf[6];
  int actual_interrupt_length;
  while(iterations--) {
    err = libusb_interrupt_transfer(dev->usb_handle, dev->endpoint_in,
                                    mouse_interrupt_buf,
                                    sizeof(mouse_interrupt_buf),
                                    &actual_interrupt_length, 0);
    if(err < 0 || actual_interrupt_length != 6) {
      return err;
    }
    //intrmsg(mouse_interrupt_buf, actual_interrupt_length);
    map_to_uinput(fd, mouse_interrupt_buf, as);
  }
  return 0;
}

void emit_intr(int fd, int type, int code, int val) {
  /*
   * emit (write) interrupt to uinput at fd
   */
  struct input_event ie = {
    .type = type,
    .code = code,
    .value = val,
    .time.tv_sec = 0,
    .time.tv_usec = 0
  };
  write(fd, &ie, sizeof(ie));
}

void map_to_uinput(int fd, unsigned char *buf, accel_settings_t *as) {
  map_key_to_uinput(fd, buf);
  map_scroll_to_uinput(fd, buf);
  map_move_to_uinput(fd, buf, as);
}

void map_scroll_to_uinput(int fd, unsigned char *buf) {
  if(buf[5] != 0) {
    emit_intr(fd, EV_REL, REL_WHEEL, (signed char) buf[5]);
    emit_intr(fd, EV_SYN, SYN_REPORT, 0);
  }
}

void map_key_to_uinput(int fd, unsigned char *buf) {
  /*
   * maps keycode to uinput.
   */
  const unsigned char key_code = buf[0];
  if(key_code == 0) {
    // release all keys
    emit_intr(fd, EV_KEY, BTN_LEFT, 0);
    emit_intr(fd, EV_KEY, BTN_RIGHT, 0);
    emit_intr(fd, EV_KEY, BTN_MIDDLE, 0);
    emit_intr(fd, EV_KEY, BTN_SIDE, 0);
    emit_intr(fd, EV_KEY, BTN_EXTRA, 0);
  }
  else if(key_code == 0x1) {
    emit_intr(fd, EV_KEY, BTN_LEFT, 1);
  }
  else if(key_code == 0x2) {
    emit_intr(fd, EV_KEY, BTN_RIGHT, 1);
  }
  else if(key_code == 0x3) {
    emit_intr(fd, EV_KEY, BTN_LEFT, 1);
    emit_intr(fd, EV_KEY, BTN_RIGHT, 1);
  }
  else if(key_code == 0x4) {
    emit_intr(fd, EV_KEY, BTN_MIDDLE, 1);
  }
  else if(key_code == 0x8) {
    // side button closer to the back
    emit_intr(fd, EV_KEY, BTN_SIDE, 1);
  }
  else if(key_code == 0x10) {
    // side button closer to the front.
    emit_intr(fd, EV_KEY, BTN_EXTRA, 1);
  }
  emit_intr(fd, EV_SYN, SYN_REPORT, 0);
}

void map_move_to_uinput(int fd, unsigned char *buf, accel_settings_t *as) {
  /*
   * Cast to signed char so that values are pos/neg. If it stays insigned, 
   * all movements are positive.
   */
  if(buf[1] && buf[1] < 0xFF) {
    emit_intr(fd, EV_REL, REL_X, as->accel((signed char) buf[1], as));
  }
  else if(buf[1] == 0xFF) {
    emit_intr(fd, EV_REL, REL_X, as->accel((signed char) buf[2], as));
  }
  if(buf[3] && buf[3] < 0xFF) {
    emit_intr(fd, EV_REL, REL_Y, as->accel((signed char) buf[3], as));
  }
  else if(buf[3] == 0xFF) {
    emit_intr(fd, EV_REL, REL_Y, as->accel((signed char) buf[4], as));
  }
  emit_intr(fd, EV_SYN, SYN_REPORT, 0);
}

