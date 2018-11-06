# if you are using Mac OS, set this flag to '1'
MAC = 

CFLAGS = -g -O2 -std=gnu99
DFLAGS = -DUSE_OPENSSL

# set these variables to openssl lib/include dir path on your system
OPENSSL_LIB_DIR = 
OPENSSL_INC_DIR = 

LDFLAGS =  -L$(OPENSSL_LIB_DIR) -lssl -lcrypto 
LDFLAGS +=  -lm -lpthread -ldl 

ifneq ("$(MAC)", "1")
LDFLAGS +=  -lrt
endif

TARGET := iothub_sample_test

# modify to your cross-compiler name if your are cross-compiling
CROSS_COMPILE = 
CC = gcc

BUILD_BASE = ./build
OBJ_BASE = $(BUILD_BASE)/objs
EDGE_SDK_DIR=../../../..


V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif


INC = -I$(OPENSSL_INC_DIR) \
-I$(EDGE_SDK_DIR)/c-utility/inc  \
-I$(EDGE_SDK_DIR)/c-utility/inc/azure_c_shared_utility \
-I$(EDGE_SDK_DIR)/iothub_client/inc \
-I$(EDGE_SDK_DIR)/umqtt/inc  \
-I$(EDGE_SDK_DIR)/umqtt/inc/azure_umqtt_c 	\
-I$(EDGE_SDK_DIR)/iothub_client/samples/iothub_client_sample \
-I$(EDGE_SDK_DIR)/c-utility/pal/generic \
-I$(EDGE_SDK_DIR)/certs \
-I$(EDGE_SDK_DIR)/c-utility/pal/inc


SOURCES = \
$(EDGE_SDK_DIR)/c-utility/src/xlogging.o	\
$(EDGE_SDK_DIR)/c-utility/src/consolelogger.o	\
$(EDGE_SDK_DIR)/c-utility/src/buffer.o	\
$(EDGE_SDK_DIR)/c-utility/src/crt_abstractions.o	\
$(EDGE_SDK_DIR)/c-utility/src/singlylinkedlist.o	\
$(EDGE_SDK_DIR)/c-utility/src/doublylinkedlist.o	\
$(EDGE_SDK_DIR)/c-utility/src/utf8_checker.o	\
$(EDGE_SDK_DIR)/c-utility/src/map.o	\
$(EDGE_SDK_DIR)/c-utility/src/optionhandler.o	\
$(EDGE_SDK_DIR)/c-utility/src/strings.o	\
$(EDGE_SDK_DIR)/c-utility/src/vector.o	\
$(EDGE_SDK_DIR)/c-utility/src/xio.o	\
$(EDGE_SDK_DIR)/certs/certs.c \
\
\
$(EDGE_SDK_DIR)/iothub_client/src/iothub_client_persistence.o	\
$(EDGE_SDK_DIR)/iothub_client/src/iothub_mqtt_client.o	\
\
\
$(EDGE_SDK_DIR)/umqtt/src/mqtt_client.o	\
$(EDGE_SDK_DIR)/umqtt/src/mqtt_codec.o	\
$(EDGE_SDK_DIR)/umqtt/src/mqtt_message.o	\
\
\
\
$(EDGE_SDK_DIR)/c-utility/adapters/agenttime.c \
$(EDGE_SDK_DIR)/c-utility/adapters/threadapi_pthreads.c \
$(EDGE_SDK_DIR)/c-utility/adapters/tickcounter_linux.c \
$(EDGE_SDK_DIR)/c-utility/adapters/socketio_berkeley.c \
$(EDGE_SDK_DIR)/c-utility/adapters/lock_pthreads.c \
$(EDGE_SDK_DIR)/c-utility/adapters/linux_time.c \
$(EDGE_SDK_DIR)/c-utility/adapters/platform_linux.c \
$(EDGE_SDK_DIR)/c-utility/src/tlsio_openssl.c \
$(EDGE_SDK_DIR)/c-utility/src/x509_openssl.c \
$(EDGE_SDK_DIR)/c-utility/pal/tlsio_options.c \
\
$(EDGE_SDK_DIR)/iothub_client/samples/iothub_client_sample/iothub_mqtt_client_sample.c \
./main.c


vpath %.c $(EDGE_SDK_DIR)/c-utility/src \
$(EDGE_SDK_DIR)/iothub_client/src \
$(EDGE_SDK_DIR)/umqtt/src \
$(EDGE_SDK_DIR)/c-utility/adapters \
$(EDGE_SDK_DIR)/c-utility/pal \
$(EDGE_SDK_DIR)/certs \
$(EDGE_SDK_DIR)/iothub_client/samples/iothub_client_sample

OBJS = $(notdir $(patsubst %.c,%.o,$(SOURCES)))

%.o:%.c | out
	$(vecho) "CC $<"
	$(Q) $(CROSS_COMPILE)$(CC) $(DFLAGS) $(CFLAGS) $(INC) -o $(addprefix $(OBJ_BASE)/,$@) -c $<

$(BUILD_BASE)/$(TARGET):$(OBJS)
	$(vecho) "TARGET $@"
	$(Q) $(CROSS_COMPILE)$(CC)  -o $(BUILD_BASE)/$(TARGET) $(addprefix $(OBJ_BASE)/,$(OBJS)) $(LDFLAGS)

all: $(BUILD_BASE)/$(TARGET)

.PHONY : clean
clean:
	$(Q) rm -rf $(OBJ_BASE)
	$(Q) rm -f $(BUILD_BASE)/$(TARGET)

out:
	$(Q) mkdir -p $(BUILD_BASE)
	$(Q) mkdir -p $(OBJ_BASE)
