/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "VICPClient"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <sys/types.h>
#include <utils/Errors.h>
#include <utils/String8.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/PermissionCache.h>
#include <private/android_filesystem_config.h>
#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <utils/String16.h>

#include "IVICPListener.h"
#include "IVICPSystem.h"
#include "VICPClient.h"

namespace android {

void VICPListener::data_arrived(int32_t app_id, 
		uint8_t *buf, int32_t len)
{
	ALOGI("%s client app_id: %d, len: %d", 
		__FUNCTION__, app_id, len);
}
void VICPListener::binderDied(const wp<IBinder>& who)
{
	ALOGI("%s, who: %p", __FUNCTION__, &who);
}
} // namespace android

