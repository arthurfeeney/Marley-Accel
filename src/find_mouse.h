#ifndef FIND_MOUSE_H
#define FIND_MOUSE_H

typedef struct mouse_info {
  bool found; /* true if mouse was found, false otherwise. */
  uint16_t vendor_id;
  uint16_t product_id;
  uint16_t endpoint_in;
  uint16_t interface;
} mouse_info_t;

mouse_info_t find_mouse(void);

#endif
