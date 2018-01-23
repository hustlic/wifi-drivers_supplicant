#Copyright @ 2013 - 2014 Suntec Software(Shanghai) Co., Ltd.
#All Rights Reserved.

#Redistribution and use in source and binary forms, with or without
#modification, are NOT permitted except as agreed by
#Suntec Software(Shanghai) Co., Ltd.

#Unless required by applicable law or agreed to in writing, software
#distributed under the License is distributed on an "AS IS" BASIS,
#WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE        := wlmdemo
LOCAL_MODULE_TAGS   := eng
LOCAL_SRC_FILES     := wlmSampleTests.c
LOCAL_C_INCLUDES    := $(LOCAL_PATH)/../wl
LOCAL_SHARED_LIBRARIES	:=  libwltm
LOCAL_LDFLAGS       := -lrt -lpthread
include $(BUILD_EXECUTABLE)