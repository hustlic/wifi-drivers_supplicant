LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
	cert.c ecc_openssl.c alg_comm.c sms4c.c wapi.c wapi_common.c wapi_interface.c wpi_pcrypt.c

LOCAL_C_INCLUDES += \
	external/openssl/include

LOCAL_CFLAGS += -DLE -DECC_NO_ECC192_ECDH -DECC_NEED_NID_X9_62_PRIME192V4 -DASUE

LOCAL_MODULE:= libwapi

include $(BUILD_STATIC_LIBRARY)
