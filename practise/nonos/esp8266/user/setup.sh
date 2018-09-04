#!/bin/bash

# our experimental codes are base on "60691b12c5"
#git submodule status
# 79feaeedab64941c70bcb9cb46292a94aa410f10 c-utility (2018-03-07-temp-pod-27-g79feaee)
# 4e8a9012423dd8dda6b8f7b4cae40cbd440011a8 parson (4e8a901)
# 2ebbe7f19a4495d5e6412d6e88acf1c5d9f0943a umqtt (2017-02-24-192-g2ebbe7f)
# copy all codes from that

TMP_DIR=__tmp
SDK_BASE=../../../../../
TMP_SDK_BASE=$TMP_DIR

SDK_VER=60691b12c5
UTIL_VER=79feaeedab64
PAR_VER=4e8a9012423d
MQTT_VER=2ebbe7f19a449

if [ ! -d "$TMP_DIR" ]; then
    mkdir -p $TMP_DIR
    mkdir -p src
    mkdir -p inc
    cd $TMP_DIR
    # copy the SDK project
    echo `pwd`
    rsync -av --progress ../../../../../ . --exclude practise

    git reset --hard HEAD
    git checkout $SDK_VER
    
    cd c-utility
    git reset --hard HEAD
    git checkout $UTIL_VER
    cd ..

    cd parson
    git reset --hard HEAD
    git checkout $PAR_VER
    cd ..

    cd umqtt
    git reset --hard HEAD
    git checkout $MQTT_VER
    cd ..

    
    echo "prepare to copying files"

    # copy .c files to src
    cp c-utility/src/xlogging.c \
c-utility/src/buffer.c \
c-utility/src/crt_abstractions.c \
c-utility/src/doublylinkedlist.c \
c-utility/src/utf8_checker.c \
c-utility/src/map.c \
c-utility/src/optionhandler.c \
c-utility/src/strings.c \
c-utility/src/usha.c \
c-utility/src/vector.c \
c-utility/src/xio.c \
c-utility/src/singlylinkedlist.c \
\
\
iothub_client/src/iothub_client_persistence.c \
iothub_client/src/iothub_mqtt_client.c \
\
\
umqtt/src/mqtt_client.c \
umqtt/src/mqtt_codec.c \
umqtt/src/mqtt_message.c \
\
\
\
c-utility/adapters/threadapi_esp8266.c \
c-utility/adapters/tickcounter_esp8266.c \
c-utility/adapters/agenttime_esp8266.c \
c-utility/adapters/lock_pthreads.c \
c-utility/pal/tlsio_options.c \
c-utility/pal/lwip/sntp_lwip.c \
\
certs/certs.c \
iothub_client/samples/iothub_client_sample/iothub_mqtt_client_sample.c   ../src

# copy .h files to inc
    cp iothub_client/samples/iothub_client_sample/iothub_mqtt_client_sample.h  \
c-utility/inc/azure_c_shared_utility/macro_utils.h   \
c-utility/inc/azure_c_shared_utility/agenttime.h   \
c-utility/inc/azure_c_shared_utility/buffer_.h   \
c-utility/inc/azure_c_shared_utility/crt_abstractions.h   \
c-utility/inc/azure_c_shared_utility/doublylinkedlist.h   \
iothub_client/inc/iothub_client_persistence.h \
iothub_client/inc/iothub_mqtt_client.h \
iothub_client/inc/iothub_client.h \
c-utility/inc/azure_c_shared_utility/strings_types.h   \
c-utility/inc/azure_c_shared_utility/tickcounter.h   \
c-utility/inc/azure_c_shared_utility/xio.h   \
c-utility/inc/azure_c_shared_utility/optionhandler.h   \
c-utility/inc/azure_c_shared_utility/strings.h   \
c-utility/inc/azure_c_shared_utility/tlsio.h   \
c-utility/inc/azure_c_shared_utility/tlsio_mbedtls.h   \
c-utility/inc/azure_c_shared_utility/tlsio_options.h   \
c-utility/inc/azure_c_shared_utility/platform.h   \
c-utility/inc/azure_c_shared_utility/socketio.h   \
c-utility/inc/azure_c_shared_utility/threadapi.h   \
c-utility/inc/azure_c_shared_utility/xlogging.h   \
c-utility/inc/azure_c_shared_utility/utf8_checker.h   \
c-utility/inc/azure_c_shared_utility/map.h   \
c-utility/inc/azure_c_shared_utility/vector.h   \
c-utility/inc/azure_c_shared_utility/vector_types.h   \
c-utility/inc/azure_c_shared_utility/vector_types_internal.h   \
c-utility/inc/azure_c_shared_utility/singlylinkedlist.h   \
c-utility/inc/azure_c_shared_utility/const_defines.h   \
c-utility/inc/azure_c_shared_utility/shared_util_options.h   \
umqtt/inc/azure_umqtt_c/mqtt_client.h \
umqtt/inc/azure_umqtt_c/mqtt_codec.h \
umqtt/inc/azure_umqtt_c/mqtt_message.h \
umqtt/inc/azure_umqtt_c/mqttconst.h  \
certs/certs.h        ../inc

    
    # do patch
    cd ../src
    patch -p1 < ../src.patch
    cd ../inc
    patch -p1 < ../inc.patch
    cd ..

    # do some cleanning
    rm -rf $TMP_DIR
fi

