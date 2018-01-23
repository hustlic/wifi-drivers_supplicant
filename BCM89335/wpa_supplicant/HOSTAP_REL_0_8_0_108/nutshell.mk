#Copyright @ 2013 - 2014 Suntec Software(Shanghai) Co., Ltd.
#All Rights Reserved.

#Redistribution and use in source and binary forms, with or without
#modification, are NOT permitted except as agreed by
#Suntec Software(Shanghai) Co., Ltd.

#Unless required by applicable law or agreed to in writing, software
#distributed under the License is distributed on an "AS IS" BASIS,
#WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

# Do not compile bluetooth and wifi code in emu
# ifeq ($(filter generic generic_x86, $(TARGET_DEVICE)),)
LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/wapilib/nutshell.mk \
	$(LOCAL_PATH)/libbcmdhd/nutshell.mk \
        $(LOCAL_PATH)/hostapd/nutshell.mk \
        $(LOCAL_PATH)/wpa_supplicant/nutshell.mk \
        $(LOCAL_PATH)/binary/nutshell.mk \
        $(LOCAL_PATH)/wl/nutshell.mk\
        $(LOCAL_PATH)/wlmdemo/nutshell.mk\
        
        
        
# endif
