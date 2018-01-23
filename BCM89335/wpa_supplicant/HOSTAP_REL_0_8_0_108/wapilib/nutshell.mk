LOCAL_PATH:= $(call my-dir)
WAPI_LIB_SOURCES_AVAILABLE := y

include $(LOCAL_PATH)/../wpa_supplicant/nutshell.config
ifndef CONFIG_WAPI
WAPI_LIB_SOURCES_AVAILABLE := n
endif
ifeq ($(WAPI_LIB_SOURCES_AVAILABLE), y)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
	src/cert.c src/ecc_openssl.c src/alg_comm.c src/sms4c.c src/wapi.c src/wapi_common.c src/wapi_interface.c src/wpi_pcrypt.c
LOCAL_C_INCLUDES += \
	$(call include-path-for, openssl)

LOCAL_CFLAGS += -DLE -DECC_NO_ECC192_ECDH -DECC_NEED_NID_X9_62_PRIME192V4 -DASUE

LOCAL_MODULE:= libwapi
LOCAL_MODULE_TAGS := debug

include $(BUILD_STATIC_LIBRARY)

else

include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH), x86)
LOCAL_SRC_FILES := libwapi-x86.a
else
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := libwapi.a
endif

LOCAL_MODULE:= libwapi
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
include $(BUILD_PREBUILT)
endif
