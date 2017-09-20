#                                                                                                    
#  Copyright (C) Firmwave Ltd., All rights reserved.                                                 
#  Licensed under the MIT license. See LICENSE file in the project root for full license information.
#                                                                                                    

# Toolchain executables
CROSS_COMPILE = /home/huyun/Source/iotsdk/arm-none-eabi/bin/arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE = $(CROSS_COMPILE)size
NM = $(CROSS_COMPILE)nm

# Include files
INCLUDES = \
    -I../ \
    -I../../../../c-utility/inc \
    -I../../../../iothub_client/inc \
    -I../../../../umqtt/inc \
    -I../../../../c-utility/adapters \
    -I../../../../certs \
    -I/home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/inc \
    -I/home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/common \
    -I/home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/demo/common/atmel/boards/same70_xplained \
    -I/home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/demo/atmel/same70_xplained/mqtt_client_demo/src \
    -I/home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto \
    -I/home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_ssl \
    -I/home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp \
    -I/home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Source/include \
    -I/home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 \
    -I/home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Demo/CORTEX_M7_SAME70_Xplained_AtmelStudio/src/ASF/sam/utils/cmsis/same70/include \
    -I/home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Demo/CORTEX_M7_SAME70_Xplained_AtmelStudio/src/ASF/thirdparty/CMSIS/Include \
    -I/home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Demo/CORTEX_M7_SAME70_Xplained_AtmelStudio/src/ASF/sam/utils/cmsis/same70/source/templates \
    -I/home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Demo/CORTEX_M7_SAME70_Xplained_AtmelStudio/src/ASF/sam/utils/fpu \
    -I/home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Demo/CORTEX_M7_SAME70_Xplained_AtmelStudio/src/ASF/sam/utils \
    -I/home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Demo/CORTEX_M7_SAME70_Xplained_AtmelStudio/src/ASF/sam/utils/preprocessor \
    -I/home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Demo/CORTEX_M7_SAME70_Xplained_AtmelStudio/src/ASF/sam/utils/header_files \
    -I/home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Demo/CORTEX_M7_SAME70_Xplained_AtmelStudio/src/ASF/common/utils \

# Global defines
CFLAGS += \
    -DBOARD=SAME70_XPLAINED \
    -DMBED_BUILD_TIMESTAMP \
    -DTRACE_LEVEL=4 \
    -DUSE_SAME70_XPLAINED \
    -D_WINSOCK_H \
    -D__SAME70Q21__ \
    -Dprintf=iprintf \
    -D__error_t_defined \

# Global C flags
CFLAGS += -std=gnu99 -O0 -g \
-march=armv7e-m -mthumb -mfloat-abi=softfp  -mfpu=fpv5-sp-d16 \
-ffunction-sections \
-Wall -Wchar-subscripts -Wcomment -Wformat=2 -Wimplicit-int \
-Wmain -Wparentheses \
-Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs \
-Wuninitialized -Wunknown-pragmas -Wfloat-equal \
-Wshadow -Wpointer-arith -Wbad-function-cast -Wwrite-strings \
-Waggregate-return \
-Wmissing-prototypes -Wmissing-declarations \
-Wformat -Wmissing-format-attribute -Wno-deprecated-declarations \
-Wredundant-decls -Wnested-externs \
-Wunreachable-code

# Global linker flags
LDFLAGS += -march=armv7e-m -mthumb -mfloat-abi=softfp  -mfpu=fpv5-sp-d16 \
-Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler \
-Wl,--unresolved-symbols=report-all -Wl,--warn-common \
-Wl,--defsym=malloc_getpagesize_P=0x80 \
-L/home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/linker \
-T"/home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/demo/atmel/same70_xplained/mqtt_client_demo/as6/same70_flash.ld" \
-Wl,--start-group -lgcc -lc -lm -Wl,--end-group --specs=nosys.specs

# Global libraries

OBJECTS = $(patsubst %.c, obj/%.o, $(SOURCES))
THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
CFLAGS += $(INCLUDES)

SOURCES_iothub_client_sample = \
    iothub_mqtt_client_sample.c \
    c-utility/src/base64.c \
    c-utility/src/buffer.c \
    c-utility/src/consolelogger.c \
    c-utility/src/constbuffer.c \
    c-utility/src/constmap.c \
    c-utility/src/crt_abstractions.c \
    c-utility/src/doublylinkedlist.c \
    c-utility/src/gb_stdio.c \
    c-utility/src/gb_time.c \
    c-utility/src/gballoc.c \
    c-utility/src/hmac.c \
    c-utility/src/hmacsha256.c \
    c-utility/src/httpapiex.c \
    c-utility/src/httpapiexsas.c \
    c-utility/src/httpheaders.c \
    c-utility/src/map.c \
    c-utility/src/optionhandler.c \
    c-utility/src/sastoken.c \
    c-utility/src/sha1.c \
    c-utility/src/sha224.c \
    c-utility/src/sha384-512.c \
    c-utility/src/singlylinkedlist.c \
    c-utility/src/string_tokenizer.c \
    c-utility/src/strings.c \
    c-utility/src/tlsio_cyclonessl.c \
    c-utility/src/tlsio_cyclonessl_socket.c \
    c-utility/src/urlencode.c \
    c-utility/src/usha.c \
    c-utility/src/vector.c \
    c-utility/src/xio.c \
    c-utility/src/xlogging.c \
    certs/certs.c \
    iothub_client/src/iothub_client_retry_control.c \
    iothub_client/src/iothub_mqtt_client.c \
    iothub_client/src/version.c \
    umqtt/src/mqtt_client.c \
    umqtt/src/mqtt_codec.c \
    umqtt/src/mqtt_message.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/aes.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/aria.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/asn1.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/base64.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/camellia.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/chacha.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/chacha20_poly1305.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/cipher_mode_cbc.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/cipher_mode_ccm.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/cipher_mode_cfb.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/cipher_mode_ctr.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/cipher_mode_ecb.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/cipher_mode_gcm.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/cipher_mode_ofb.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/des.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/des3.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/dh.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/dsa.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/ec.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/ec_curves.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/ecdh.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/ecdsa.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/hmac.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/idea.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/keccak.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/md2.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/md4.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/md5.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/mpi.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/oid.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/pem.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/pkcs5.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/poly1305.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/rc4.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/rc6.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/ripemd128.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/ripemd160.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/rsa.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/seed.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/sha1.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/sha224.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/sha256.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/sha384.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/sha3_224.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/sha3_256.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/sha3_384.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/sha3_512.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/sha512.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/sha512_224.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/sha512_256.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/tiger.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/whirlpool.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/x509.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_crypto/yarrow.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_ssl/ssl_common.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_ssl/tls.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_ssl/tls_cache.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_ssl/tls_cipher_suites.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_ssl/tls_client.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_ssl/tls_common.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_ssl/tls_io.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_ssl/tls_misc.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_ssl/tls_record.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/ethernet.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/ip.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/net.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/net_mem.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/nic.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/ping.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/raw_socket.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/socket.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/tcp.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/tcp_fsm.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/tcp_misc.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/tcp_timer.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/core/udp.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/dhcp/dhcp_client.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/dhcp/dhcp_common.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/dhcp/dhcp_debug.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/dns/dns_cache.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/dns/dns_client.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/dns/dns_common.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/dns/dns_debug.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv4/arp.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv4/auto_ip.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv4/icmp.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv4/igmp.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv4/ipv4.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv4/ipv4_frag.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/icmpv6.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/ipv6.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/ipv6_frag.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/ipv6_misc.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/ipv6_pmtu.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/ipv6_routing.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/mld.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/ndp.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/ndp_cache.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/ndp_misc.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/ndp_router_adv.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/ipv6/slaac.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/netbios/nbns_client.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/netbios/nbns_common.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/netbios/nbns_responder.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/demo/atmel/same70_xplained/mqtt_client_demo/src/debug.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/demo/atmel/same70_xplained/mqtt_client_demo//src/ext_int_driver.c \
#    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/demo/atmel/same70_xplained/mqtt_client_demo/src/rtc.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/demo/atmel/same70_xplained/mqtt_client_demo/src/startup_same70.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/demo/atmel/same70_xplained/mqtt_client_demo/as6/syscalls.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/demo/atmel/same70_xplained/mqtt_client_demo/src/system_same70.c \
    /home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Demo/CORTEX_M7_SAME70_Xplained_AtmelStudio/src/ASF/common/utils/interrupt/interrupt_sam_nvic.c \
    /home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Source/croutine.c \
    /home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Source/event_groups.c \
    /home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Source/list.c \
    /home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Source/queue.c \
    /home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Source/tasks.c \
    /home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Source/timers.c \
    /home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.c \
    c-utility/adapters/platform_freertos.c \
    c-utility/adapters/tickcounter_freertos.c \
    c-utility/adapters/agenttime.c \
    c-utility/adapters/threadapi_freertos.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/common/os_port_freertos.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/common/endian.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/common/str.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/common/date_time.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/demo/common/atmel/boards/same70_xplained/sdram.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/drivers/ksz8081.c \
    /home/huyun/Source/iotsdk/CycloneTCP_SSL_Crypto_Open_1_7_4/cyclone_tcp/drivers/same70_eth.c \
    /home/huyun/Source/iotsdk/FreeRTOSv9.0.0/FreeRTOS/Source/portable/MemMang/heap_3.c \

OBJECTS_iothub_client_sample = $(patsubst %.c, obj/%.o, $(SOURCES_iothub_client_sample))

VPATH += \
    ../ \
    ../../../../ \

V ?= 0
Q_0 := @
Q_1 := 
Q = $(Q_$(V))

.SUFFIXES:
.PHONY: all clean

define loginfo
      $(Q)echo "[INFO] $1"
endef

define logerror
      $(Q)echo "[ERR ] $1"
endef

all: build

build: iothub_client_sample

iothub_client_sample: $(OBJECTS_iothub_client_sample) $(THIS_MAKEFILE)
	$(call loginfo,"Linking exec" $@)
	$(Q)$(CC) $(LDFLAGS) -Wl,-M=bin/$@.map $(CFLAGS) $(OBJECTS_iothub_client_sample) $(LIBS) -o bin/$@.elf
	$(Q)$(NM) bin/$@.elf >bin/$@.elf.txt
	$(Q)$(OBJCOPY) -O binary bin/$@.elf bin/$@.bin
	$(Q)$(SIZE) $(OBJECTS_iothub_client_sample) bin/$@.elf

$(OBJECTS): | bin obj

bin:
	$(Q)mkdir -p $@

obj:
	$(Q)mkdir -p $@

obj/%.o: %.c $(THIS_MAKEFILE)
	$(call loginfo,"Compiling file" $<)
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) -MT $@ -MMD -MP -MF obj/$*.Td $(CFLAGS) -c $< -o $@
	$(Q)mv -f obj/$*.Td obj/$*.d

clean:
	$(Q)rm -R bin
	$(Q)rm -R obj

obj/%.d: ;
.PRECIOUS: obj/%.d

-include $(patsubst %,obj/%.d,$(basename $(SOURCES)))
