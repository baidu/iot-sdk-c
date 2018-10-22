#
# specify the related path of iot-edge-c-sdk that you've cloned from github
#

BAIDU_SRC = .


#
# Get the list of C files
#

C_FILES += $(BAIDU_SRC)/certs/certs.c \
$(BAIDU_SRC)/c-utility/src/buffer.c \
$(BAIDU_SRC)/c-utility/src/consolelogger.c \
$(BAIDU_SRC)/c-utility/src/crt_abstractions.c \
$(BAIDU_SRC)/c-utility/src/doublylinkedlist.c \
$(BAIDU_SRC)/c-utility/src/map.c \
$(BAIDU_SRC)/c-utility/src/optionhandler.c \
$(BAIDU_SRC)/c-utility/src/singlylinkedlist.c \
$(BAIDU_SRC)/c-utility/src/strings.c \
$(BAIDU_SRC)/c-utility/src/utf8_checker.c \
$(BAIDU_SRC)/c-utility/src/vector.c \
$(BAIDU_SRC)/c-utility/src/xio.c \
$(BAIDU_SRC)/c-utility/src/xlogging.c \
$(BAIDU_SRC)/serializer/src/agenttypesystem.c \
$(BAIDU_SRC)/serializer/src/codefirst.c \
$(BAIDU_SRC)/serializer/src/commanddecoder.c \
$(BAIDU_SRC)/serializer/src/datamarshaller.c \
$(BAIDU_SRC)/serializer/src/datapublisher.c \
$(BAIDU_SRC)/serializer/src/dataserializer.c \
$(BAIDU_SRC)/serializer/src/iotdevice.c \
$(BAIDU_SRC)/serializer/src/jsondecoder.c \
$(BAIDU_SRC)/serializer/src/jsonencoder.c \
$(BAIDU_SRC)/serializer/src/methodreturn.c \
$(BAIDU_SRC)/serializer/src/multitree.c \
$(BAIDU_SRC)/serializer/src/schema.c \
$(BAIDU_SRC)/serializer/src/schemalib.c \
$(BAIDU_SRC)/serializer/src/schemaserializer.c \
$(BAIDU_SRC)/umqtt/src/mqtt_client.c \
$(BAIDU_SRC)/umqtt/src/mqtt_codec.c \
$(BAIDU_SRC)/umqtt/src/mqtt_message.c \
$(BAIDU_SRC)/parson/parson.c \
$(BAIDU_SRC)/iothub_client/src/iothub_mqtt_client.c \
$(BAIDU_SRC)/iothub_client/src/iothub_client_persistence.c \
\
\
$(BAIDU_SRC)/platform/agenttime.c \
$(BAIDU_SRC)/platform/platform_mbed.c \
$(BAIDU_SRC)/platform/threadapi_mbed.c \
$(BAIDU_SRC)/platform/tickcounter_mbed.c \
$(BAIDU_SRC)/platform/socketio_berkeley.c \
$(BAIDU_SRC)/platform/tlsio_mbedtls.c \
$(BAIDU_SRC)/platform/iot_smarthome_client.c \

#
# test modules
#
#C_FILES += $(BAIDU_SRC)/iothub_client/samples/iothub_client_sample/iothub_mqtt_client_sample.c
C_FILES += $(BAIDU_SRC)/platform/iot_smarthome_client_sample.c 


#
# compile options
# note: to minimize the code size, disable the logging

CFLAGS  += -DNO_LOGGING
CFLAGS  += -DUSE_MBED_TLS
CFLAGS  += -DSUPPORT_MBEDTLS

#
# include paths
#

CFLAGS	+= -I$(BAIDU_SRC)/certs
CFLAGS	+= -I$(BAIDU_SRC)/c-utility/inc
CFLAGS	+= -I$(BAIDU_SRC)/parson
CFLAGS	+= -I$(BAIDU_SRC)/umqtt/inc
CFLAGS  += -I$(BAIDU_SRC)/serializer/inc
CFLAGS  += -I$(BAIDU_SRC)/samples/inc
CFLAGS  += -I$(BAIDU_SRC)/iothub_client/inc
