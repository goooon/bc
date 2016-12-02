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

#define LOG_TAG "IVICPSystem"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include "IVICPListener.h"
#include "IVICPSystem.h"
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>

namespace android {

class BpVICPSystem : public BpInterface<IVICPSystem> {
public:
    BpVICPSystem(const sp<IBinder>& impl)
        : BpInterface<IVICPSystem>(impl) {}

        void registerListener(int32_t app_id, 
				const sp<IVICPListener>& listener) {
            Parcel data;
            data.writeInterfaceToken(IVICPSystem::getInterfaceDescriptor());
			data.writeInt32(app_id);
			data.writeStrongBinder(listener->asBinder());
            remote()->transact(REGISTER_LISTENER, data, NULL);
        }

        void unregisterListener(int32_t app_id, 
				const sp<IVICPListener>& listener) {
            Parcel data;
            data.writeInterfaceToken(IVICPSystem::getInterfaceDescriptor());
			data.writeInt32(app_id);
			data.writeStrongBinder(listener->asBinder());
            remote()->transact(UNREGISTER_LISTENER, data, NULL);
        }

		void send(int32_t app_id, uint8_t *buf, int32_t len) {
            Parcel data;
            data.writeInterfaceToken(IVICPSystem::getInterfaceDescriptor());
			data.writeInt32(app_id);
			data.writeByteArray(len, buf);
			data.writeInt32(len);
            remote()->transact(SEND_DATA, data, NULL);
		}
};

IMPLEMENT_META_INTERFACE(VICPSystem, "com.beecloud.vicp.IVICPSystem");

status_t BnVICPSystem::onTransact(uint32_t code,
	const Parcel& data,
	Parcel* reply,
	uint32_t flags)
{
    switch(code) {
        case REGISTER_LISTENER: {
            CHECK_INTERFACE(IVICPSystem, data, reply);
			int32_t app_id = data.readInt32();
            sp<IVICPListener> listener =
                interface_cast<IVICPListener>(data.readStrongBinder());
            registerListener(app_id, listener);
            return OK;
        }

        case UNREGISTER_LISTENER: {
            CHECK_INTERFACE(IVICPSystem, data, reply);
			int32_t app_id = data.readInt32();
            sp<IVICPListener> listener =
                interface_cast<IVICPListener>(data.readStrongBinder());
            unregisterListener(app_id, listener);
            return OK;
        }

		case SEND_DATA: {
            CHECK_INTERFACE(IVICPSystem, data, reply);
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
            	send(app_id, buf, len);
			}
			return OK;
		}
    }
    return BBinder::onTransact(code, data, reply, flags);
};

// ----------------------------------------------------------------------------

}; // namespace android

