
CC = gcc
CFLAGS = -O2 -Wall -pedantic -lm
USB = -lusb
LIBUSB = `pkg-config libusb-1.0 --cflags --libs`
MODULES = loading_util.o m_accel.o m_driver.o find_mouse.o
OBJ = -o zowie_hack.o

target: loading_util m_driver m_accel find_mouse errmsg
	$(CC) $(CFLAGS) $(OBJ) src/zowie_hack.c $(MODULES) $(USB) $(LIBUSB);
	su -c "./zowie_hack.o config.txt"

errmsg:
	$(CC) $(CFLAGS) -o errmsg.o src/errmsg.c -c

loading_util:
	$(CC) $(CFLAGS) -o loading_util.o src/loading_util.c -c

m_driver:
	$(CC) $(CFLAGS) -o m_driver.o src/m_driver.c -c

m_accel:
	$(CC) $(CFLAGS) -o m_accel.o src/m_accel.c -c

find_mouse:
	$(CC) $(CFLAGS) -o find_mouse.o src/find_mouse.c -c

clean:
	rm *.o
