
TARGET	= marley_accel
TEST    = test_marley_accel

SRCDIR  = src
OBJDIR  = obj

SRCS    := $(shell find $(SRCDIR) -name '*.c')
SRCDIRS := $(shell find . -name '*.c' -exec dirname {} \; | uniq)
OBJS    := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
# Test ojects. We only test the accel functions.
TOBJS   := $(patsubst src/m_accel.c,$(OBJDIR)/src/m_accel.o,src/m_accel.c)

CFLAGS  = -std=gnu11 -O2 -Wall -pedantic
USB     = -lusb `pkg-config libusb-1.0 --cflags --libs`


all: $(TARGET)

test: $(TEST)

run: all
	su -c "./marley_accel $(CONFIG_FILE_PATH)"

$(TEST): buildrepo $(OBJS)
	cc $(TOBJS) $(CFLAGS) $(USB) unit_tests.c -o $@ -lm;
	./test_marley_accel

$(TARGET) : buildrepo $(OBJS)
	cc $(OBJS) $(USB) -o $@ -lm

$(OBJDIR)/%.o: %.c
	cc $(CFLAGS) -c $< -o $@ -lm

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
