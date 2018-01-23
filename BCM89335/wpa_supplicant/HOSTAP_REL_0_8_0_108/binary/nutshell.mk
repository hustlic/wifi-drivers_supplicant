#Copyright @ 2013 - 2014 Suntec Software(Shanghai) Co., Ltd.
#All Rights Reserved.

#Redistribution and use in source and binary forms, with or without
#modification, are NOT permitted except as agreed by
#Suntec Software(Shanghai) Co., Ltd.

#Unless required by applicable law or agreed to in writing, software
#distributed under the License is distributed on an "AS IS" BASIS,
#WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

LOCAL_PATH := $(call my-dir)

#####################################
#      bcmdhd.ko                    # 
#####################################
#include $(CLEAR_VARS)
#LOCAL_MODULE := bcmdhd.ko
#LOCAL_MODULE_CLASS := ETC
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/wifi/firmware
#LOCAL_SRC_FILES := firmware/bcmdhd.ko
#include $(BUILD_PREBUILT)

#####################################
#       nvram.txt                   # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := nvram.txt
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/wifi/firmware
LOCAL_SRC_FILES := firmware/nvram.txt
include $(BUILD_PREBUILT)

#####################################
#       bcm4339_sta_p2p.bin         # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := bcm4339_sta_p2p.bin
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/wifi/firmware
LOCAL_SRC_FILES := firmware/bcm4339_sta_p2p.bin
include $(BUILD_PREBUILT)

#####################################
#       bcmdhd_apsta.bin            # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := bcmdhd_apsta.bin
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/wifi/firmware
LOCAL_SRC_FILES := firmware/bcmdhd_apsta.bin
include $(BUILD_PREBUILT)

#####################################
#       bcm4339_mfg.bin             # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := bcm4339_mfg.bin
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/wifi/firmware
LOCAL_SRC_FILES := firmware/bcm4339_mfg.bin
include $(BUILD_PREBUILT)


#####################################
#      sta_supplicant.conf           # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := wpa_supplicant.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/wifi
LOCAL_SRC_FILES := etc/wpa_supplicant.conf
include $(BUILD_PREBUILT)

#####################################
#      p2p_supplicant.conf           # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := p2p_supplicant.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/wifi
LOCAL_SRC_FILES := etc/p2p_supplicant.conf
include $(BUILD_PREBUILT)

#####################################
#      hostapd.conf                 # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := hostapd.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/var
LOCAL_SRC_FILES := etc/hostapd.conf
include $(BUILD_PREBUILT)


#####################################
#       iperf                       # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := iperf
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/bin
LOCAL_SRC_FILES := bin/iperf
include $(BUILD_PREBUILT)

#####################################
#      insmod_ap.sh                 # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := insmod_ap.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/bin
LOCAL_SRC_FILES := bin/insmod_ap.sh
include $(BUILD_PREBUILT)

#####################################
#      insmod_stap2p.sh             # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := insmod_stap2p.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/bin
LOCAL_SRC_FILES := bin/insmod_stap2p.sh
include $(BUILD_PREBUILT)

#####################################
#       dhdutil                     #
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := dhdutil
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/bin
LOCAL_SRC_FILES := bin/dhdutil
include $(BUILD_PREBUILT)
