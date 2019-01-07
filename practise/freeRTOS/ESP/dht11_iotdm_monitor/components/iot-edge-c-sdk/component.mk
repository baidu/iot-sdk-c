#
# Component Makefile
#

# Component configuration in preprocessor defines
CFLAGS += -DESP8266_SOC_RTC
CFLAGS += -DUSE_MBED_TLS
CFLAGS += -Wno-error=char-subscripts
CFLAGS += -Wno-error=enum-compare
CFLAGS += -Wno-error=pointer-sign
CFLAGS += -Wno-error=format-zero-length
CFLAGS += -Wno-error=format=



EDGE_SDK_PATH = ../../../../../../../iot-edge-c-sdk
ESP_PAL_PATH = ../../../smarthome_demo/components/pal/

COMPONENT_ADD_INCLUDEDIRS :=  \
$(ESP_PAL_PATH)/inc \
$(EDGE_SDK_PATH)/c-utility/inc  \
$(EDGE_SDK_PATH)/iothub_client/inc \
$(EDGE_SDK_PATH)/umqtt/inc  \
$(EDGE_SDK_PATH)/umqtt/inc/azure_umqtt_c 	\
$(EDGE_SDK_PATH)/c-utility/pal/generic \
$(EDGE_SDK_PATH)/certs \
$(EDGE_SDK_PATH)/c-utility/pal/inc \
$(EDGE_SDK_PATH)/parson \
$(EDGE_SDK_PATH)/serializer/inc

COMPONENT_OBJS =  \
$(EDGE_SDK_PATH)/c-utility/src/xlogging.o	\
$(EDGE_SDK_PATH)/c-utility/src/consolelogger.o	\
$(EDGE_SDK_PATH)/c-utility/src/buffer.o	\
$(EDGE_SDK_PATH)/c-utility/src/crt_abstractions.o	\
$(EDGE_SDK_PATH)/c-utility/src/singlylinkedlist.o	\
$(EDGE_SDK_PATH)/c-utility/src/doublylinkedlist.o	\
$(EDGE_SDK_PATH)/c-utility/src/utf8_checker.o	\
$(EDGE_SDK_PATH)/c-utility/src/map.o	\
$(EDGE_SDK_PATH)/c-utility/src/optionhandler.o	\
$(EDGE_SDK_PATH)/c-utility/src/strings.o	\
$(EDGE_SDK_PATH)/c-utility/src/vector.o	\
$(EDGE_SDK_PATH)/c-utility/src/xio.o	\
$(EDGE_SDK_PATH)/serializer/src/agenttypesystem.o \
$(EDGE_SDK_PATH)/serializer/src/codefirst.o \
$(EDGE_SDK_PATH)/serializer/src/commanddecoder.o \
$(EDGE_SDK_PATH)/serializer/src/datamarshaller.o \
$(EDGE_SDK_PATH)/serializer/src/datapublisher.o \
$(EDGE_SDK_PATH)/serializer/src/dataserializer.o \
$(EDGE_SDK_PATH)/serializer/src/iotdevice.o \
$(EDGE_SDK_PATH)/serializer/src/jsondecoder.o \
$(EDGE_SDK_PATH)/serializer/src/jsonencoder.o \
$(EDGE_SDK_PATH)/serializer/src/methodreturn.o \
$(EDGE_SDK_PATH)/serializer/src/multitree.o \
$(EDGE_SDK_PATH)/serializer/src/schema.o \
$(EDGE_SDK_PATH)/serializer/src/schemalib.o \
$(EDGE_SDK_PATH)/serializer/src/schemaserializer.o \
$(EDGE_SDK_PATH)/parson/parson.o \
$(EDGE_SDK_PATH)/iothub_client/src/rsa_signer/rsa_signer_mbedtls.o \
$(EDGE_SDK_PATH)/iothub_client/src/bos.o \
$(EDGE_SDK_PATH)/c-utility/src/httpapiex.o	\
$(EDGE_SDK_PATH)/c-utility/src/httpapiexsas.o	\
$(EDGE_SDK_PATH)/c-utility/src/httpheaders.o	\
$(EDGE_SDK_PATH)/c-utility/adapters/httpapi_compact.o \
\
\
$(EDGE_SDK_PATH)/iothub_client/src/iothub_client_persistence.o	\
$(EDGE_SDK_PATH)/iothub_client/src/iotdm_client.o	\
$(EDGE_SDK_PATH)/iothub_client/src/iothub_mqtt_client.o	\
\
\
$(EDGE_SDK_PATH)/umqtt/src/mqtt_client.o	\
$(EDGE_SDK_PATH)/umqtt/src/mqtt_codec.o	\
$(EDGE_SDK_PATH)/umqtt/src/mqtt_message.o	\
\
\
\
$(ESP_PAL_PATH)/src/tlsio_mbedtls_direct_no_buffer.o \
$(ESP_PAL_PATH)/src/platform_esp.o	\
$(ESP_PAL_PATH)/src/socketio_lwip.o \
$(EDGE_SDK_PATH)/c-utility/adapters/agenttime.o	\
$(EDGE_SDK_PATH)/umqtt/deps/c-utility/pal/freertos/threadapi.o	\
$(EDGE_SDK_PATH)/umqtt/deps/c-utility/pal/freertos/tickcounter.o	\
$(EDGE_SDK_PATH)/c-utility/pal/lwip/sntp_lwip.o	\
$(EDGE_SDK_PATH)/certs/certs.o \
$(EDGE_SDK_PATH)/c-utility/pal/tlsio_options.o \
\
\
\


COMPONENT_SRCDIRS :=  \
$(EDGE_SDK_PATH)/c-utility/src \
$(EDGE_SDK_PATH)/c-utility/adapters  \
$(EDGE_SDK_PATH)/umqtt/src	\
$(EDGE_SDK_PATH)/iothub_client/src  \
$(EDGE_SDK_PATH)/certs \
$(EDGE_SDK_PATH)/umqtt/deps/c-utility/pal/freertos \
$(EDGE_SDK_PATH)/c-utility/pal/lwip \
$(EDGE_SDK_PATH)/c-utility/pal \
$(ESP_PAL_PATH)/src \
$(EDGE_SDK_PATH)/serializer/src   \
$(EDGE_SDK_PATH)/parson   \
$(EDGE_SDK_PATH)/iothub_client/src/rsa_signer \
















