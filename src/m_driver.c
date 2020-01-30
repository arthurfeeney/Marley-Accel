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

#define DEBUG 0

static void press_keys(int, int *);

#if DEBUG
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
#endif

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

  const int buf_size = dev->buf_size;
  int iterations = 300;
  unsigned char mouse_interrupt_buf[buf_size];
  int actual_interrupt_length;
  while (iterations--) {
    err = libusb_interrupt_transfer(
        dev->usb_handle, dev->endpoint_in, mouse_interrupt_buf,
        sizeof(mouse_interrupt_buf), &actual_interrupt_length, 0);
    if (err < 0 || actual_interrupt_length > buf_size) {
      printf("length %d", actual_interrupt_length);
      return err;
    }
#if DEBUG
    intrmsg(mouse_interrupt_buf, actual_interrupt_length);
#endif
    map_to_uinput(fd, mouse_interrupt_buf, buf_size, as);
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

void map_to_uinput(int fd, unsigned char *buf, int buf_size,
                   accel_settings_t *as) {
  map_key_to_uinput(fd, buf);
  map_scroll_to_uinput(fd, buf, buf_size);
  map_move_to_uinput(fd, buf, buf_size, as);
}

void map_scroll_to_uinput(int fd, unsigned char *buf, int buf_size) {
  const int scroll_idx = buf_size - 1; // always at the last index.
  if (buf[scroll_idx] != 0) {
    emit_intr(fd, EV_REL, REL_WHEEL, (signed char)buf[scroll_idx]);
    emit_intr(fd, EV_SYN, SYN_REPORT, 0);
  }
}

static void assign_pressed(const int *const key_codes, int *pressed) {
  const int len = key_codes[0];
  for (int i = 1; i < len + 1; ++i) {
    if (key_codes[i] == BTN_LEFT)
      pressed[0] = 1;
    else if (key_codes[i] == BTN_RIGHT)
      pressed[1] = 1;
    else if (key_codes[i] == BTN_MIDDLE)
      pressed[2] = 1;
    else if (key_codes[i] == BTN_SIDE)
      pressed[3] = 1;
    else if (key_codes[i] == BTN_EXTRA)
      pressed[4] = 1;
  }
}

void map_key_to_uinput(int fd, unsigned char *buf) {
  /*
   * value = 1 is a press, 0 is a release
   * Some keycodes are for when multiple keys are held down.
   * These can be seen by printing with intrmsg.
   */
  const int idx = buf[0];
  int pressed[5] = {0, 0, 0, 0, 0};
  // for (int i = 1; i < len + 1; ++i) {
  //  emit_intr(fd, EV_KEY, key_code_map[idx][i], 1);
  //}
  assign_pressed(key_code_map[idx], pressed);
  press_keys(fd, pressed);
  emit_intr(fd, EV_SYN, SYN_REPORT, 0);
}

static void press_keys(int fd, int *pressed) {
  /*
   * emits interrupt for all mouse keys. Releases any keys
   * that are not held down.
   */
  emit_intr(fd, EV_KEY, BTN_LEFT, pressed[0]);
  emit_intr(fd, EV_KEY, BTN_RIGHT, pressed[1]);
  emit_intr(fd, EV_KEY, BTN_MIDDLE, pressed[2]);
  emit_intr(fd, EV_KEY, BTN_SIDE, pressed[3]);
  emit_intr(fd, EV_KEY, BTN_EXTRA, pressed[4]);
}

void map_move_to_uinput(int fd, unsigned char *buf, int buf_size,
                        accel_settings_t *as) {
  // retrieve the changes in mouse position.
  // convert to signed char (overflowed values become proper d* in negative
  // direction.)
  // When the sent packet is six bytes, buf[2] and buf[4] contain
  // the signs for buf[1] and buf[3]. They are not needed because
  // converted to signed char gives the correct negation.
  signed char dx = (signed char)buf[1];
  signed char dy = (signed char)buf_size == 6 ? buf[3] : buf[2];
  // dx and dy are updated in-place.
  accelerate(&dx, &dy, as);
  // write accelerated change to uinput
  emit_intr(fd, EV_REL, REL_X, dx);
  emit_intr(fd, EV_REL, REL_Y, dy);
  emit_intr(fd, EV_SYN, SYN_REPORT, 0);
}
