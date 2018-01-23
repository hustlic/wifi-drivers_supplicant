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
#      rtl_wpa_supplicant.conf          # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := wpa_supplicant.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/wifi
LOCAL_SRC_FILES := etc/rtl_wpa_supplicant.conf
include $(BUILD_PREBUILT)

#####################################
#      rtl_p2p_supplicant.conf          # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := p2p_supplicant.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/wifi
LOCAL_SRC_FILES := etc/rtl_p2p_supplicant.conf
include $(BUILD_PREBUILT)

#####################################
#      hostapd.conf                 # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := hostapd.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/var
LOCAL_SRC_FILES := etc/rtl_hostapd.conf
include $(BUILD_PREBUILT)


#####################################
#       iperf                       # 
#####################################
include $(CLEAR_VARS)
LOCAL_MODULE := iperf
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/bin
LOCAL_SRC_FILES := bin/iperf_h3_arm64
include $(BUILD_PREBUILT)

