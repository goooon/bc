TOPDIR = ..

include $(TOPDIR)/Makefile.inc

#
# Settings
#

SAMPLESDYNAMICLINK ?= 0

H_FILES = $(wildcard ../include/nmealib/*.h)
C_FILES = $(wildcard */main.c)
SAMPLES = $(sort $(patsubst %/,%,$(dir $(C_FILES))))

OBJDIRS = $(SAMPLES:%=build/%)
BINARIES = $(SAMPLES:%=lib/%)

.PRECIOUS: $(BINARIES) $(OBJDIRS:%=%/main.o)

#CFLAGS += -I $(TOPDIR)/include
LDLAGS += -L $(TOPDIR)/lib
STATICLIBS =

ifneq ($(SAMPLESDYNAMICLINK),0)
  LDLAGS += -lnmea  -lm
else
  STATICLIBS += $(TOPDIR)/lib/$(LIBNAMESTATIC)
endif


#
# Targets
#

all: all-before samples

remake: clean all

samples: $(BINARIES)

lib/%: %/main.c $(H_FILES) Makefile $(TOPDIR)/Makefile.inc
ifeq ($(VERBOSE),0)
	@echo "[CC] $<"
endif
	$(MAKECMDPREFIX)$(CC) $(CFLAGS) $< -I $(TOPDIR)/include $(INCLUDES) $(LDLAGS) -o $@


#
# Phony Targets
#

.PHONY: all-before clean

all-before:
	$(MAKECMDPREFIX)mkdir -p build lib $(OBJDIRS)

clean:
	$(MAKECMDPREFIX)rm -fr build lib