
TARGET	= marley_accel

SRCDIR  = src
OBJDIR  = obj

SRCS    := $(shell find $(SRCDIR) -name '*.c')
SRCDIRS := $(shell find . -name '*.c' -exec dirname {} \; | uniq)
OBJS    := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

CC = gcc
CFLAGS  = -std=gnu11 -O2 -Wall -pedantic
USB = -lusb `pkg-config libusb-1.0 --cflags --libs`
LDFLAGS =


all: $(TARGET)

$(TARGET) : buildrepo $(OBJS)
	$(CC) $(OBJS) $(USB) $(LDFLAGS) -o $@ -lm


run: all
	su -c "./marley_accel $(CONFIG_FILE_PATH)"

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -lm

clean:
	$(RM) $(TARGET)
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
