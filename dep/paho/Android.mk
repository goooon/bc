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
# libmqtt3

include $(CLEAR_VARS)

COMPILE_DATE := $(shell date)
MQTT_VERSION := 1.0.3
VERSIONINFO_FILE := $(LOCAL_PATH)/VersionInfo.h

$(shell echo "#ifndef VERSIONINFO_H\"" > $(VERSIONINFO_FILE))
$(shell echo "#define VERSIONINFO_H" >> $(VERSIONINFO_FILE))
$(shell echo "" >> $(VERSIONINFO_FILE))
$(shell echo "#define BUILD_TIMESTAMP \"$(COMPILE_DATE)\"" >> $(VERSIONINFO_FILE))
$(shell echo "#define CLIENT_VERSION \"$(MQTT_VERSION)\"" >> $(VERSIONINFO_FILE))
$(shell echo "" >> $(VERSIONINFO_FILE))
$(shell echo "#endif /* VERSIONINFO_H */" >> $(VERSIONINFO_FILE))

LOCAL_SRC_FILES:= \
	VersionInfo.h \
	src/MQTTProtocolClient.c \
	src/Clients.c \
	src/utf-8.c \
	src/StackTrace.c \
	src/MQTTPacket.c \
	src/MQTTPacketOut.c \
	src/Messages.c \
	src/Tree.c \
	src/Socket.c \
	src/Log.c \
	src/MQTTPersistence.c \
	src/Thread.c \
	src/MQTTProtocolOut.c \
	src/MQTTPersistenceDefault.c \
	src/SocketBuffer.c \
	src/Heap.c \
	src/LinkedList.c \
	src/MQTTAsync.c

LOCAL_MODULE := libpaho-mqtt3a

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog

include $(BUILD_SHARED_LIBRARY)
