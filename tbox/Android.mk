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
# test

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../fundation \
	$(LOCAL_PATH)/../dep/paho/src \
	$(LOCAL_PATH)/../dep/nmealib 

LOCAL_SRC_FILES:= \
	src/cpp/Application.cpp          \
	src/cpp/CanBus.cpp               \
	src/cpp/CmdParser.cpp            \
	src/cpp/Config.cpp               \
	src/cpp/dep.cpp                  \
	src/cpp/Event.cpp                \
	src/cpp/main.cpp                 \
	src/cpp/Mqtt.cpp                 \
	src/cpp/Schedule.cpp             \
	src/cpp/Sensor.cpp               \
	src/cpp/Task.cpp                 \
	src/cpp/Vehicle.cpp              \
	src/cpp/channels.cpp             \
	src/tasks/AcquireConfigTask.cpp  \
	src/tasks/BCMessage.cpp          \
	src/tasks/Element.cpp            \
	src/tasks/GPSDataQueue.cpp       \
	src/tasks/GpsUploadTask.cpp      \
	src/tasks/MqttConnTask.cpp       \
	src/tasks/PackageQueue.cpp       \
	src/tasks/StateUploadTask.cpp    \
	src/tasks/TaskTable.cpp          \
	src/tasks/VehicleAuthTask.cpp    \
	src/tasks/VKeyActiveTask.cpp     \
	src/tasks/VKeyDeactiveTask.cpp   \
	src/tasks/VKeyIgnitionTask.cpp   \
	src/tasks/VKeyUnIgnitTask.cpp    \
	src/tasks/PackageQueueTask.cpp   \
	src/tasks/VehicleShakeTask.cpp    \
	src/tasks/vis/visGpsTask.cpp     \
	src/test/ActiveTest.cpp          \
	src/android/IVICPListener.cpp    \
	src/android/IVICPSystem.cpp      \
	src/android/VICPSystem.cpp       \
	dep/mpu6050/main6050.cpp

LOCAL_MODULE := tbox

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog \
	libnmea \
	libbinder \
	libpaho-mqtt3a \
	libfoundation

include $(BUILD_EXECUTABLE)
