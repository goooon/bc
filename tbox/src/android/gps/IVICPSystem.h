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

#ifndef __IVICPSYSTEM_H__
#define __IVICPSYSTEM_H__

#include <stdint.h>
#include <binder/IInterface.h>
#include "IVICPListener.h"

namespace android {

enum {
    REGISTER_LISTENER = IBinder::FIRST_CALL_TRANSACTION,
    UNREGISTER_LISTENER,
    SEND_DATA
};

class IVICPSystem : public IInterface {
public:
    DECLARE_META_INTERFACE(VICPSystem);

    virtual void registerListener(int32_t app_id, 
		const sp<IVICPListener>& listener) = 0;
    virtual void unregisterListener(int32_t app_id, 
		const sp<IVICPListener>& listener) = 0;
	virtual void send(int32_t app_id, uint8_t *buf, int32_t len) = 0;
};

class BnVICPSystem : public BnInterface<IVICPSystem> {
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data,
                                Parcel* reply, uint32_t flags = 0);
};

}; // namespace android

#endif // __IVICPSYSTEM_H__

