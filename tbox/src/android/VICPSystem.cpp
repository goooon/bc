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
#define LOG_TAG "VICPSystem"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <sys/types.h>
#include <utils/Errors.h>
#include <utils/String8.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/PermissionCache.h>
#include <private/android_filesystem_config.h>
#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <utils/String16.h>

#include "VICPSystem.h"
#include "IVICPListener.h"
#include "IVICPSystem.h"
#include "../inc/Application.h"


/* tbox */
#include "../inc/dep.h"
#include "../inc/Application.h"
#include "../inc/channels.h"

namespace android {

void VICPSystem::notifyListeners(int32_t app_id, 
		uint8_t* buf, int32_t len)
{
	sp<myVICPListener> lsr;

    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mListeners.size(); i++) {
		lsr = mListeners[i];
		if (app_id == lsr->app_id) {
	        lsr->listener->data_arrived(app_id, 
				buf, len);
		}
    }
}

void VICPSystem::registerListener(int32_t app_id, 
		const sp<IVICPListener>& listener)
{
	sp<myVICPListener> lsr;

	LOG_I("%s, app_id: %d, listener: %p", __FUNCTION__, 
		app_id, &listener);

    if (listener == NULL)
        return;

    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mListeners.size(); i++) {
		lsr = mListeners[i];
        if (lsr->listener->asBinder() == listener->asBinder()) {
            return;
        }
    }

	lsr = new myVICPListener(app_id, listener);
	if (lsr != NULL) {
    	mListeners.add(lsr);
	}
    listener->asBinder()->linkToDeath(this);
}

void VICPSystem::unregisterListener(int32_t app_id, 
		const sp<IVICPListener>& listener)
{
	sp<myVICPListener> lsr;

	LOG_I("%s, app_id: %d, listener: %p", __FUNCTION__, 
		app_id, &listener);

    if (listener == NULL)
        return;

    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mListeners.size(); i++) {
		lsr = mListeners[i];
        if (lsr->listener->asBinder() == listener->asBinder()) {
            lsr->listener->asBinder()->unlinkToDeath(this);
            mListeners.removeAt(i);
            break;
        }
    }
}

static void android_vicp_send_cb(void *context, int result)
{
	if (result != 0) {
		LOG_W("%s result = %d", __FUNCTION__, result);
	}
}

static void android_pack_send(bcp_channel_t *ch, int app_id, 
	const char *buf, int len)
{
	bcp_packet_t *p;
	u8 *out;
	u32 olen;

	p = bcp_create_one_message(app_id, 0, 
		bcp_next_seq_id(), (u8*)buf, len);
	if (p) {
		if (bcp_packet_serialize(p, &out, &olen) >= 0) {
			bcp_vicp_send(ch, (const char*)out, (int)olen, 
				android_vicp_send_cb, NULL, NULL);
			free(out);
		}
		bcp_packet_destroy(p);
	}
}

void VICPSystem::send(int32_t app_id, uint8_t *buf, int32_t len)
{
	bcp_channel_t *ch;
	const char *ch_name = "serial";

	if (!buf) {
		return;
	}

	ch = channels_get(ch_name);
	if (!ch) {
		free(buf);
		LOG_W("find channel failed. name: %s\n", ch_name);
		return;
	} else {
		android_pack_send(ch, (int)app_id, 
			(const char*)buf, (int)len);
		free(buf);
	}
}

status_t VICPSystem::dump(int fd, const Vector<String16>& /*args*/)
{
    IPCThreadState* self = IPCThreadState::self();
    const int pid = self->getCallingPid();
    const int uid = self->getCallingUid();
    if ((uid != AID_SHELL) &&
        !PermissionCache::checkPermission(
                String16("android.permission.DUMP"), pid, uid))
        return PERMISSION_DENIED;
    return OK;
}

void VICPSystem::binderDied(const wp<IBinder>& who)
{    
	sp<myVICPListener> lsr;

	LOG_I("%s, who: %p", __FUNCTION__, &who);

	Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mListeners.size(); i++) {
		lsr = mListeners[i];
        if (lsr->listener->asBinder() == who) {
            mListeners.removeAt(i);
            break;
        }
    }
}

static sp<VICPSystem> s = NULL;
static int VICPSystem_init(void)
{
    s = new VICPSystem();
	if (s != NULL) {
		s->publishAndJoinThreadPool();
		return 0;
	}
	return -1;
}
static void VICP_System_notify(int32_t app_id,
	uint8_t *buf, int32_t len)
{
	if (s != NULL) {
		s->notifyListeners(app_id, buf, len);
	}
}
}  // namespace android

class androidVicpLoop : public Thread
{
	virtual void run() OVERRIDE
	{
		android::VICPSystem_init();
	}
};

int android_vicp_server_start(void)
{
	Thread::startThread(new androidVicpLoop());
	return 0;
}

void android_vicp_notify(int32_t app_id,
	uint8_t *buf, int32_t len)
{
	android::VICP_System_notify(app_id, buf, len);
}

