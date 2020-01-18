#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libusb-1.0/libusb.h>
#include <linux/input-event-codes.h>
#include <linux/uinput.h>

#include "key_codes.h"
#include "loading_util.h"
#include "m_accel.h"
#include "m_driver.h"

static void release_keys(int fd);
static void intrmsg(const unsigned char *buf, int len) {
  /*
   * only used for debugging to see that output of mouse interrupts
   * in hexadecimal
   */
  for (int i = 0; i < len; ++i) {
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

#if PRECOMP
  // precompute accel values
  precomp(as);
#endif

  int iterations = 3000;
  unsigned char mouse_interrupt_buf[6];
  int actual_interrupt_length;
  while (iterations--) {
    err = libusb_interrupt_transfer(
        dev->usb_handle, dev->endpoint_in, mouse_interrupt_buf,
        sizeof(mouse_interrupt_buf), &actual_interrupt_length, 0);
    if (err < 0 || actual_interrupt_length != 6) {
      return err;
    }
    // intrmsg(mouse_interrupt_buf, actual_interrupt_length);
    map_to_uinput(fd, mouse_interrupt_buf, as);
  }
  return 0;
}

void emit_intr(int fd, int type, int code, int val) {
  /*
   * emit (write) interrupt to uinput at fd
   */
  struct input_event ie = {.type = type,
                           .code = code,
                           .value = val,
                           .time.tv_sec = 0,
                           .time.tv_usec = 0};
  write(fd, &ie, sizeof(ie));
}

void map_to_uinput(int fd, unsigned char *buf, accel_settings_t *as) {
  map_key_to_uinput(fd, buf);
  map_scroll_to_uinput(fd, buf);
  map_move_to_uinput(fd, buf, as);
}

void map_scroll_to_uinput(int fd, unsigned char *buf) {
  if (buf[5] != 0) {
    emit_intr(fd, EV_REL, REL_WHEEL, (signed char)buf[5]);
    emit_intr(fd, EV_SYN, SYN_REPORT, 0);
  }
}

void map_key_to_uinput(int fd, unsigned char *buf) {
  /*
   * value = 1 is a press, 0 is a release
   * Some keycodes are for when multiple keys are held down.
   * These can be seen by printing with intrmsg.
   */
  const unsigned char key_code = buf[0];
  if (key_code == 0) {
    // no key is pressed, so realease them all.
    release_keys(fd);
  } else {
    const int idx = buf[0];
    const int len = key_code_map[idx][0];
    for (int i = 1; i < len + 1; ++i) {
      emit_intr(fd, EV_KEY, key_code_map[idx][i], 1);
    }
  }
  emit_intr(fd, EV_SYN, SYN_REPORT, 0);
}

void map_move_to_uinput(int fd, unsigned char *buf, accel_settings_t *as) {
  // retrieve the changes in mouse position.
  // convert to signed char (overflowed values become proper d* in negative
  // direction.) buf[2] and buf[4] contain the signs for buf[1] and buf[3].
  // They are not needed because converted to signed char gives the correct
  // negation.
  signed char dx = (signed char)buf[1];
  signed char dy = (signed char)buf[3];
  // dx and dy are updated in-place.
  accelerate(&dx, &dy, as);
  // write accelerated change to uinput
  emit_intr(fd, EV_REL, REL_X, dx);
  emit_intr(fd, EV_REL, REL_Y, dy);
  emit_intr(fd, EV_SYN, SYN_REPORT, 0);
}

static void release_keys(int fd) {
  /*
   * writes that all mouse keys are unpressed
   */
  emit_intr(fd, EV_KEY, BTN_LEFT, 0);
  emit_intr(fd, EV_KEY, BTN_RIGHT, 0);
  emit_intr(fd, EV_KEY, BTN_MIDDLE, 0);
  emit_intr(fd, EV_KEY, BTN_SIDE, 0);
  emit_intr(fd, EV_KEY, BTN_EXTRA, 0);
}
