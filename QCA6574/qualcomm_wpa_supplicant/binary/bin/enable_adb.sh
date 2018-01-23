#!/bin/sh
insmod /system/lib/modules/g_androidadb.ko
sleep 1
echo 0 > /sys/devices/virtual/android_usb/android0/enable
echo 18d1 > /sys/devices/virtual/android_usb/android0/idVendor
echo D002 > /sys/devices/virtual/android_usb/android0/idProduct
echo adb > /sys/devices/virtual/android_usb/android0/functions
sleep 1
echo 1 > /sys/devices/virtual/android_usb/android0/enable

mount -o remount rw /system

/system/bin/adb_client &



