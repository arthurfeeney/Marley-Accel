#include <fcntl.h>
#include <limits.h>
#include <signal.h>
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

/**
 * Boolean flag to run the mouse driver. When it is switched to false, the
 * driver will stop. The original kernel mouse driver should reattach.
 */
static bool run_mouse_driver = true;

static void press_keys(int, int *);

#if defined(DEBUG) && DEBUG + 0
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

/**
 * On interrupt (CTRL-c), elegantly turn off the mouse driver.
 */
void interrupt_handler(int sig) { run_mouse_driver = false; }

/**
 * The actual acceleration driver.
 * gets mouse interrupt packets, applies acceleration functions to the relative
 * change in mouse position, and writes it to uinput.
 */
int accel_driver(int fd, mouse_dev_t *dev, accel_settings_t *as) {
  int err;

#if defined(PRECOMP) && PRECOMP + 0
  // precompute accel values
  precomp(as);
#endif

  struct sigaction act = {.sa_handler = interrupt_handler};
  sigaction(SIGINT, &act, NULL);

  const int buf_size = dev->buf_size;
  unsigned char mouse_interrupt_buf[buf_size];
  int actual_interrupt_length;
  while (run_mouse_driver) {
    err = libusb_interrupt_transfer(
        dev->usb_handle, dev->endpoint_in, mouse_interrupt_buf,
        sizeof(mouse_interrupt_buf), &actual_interrupt_length, 0);
    if (err < 0 || actual_interrupt_length > buf_size) {
      printf("interrupt length %d\n", actual_interrupt_length);
      return err;
    }
#if defined(DEBUG) && DEBUG + 0
    intrmsg(mouse_interrupt_buf, actual_interrupt_length);
#endif
    map_to_uinput(fd, mouse_interrupt_buf, actual_interrupt_length, as);
  }
  return 0;
}

/*
 * emit (write) interrupt to uinput at fd
 */
void emit_intr(int fd, int type, int code, int val) {
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
  emit_intr(fd, EV_SYN, SYN_REPORT, 0);
}

void map_scroll_to_uinput(int fd, unsigned char *buf, int buf_size) {
  const int scroll_idx = buf_size - 1; // always at the last index.
  if (buf[scroll_idx] != 0) {
    emit_intr(fd, EV_REL, REL_WHEEL, (signed char)buf[scroll_idx]);
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

/**
 * Given mask of currently pressed keys, write the info to uinput
 *
 * pressed[i] = 1 means that key i is pressed. If zero, it is released.
 * This can easily handle keycodes when multiple keys are pressed.
 */
void map_key_to_uinput(int fd, unsigned char *buf) {
  const int idx = buf[0];
  int pressed[5] = {0, 0, 0, 0, 0};
  assign_pressed(key_code_map[idx], pressed);
  press_keys(fd, pressed);
}

/**
 * Given mask of pressed keys, Emits interrupt for all mouse keys. Releases any
 * keys that are not held down.
 */
static void press_keys(int fd, int *pressed) {
  emit_intr(fd, EV_KEY, BTN_LEFT, pressed[0]);
  emit_intr(fd, EV_KEY, BTN_RIGHT, pressed[1]);
  emit_intr(fd, EV_KEY, BTN_MIDDLE, pressed[2]);
  emit_intr(fd, EV_KEY, BTN_SIDE, pressed[3]);
  emit_intr(fd, EV_KEY, BTN_EXTRA, pressed[4]);
}

static int buf_to_delta(unsigned char low, unsigned char sign) {
  return (delta_t)(sign == 0 ? low : (signed char)low);
}

void map_move_to_uinput(int fd, unsigned char *buf, int buf_size,
                        accel_settings_t *as) {
  // retrieve the changes in mouse position.
  // convert to signed char (overflowed values become proper d* in negative
  // direction.)
  delta_t dx = buf_to_delta(buf[1], buf[2]);
  delta_t dy = buf_to_delta(buf[3], buf[4]);
  // dx and dy are updated in-place.
  accelerate(&dx, &dy, as);
  emit_intr(fd, EV_REL, REL_X, dx);
  emit_intr(fd, EV_REL, REL_Y, dy);
}
