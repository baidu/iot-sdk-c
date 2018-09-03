BUILD_BASE	= build
OPEN_SDK   = /Volumes/esp-open-sdk/esp-open-sdk
ESP_CROSS_COMPILE_ROOT=$(OPEN_SDK)/xtensa-lx106-elf
ESP_SDK_DIR=$(OPEN_SDK)/sdk

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= $(ESP_CROSS_COMPILE_ROOT)/bin/

# base directory of the ESP8266 SDK package, absolute
SESPBASE	?= $(OPEN_SDK)/sdk

#Esptool.py path and port
ESPTOOL		?= esp8266/tools/esptool/esptool.py
ESPPORT		?= /dev/ttyUSB0

# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
MODULES		= user user/src
EXTRA_INCDIR    =  mbedtls/include mbedtls/include/mbedtls  mbedtls esp8266_lwip/include user/inc
EXTRA_LIBDIR	= $(OPEN_SDK)/xtensa-lx106-elf/xtensa-lx106-elf/sysroot/lib

# self compiled libs
EXTRA_ARS = esp8266_lwip/liblwip.a mbedtls/libmbedtls.a

# libraries used in this project, mainly provided by the SDK
LIBS		=  c gcc hal json main net80211 phy pp upgrade wpa crypto

# compiler flags using during compilation of source files
CFLAGS		= -O3  -Wreturn-type -Wpointer-arith -Wundef -Werror \
				-Wl,-EL -fno-inline-functions -nostdlib -mlongcalls \
				-mtext-section-literals  \
				-D__ets__ -DICACHE_FLASH -DLWIP_OPEN_SRC\
				-DMBEDTLS_CONFIG_FILE='"config_esp.h"' \
				-DUSE_MBED_TLS

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.new.1024.app1.ld

# various paths from the SDK used in this project
SESPLIBDIR	= lib
SESPLDDIR	= ld
SESPINCDIR	= include include/json

# we create two different files for uploading into the flash
# these are the names and options to generate them
FW_FILE_1	= 0x00000
FW_FILE_1_ARGS	= -bo $@ -bs .text -bs .data -bs .rodata -bc -ec
FW_FILE_2	= 0x40000
FW_FILE_2_ARGS	= -es .irom0.text $@ -ec

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
OBJCOPY := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy



####
#### no user configurable options below here
####
FW_TOOL		?= esp8266/tools/esptool/esptool
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SESPLIBDIR	:= $(addprefix $(SESPBASE)/,$(SESPLIBDIR))
SESPINCDIR	:= $(addprefix -I$(SESPBASE)/,$(SESPINCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

LD_SCRIPT	:= $(addprefix -T$(SESPBASE)/$(SESPLDDIR)/,$(LD_SCRIPT))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))
EXTRA_LIBDIR	:= $(addprefix -L,$(EXTRA_LIBDIR))

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
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SESPINCDIR) $(CFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs flash clean

all: checkdirs user1.bin

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SESPLIBDIR) $(EXTRA_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(EXTRA_ARS) $(APP_AR) -lm -Wl,--end-group -o $@
	
user1.bin: $(TARGET_OUT)
	@$(OBJCOPY) --only-section .text -O binary $< eagle.app.v6.text.bin
	@$(OBJCOPY) --only-section .data -O binary $< eagle.app.v6.data.bin
	@$(OBJCOPY) --only-section .rodata -O binary $< eagle.app.v6.rodata.bin
	@$(OBJCOPY) --only-section .irom0.text -O binary $< eagle.app.v6.irom0text.bin

	@COMPILE=gcc PATH=$(ESP_CROSS_COMPILE_ROOT)/bin:${PATH} python $(ESP_SDK_DIR)/tools/gen_appbin.py $< 2 0 0 4 1

	@mv eagle.app.flash.bin $@
	@rm eagle.app.v6.*
	@echo "Generate $@ successully."
	@echo "boot_v1.2.bin------------>0x00000"
	@echo "$@--->0x01000"
	
	
$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	$(Q) mkdir -p $@

firmware:
	#$(Q) mkdir -p $@

flash: firmware/0x00000.bin firmware/0x40000.bin
	-$(ESPTOOL) --port $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin 0x40000 firmware/0x40000.bin

clean:
	$(Q) rm -f	user1.bin
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -rf $(BUILD_DIR)
	$(Q) rm -rf $(BUILD_BASE)


$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
