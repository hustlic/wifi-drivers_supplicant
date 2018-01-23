KERNEL_SRC=`pwd`/../../../../linux-gerda

KBUILD_OPTIONS := WLAN_ROOT=$(PWD)
KBUILD_OPTIONS += MODNAME=wlan

# Determine if the driver license is Open source or proprietary
# This is determined under the assumption that LICENSE doesn't change.
# Please change here if driver license text changes.
LICENSE_FILE ?= $(PWD)/$(WLAN_ROOT)/CORE/HDD/src/wlan_hdd_main.c
WLAN_OPEN_SOURCE = $(shell if grep -q "MODULE_LICENSE(\"Dual BSD/GPL\")" \
		$(LICENSE_FILE); then echo 1; else echo 0; fi)

#By default build for CLD
WLAN_SELECT := CONFIG_QCA_CLD_WLAN=m
KBUILD_OPTIONS += CONFIG_QCA_WIFI_ISOC=0
KBUILD_OPTIONS += CONFIG_QCA_WIFI_2_0=1
KBUILD_OPTIONS += $(WLAN_SELECT)
KBUILD_OPTIONS += WLAN_OPEN_SOURCE=$(WLAN_OPEN_SOURCE)
KBUILD_OPTIONS += $(KBUILD_EXTRA) # Extra config if any

all:
	$(MAKE) ARCH=arm CROSS_COMPILE=`pwd`/../../../../prebuilts/gcc/linux-x86/arm/arm-linux-gnueabihf/bin/arm-linux-gnueabihf- -C $(KERNEL_SRC) M=$(shell pwd) modules $(KBUILD_OPTIONS)
	`pwd`/../../../../prebuilts/gcc/linux-x86/arm/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-strip --strip-debug --strip-unneeded wlan.ko
	cp wlan.ko ../../wpa_supplicant/qualcomm_wpa_supplicant/binary/firmware/
	
strip:
	/home/licheng/goni/prebuilts/gcc/linux-x86/arm/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-strip --strip-debug --strip-unneeded wlan.ko	
	
modules_install:
	$(MAKE) INSTALL_MOD_STRIP=1 -C $(KERNEL_SRC) M=$(shell pwd) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
