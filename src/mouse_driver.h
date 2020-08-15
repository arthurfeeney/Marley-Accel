#ifndef MOUSE_DRIVER_H
#define MOUSE_DRIVER_H

// Defined in loading_util.h
typedef struct mouse_dev mouse_dev_t;
// Defined in m_accel.h
typedef struct accel_settings accel_settings_t;

int accel_driver(int fd, mouse_dev_t *, accel_settings_t *);
void emit_intr(int, unsigned short, unsigned short, int);
void map_to_uinput(int, unsigned char *, int, accel_settings_t *);
void map_key_to_uinput(int, unsigned char *);
void map_move_to_uinput(int, unsigned char *, accel_settings_t *);
void map_scroll_to_uinput(int, unsigned char *, int);

#endif
