
CC = gcc 
PRECOMP = 0 # 1 to precompute accel values 
DEBUG = 0	# 1 to enter the debug mode
PREPROC = -D PRECOMP=$(PRECOMP) -D DEBUG=$(DEBUG)
CFLAGS = -std=gnu11 -g -O2 -Wall -pedantic 
USB = -lusb `pkg-config libusb-1.0 --cflags --libs`
MODULES = loading_util.o m_accel.o m_driver.o find_mouse.o
OBJ = -o marley_accel.o

target: $(MODULES)
	$(CC) $(CFLAGS) -DDEBUG=$(DEBUG) $(OBJ) src/marley_accel.c -lm $(MODULES) $(USB);

run: target
	su -c "./marley_accel.o $(CONFIG_FILE_PATH)"


errmsg.o:
	$(CC) $(CFLAGS) -o errmsg.o src/errmsg.c -c

loading_util.o:
	$(CC) $(CFLAGS) -o loading_util.o src/loading_util.c -c

m_driver.o:
	$(CC) $(CFLAGS) -DDEBUG=$(DEBUG) -o m_driver.o src/m_driver.c -c

m_accel.o:
	$(CC) $(CFLAGS) -o m_accel.o src/m_accel.c -c

find_mouse.o:
	$(CC) $(CFLAGS) -o find_mouse.o src/find_mouse.c -c

clean:
	rm *.o

