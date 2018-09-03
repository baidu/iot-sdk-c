OPEN_SDK   = /Volumes/esp-open-sdk/esp-open-sdk
CROSS_TOOL = $(OPEN_SDK)/xtensa-lx106-elf/bin/xtensa-lx106-elf-
SDK_BASE   = $(OPEN_SDK)/sdk

CC = $(CROSS_TOOL)gcc
AR = $(CROSS_TOOL)ar
OBJCOPY = $(CROSS_TOOL)objcopy

CFLAGS = -Iinclude/ -Iinclude/arch/ -Iinclude/lwip/ -Iinclude/lwip/app -Iinclude/netif -I$(SDK_BASE)/include

CFLAGS +=  -O3 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DLWIP_OPEN_SRC  -DPBUF_RSV_FOR_WLAN -DEBUF_LWIP

OBJS += \
lwip/api/api_lib.o \
lwip/api/api_msg.o \
lwip/api/err.o \
lwip/api/netbuf.o \
lwip/api/netdb.o \
lwip/api/netifapi.o \
lwip/api/sockets.o \
lwip/api/tcpip.o \
lwip/app/dhcpserver.o \
lwip/app/espconn.o \
lwip/app/espconn_mdns.o \
lwip/app/espconn_tcp.o \
lwip/app/espconn_udp.o \
lwip/app/netio.o \
lwip/app/ping.o \
lwip/app/espconn_buf.o \
lwip/core/ipv4/autoip.o \
lwip/core/ipv4/icmp.o \
lwip/core/ipv4/igmp.o \
lwip/core/ipv4/inet.o \
lwip/core/ipv4/inet_chksum.o \
lwip/core/ipv4/ip.o \
lwip/core/ipv4/ip_addr.o \
lwip/core/ipv4/ip_frag.o \
lwip/core/def.o \
lwip/core/dhcp.o \
lwip/core/dns.o \
lwip/core/init.o \
lwip/core/mdns.o \
lwip/core/mem.o \
lwip/core/memp.o \
lwip/core/netif.o \
lwip/core/pbuf.o \
lwip/core/raw.o \
lwip/core/sntp.o \
lwip/core/stats.o \
lwip/core/sys.o \
lwip/core/sys_arch.o \
lwip/core/tcp.o \
lwip/core/tcp_in.o \
lwip/core/tcp_out.o \
lwip/core/timers.o \
lwip/core/udp.o \
lwip/netif/etharp.o \

.PHONY: all clean 

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
	$(OBJCOPY) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $@

all: liblwip.a

liblwip.a: $(OBJS)
	$(AR) rcs liblwip.a $(OBJS)

clean:
	rm -f $(OBJS) liblwip.a our/eagle_lwip_if.o
