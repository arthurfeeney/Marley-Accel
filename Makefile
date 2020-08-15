
CC      = clang
TARGET	= marley_accel
TEST    = test_marley_accel

SRCDIR  = src
OBJDIR  = obj

SRCS    := $(shell find $(SRCDIR) -name '*.c')
SRCDIRS := $(shell find . -name '*.c' -exec dirname {} \; | uniq)
OBJS    := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

DEBUG      = 0
SAN 	   = -fsanitize=address,undefined
CFLAGS     = -std=gnu11 -O2 -Wall -Wextra -pedantic -DDEBUG -ffast-math -pipe
TESTFLAGS  = $(SAN) -fno-omit-frame-pointer -g
USB        = -lusb `pkg-config libusb-1.0 --cflags --libs`


all: $(TARGET)

test: $(TEST)

run: all
	su -c "./marley_accel $(CONFIG_FILE_PATH)"

$(TEST): buildrepo $(OBJS)
	$(CC) obj/src/mouse_accel.o obj/src/marley_map.o $(CFLAGS) $(TESTFLAGS) $(USB) unit_tests.c -o $@ -lm;
	./test_marley_accel

$(TARGET) : buildrepo $(OBJS)
	$(CC) $(OBJS) -fsanitize=address,undefined $(USB) -o $@ -lm

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET)
	$(RM) $(TEST)
	@rm -rf $(OBJDIR)

distclean: clean
	$(RM) $(APP)

buildrepo:
	@$(call make-repo)

define make-repo
   for dir in $(SRCDIRS); \
   do \
	mkdir -p $(OBJDIR)/$$dir; \
   done
endef
