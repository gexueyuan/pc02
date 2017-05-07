
TOOLCHAIN_PATH=/home/gexueyuan/bin/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin/
CROSS_COMPILE=$(TOOLCHAIN_PATH)mips-openwrt-linux-

AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
ECHO    = echo

CDEFS          = -pipe 
_ENDIAN        = __BIG_ENDIAN

VNET_DRIVER_TYPE  = VNET_DRIVER_TYPE_WIFI

OSFLAG = -DLINUX -D_GNU_SOURCE -DCONFIG_LIBNL30 -D$(_ENDIAN) -D_BYTE_ORDER=$(_ENDIAN) -D$(VNET_DRIVER_TYPE)

ifeq ("$(release)", "y")
	OSFLAG += -D_NDEBUG
endif


INCLUDEFLAGS += -I$(TOPDIR)/include \
		-I$(TOPDIR)/osal/ \
		-I$(TOPDIR)/osal/linux \



		
WARNING = -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common
#-Werror-implicit-function-declaration
CFLAGS = -O2 $(OSFLAG) $(WARNING) $(INCLUDEFLAGS)

DEPENDENCIES = dependence


