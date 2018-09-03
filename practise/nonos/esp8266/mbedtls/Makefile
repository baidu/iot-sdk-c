OPEN_SDK   = /Volumes/esp-open-sdk/esp-open-sdk
BUILD_BASE	= build
SDK_BASE = $(OPEN_SDK)/sdk

# name for the target
TARGET		= libmbedtls

# which modules (subdirectories) of the project to include in compiling
MODULES		= platform library

# crosstool dir
XTENSA_TOOLS_ROOT ?= $(OPEN_SDK)/xtensa-lx106-elf/bin/

# include dirs
EXTRA_INCDIR = ./include ./include/mbedtls  ../esp8266_lwip/include  $(SDK_BASE)/include .

# compiler flags using during compilation of source files
CFLAGS		= -O3 -Wreturn-type -Wpointer-arith -Wundef \
				-Werror -Wl,-EL -fno-inline-functions -nostdlib \
				-mlongcalls -mtext-section-literals  -D__ets__ \
				-DICACHE_FLASH -DLWIP_OPEN_SRC \
				-DMBEDTLS_CONFIG_FILE='"config_esp.h"'

# select which tools to use as compiler and librarian
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
OBJCOPY := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy


# source files dir
SRC_DIR		:= $(MODULES)

# obj files dir
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))


SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
APP_AR		:= $(addprefix ./,$(TARGET).a)

INCDIR	:= $(addprefix -I,$(SRC_DIR)/../..)
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(CFLAGS) -c $$< -o $$@
	$(OBJCOPY) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $$@
endef

.PHONY: all clean checkdirs

all: checkdirs $(APP_AR)

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	$(Q) mkdir -p $@

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -rf $(BUILD_DIR)
	$(Q) rm -rf $(BUILD_BASE)


$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
