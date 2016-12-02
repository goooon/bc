ifneq ($(BUILD_TINY_ANDROID),true)
#Compile this library only for builds with the latest modem image

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := gps.default
LOCAL_MODULE_OWNER := beecloud

LOCAL_MODULE_TAGS := optional

## Libs

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
	libnmea \
	libfoundation \
	libdl \
	libbinder \
	libhardware

LOCAL_SRC_FILES += \
	IVICPListener.cpp \
	IVICPSystem.cpp \
	VICPClient.cpp \
    location.cpp \
    gps.c

LOCAL_CFLAGS += \
    -fno-short-enums \
    -D_ANDROID_

## Includes
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../../../fundation \
	$(LOCAL_PATH)/../../../../dep/paho/src \
	$(LOCAL_PATH)/../../../../dep/nmealib/include

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_RELATIVE_PATH := hw

include $(BUILD_SHARED_LIBRARY)

endif # not BUILD_TINY_ANDROID
