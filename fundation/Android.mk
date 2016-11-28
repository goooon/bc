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

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../dep/nmealib/include

LOCAL_SRC_FILES:= \
    src/cpp/core/Types.cpp              \
	src/cpp/util/SharedMemory.cpp       \
	src/cpp/util/Semaphore.cpp          \
	src/cpp/util/Thread.cpp             \
	src/cpp/util/Tree.cpp               \
	src/cpp/util/Heap.cpp               \
	src/cpp/util/StackTrace.cpp         \
	src/cpp/util/Log.cpp                \
	src/cpp/util/LinkedList.cpp         \
	src/cpp/util/Pipe.cpp               \
	src/cpp/bcp.cpp                     \
	src/cpp/bcp_comm.cpp                \
	src/cpp/bcp_packet.cpp              \
	src/cpp/binary_formater.cpp         \
	src/cpp/crc32.cpp                   \
	src/cpp/bcp_serial.cpp              \
	src/cpp/bcp_nmea.cpp                \
	src/cpp/util/Timestamp.cpp          \
	src/cpp/bcp_channel.cpp             \
	src/cpp/vicp/bcp_vicp_packet.cpp    \
	src/cpp/vicp/slice_proto.cpp        \
	src/cpp/vicp/bcp_vicp_receiver.cpp  \
	src/cpp/vicp/bcp_vicp_sender.cpp    \
	src/cpp/vicp/bcp_vicp_slice.cpp     \
	src/cpp/vicp/bcp_vicp.cpp

LOCAL_MODULE := libfoundation

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog \
	libnmea \
	libpaho-mqtt3a

include $(BUILD_SHARED_LIBRARY)
