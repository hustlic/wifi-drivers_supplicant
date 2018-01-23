#!/bin/sh

rmmods() { for i in $*; do rmmod $i; done > /dev/null 2>&1; }

rmmods dhd bcmdhd

dhdko=/usr/lib/modules/bcmdhd.ko

fmwr=/etc/wifi/firmware/bcmdhd_apsta.bin
#fmwr=/home/root/firmware/bcm4339_mfg.bin

nvrm=/etc/wifi/firmware/nvram.txt

for i in dhdko fmwr nvrm; do
  eval i=\$$i
  if [ ! -f $i ]; then
    echo "FILE $i does not exist!"
    exit 1
  fi
done


insmod $dhdko firmware_path=$fmwr nvram_path=$nvrm #dhd_msg_level=0xffff
#/usr/local/sbin/sshd
rm -rf /var/run/wpa_supplicant
ifconfig wlan0 up
