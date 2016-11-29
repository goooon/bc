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
#include "../inc/Application.h"

#define APPID_GPS 0x8000

namespace android {

void VICPListener::data_arrived(int32_t app_id, 
		uint8_t *buf, int32_t len)
{
	LOG_I("%s app_id: %d, data: %s, len: %d", __FUNCTION__,
		app_id, buf, len);
}
void VICPListener::binderDied(const wp<IBinder>& who)
{
	LOG_I("%s, who: %p", __FUNCTION__, &who);
}

static void joinThreadPool(void)
{
	sp<ProcessState> ps(ProcessState::self());
	ps->startThreadPool();
	ps->giveThreadPoolName();
	IPCThreadState::self()->joinThreadPool();
}
static int client_main(void)
{
    sp<IServiceManager> sm = defaultServiceManager();

    sp<IBinder> binder =
        sm->getService(String16("VICPSystem"));
    sp<IVICPSystem> service =
        interface_cast<IVICPSystem>(binder);

	if (service == NULL) {
		return -1;
	}

	sp<VICPListener> listener = new VICPListener();
	service->registerListener(APPID_GPS, listener);

	joinThreadPool();

	return 0;
}
} // namespace android

int android_vicp_client_start(void)
{
	return android::client_main();
}

