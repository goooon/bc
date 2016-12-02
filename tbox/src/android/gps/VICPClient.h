#ifndef __VICPCLIENT_H__
#define __VICPCLIENT_H__
#include <stdint.h>
#include <binder/IInterface.h>

#include "IVICPListener.h"
#include "IVICPSystem.h"

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

namespace android {

class VICPListener : 
		public BnVICPListener,
		public IBinder::DeathRecipient
{
public:

private:
	void data_arrived(int32_t app_id, uint8_t *buf, int32_t len);
    void binderDied(const wp<IBinder>& who);
};
}  // namespace android

#endif // __VICPClient_H__
