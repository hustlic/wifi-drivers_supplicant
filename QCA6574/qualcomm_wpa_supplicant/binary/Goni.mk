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
#       wpa_supplicant.conf         # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := wpa_supplicant.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/wifi
LOCAL_SRC_FILES := etc/wpa_supplicant.conf
include $(BUILD_PREBUILT)

#####################################
#         empty.conf                # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := empty.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/wifi
LOCAL_SRC_FILES := etc/empty.conf
include $(BUILD_PREBUILT)

#####################################
#       iperf                       # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := iperf
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
LOCAL_SRC_FILES := bin/iperf
include $(BUILD_PREBUILT)

#####################################
#       enable_adb                  # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := enable_adb.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
LOCAL_SRC_FILES := bin/enable_adb.sh
include $(BUILD_PREBUILT)

#####################################
#            adbd                   # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := adb_client
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
LOCAL_SRC_FILES := bin/adb_client
include $(BUILD_PREBUILT)

#####################################
#            devmem                 #
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := devmem
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
LOCAL_SRC_FILES := bin/devmem
include $(BUILD_PREBUILT)

#####################################
#       entropy                     # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := entropy
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := firmware/entropy
include $(BUILD_PREBUILT)

#####################################
#       bdwlan30.bin                # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := bdwlan30.bin
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/firmware
LOCAL_SRC_FILES := firmware/bdwlan30.bin
include $(BUILD_PREBUILT)

#####################################
#       otp30.bin                   # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := otp30.bin
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/firmware
LOCAL_SRC_FILES := firmware/otp30.bin
include $(BUILD_PREBUILT)

#####################################
#       qwlan30.bin                 # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := qwlan30.bin
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/firmware
LOCAL_SRC_FILES := firmware/qwlan30.bin
include $(BUILD_PREBUILT)

#####################################
#       utf30.bin                   # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := utf30.bin
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/firmware
LOCAL_SRC_FILES := firmware/utf30.bin
include $(BUILD_PREBUILT)

#####################################
#       utfbd30.bin                 # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := utfbd30.bin
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/firmware
LOCAL_SRC_FILES := firmware/utfbd30.bin
include $(BUILD_PREBUILT)

#####################################
#       cfg.dat                     # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := cfg.dat
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/firmware/wlan
LOCAL_SRC_FILES := firmware/wlan/cfg.dat
include $(BUILD_PREBUILT)

#####################################
#       qcom_cfg.ini                # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := qcom_cfg.ini
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/firmware/wlan
LOCAL_SRC_FILES := firmware/wlan/qcom_cfg.ini
include $(BUILD_PREBUILT)

#####################################
#       wlan.ko                     # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := wlan.ko
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/modules
LOCAL_SRC_FILES := firmware/wlan.ko
include $(BUILD_PREBUILT)

#####################################
#      qualcomm debug tools         # 
#      iw tools and lib             #
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng
LOCAL_PREBUILT_EXECUTABLES := bin/athdiag bin/cld-fwlog-netlink bin/cld-fwlog-parser bin/cld-fwlog-record bin/pktlogconf
LOCAL_PREBUILT_EXECUTABLES += bin/ifrename bin/iwconfig bin/iwevent bin/iwgetid bin/iwlist bin/iwpriv bin/iwspy
LOCAL_PREBUILT_LIBS := lib/libiw.so.29
include $(BUILD_MULTI_PREBUILT)
