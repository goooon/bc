#
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(my-dir)

#########################################
# nmea1803

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_SRC_FILES:= \
	src/context.c \
	src/generator.c \
	src/gpgga.c \
	src/gpgsa.c \
	src/gpgsv.c \
	src/gprmc.c \
	src/gpvtg.c \
	src/gpzda.c \
	src/info.c \
	src/nmath.c \
	src/parser.c \
	src/sentence.c \
	src/util.c \
	src/validate.c \

LOCAL_MODULE := libnmea

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog

include $(BUILD_SHARED_LIBRARY)
