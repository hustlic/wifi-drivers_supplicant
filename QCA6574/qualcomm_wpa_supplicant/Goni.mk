LOCAL_PATH:= $(call my-dir)

ifneq ($(filter LA_AF_1_1_1,$(WPA_SUPPLICANT_VERSION)),)
# The order of the 2 Android.mks does matter!
# TODO: Clean up the Android.mks, reset all the temporary variables at the
# end of each Android.mk, so that one Android.mk doesn't depend on variables
# set up in the other Android.mk.
include $(LOCAL_PATH)/hostapd/Goni.mk \
        $(LOCAL_PATH)/wpa_supplicant/Goni.mk \
        $(LOCAL_PATH)/binary/Goni.mk
endif
