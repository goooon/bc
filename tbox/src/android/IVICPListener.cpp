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
#define LOG_TAG "BpVICPListener"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include "IVICPListener.h"

namespace android {

class BpVICPListener : public BpInterface<IVICPListener>
{
public:
    BpVICPListener(const sp<IBinder>& impl)
        : BpInterface<IVICPListener>(impl)
    {
    }

    void data_arrived(int32_t app_id, 
		uint8_t *buf, int32_t len)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IVICPListener::getInterfaceDescriptor());
        data.writeInt32(app_id);
		data.writeByteArray(len, buf);
		data.writeInt32(len);
        status_t err = remote()->transact(TRANSACT_VICP_DATA_ARRIVED, 
			data, &reply);
    }
};

IMPLEMENT_META_INTERFACE(VICPListener, "com.beecloud.vicp.IVICPListener");

status_t BnVICPListener::onTransact(uint32_t code,
	const Parcel& data,
	Parcel* reply,
	uint32_t flags)
{
    switch(code) {
		case TRANSACT_VICP_DATA_ARRIVED: {
            CHECK_INTERFACE(IVICPListener, data, reply);
			int32_t app_id = data.readInt32();
			int32_t buf_len = data.readInt32();
			uint8_t *buf = NULL;

			if ((buf_len >= 0 && buf_len <= (int32_t)data.dataAvail())) {
				buf = (uint8_t*)malloc(buf_len);
				const void *p = data.readInplace(buf_len);
				if (buf) {
					memcpy(buf, p, buf_len);
				}
			}
			int32_t len = data.readInt32();
			if (buf_len != len) {
				ALOGW("buf_len=%d != len=%d", buf_len, len);
				if (buf) {
					free(buf);
					buf = NULL;
				}
			}
			if (buf) {
            	data_arrived(app_id, buf, len);
			}
			return OK;
		}
    }
    return BBinder::onTransact(code, data, reply, flags);
};

// ----------------------------------------------------------------------------

}; // namespace android

