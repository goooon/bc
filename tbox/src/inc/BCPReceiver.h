#ifndef MQTT_GUARD_BCPReceiver_h__
#define MQTT_GUARD_BCPReceiver_h__

#include "../../../fundation/src/inc/fundation.h"
class 手机虚拟钥匙离开时自动锁车 : task {
	appid;
	sessionid;

	doorIndex;
	run(){
		WaitEvent;
	}

	handPackage(data, dagtalen)
	{
		dorrIndex = ;
		SetEvent;
		return;
	}
};
//////////////////////////////////////////////////////////////////////////
Appication
{
	FIFO Tasks[] = {new A01手机虚拟钥匙离开时自动锁车(appID,sessionID),new A02 };
	void onSessionPackage(applicationID,sessionID, pkgdata, pkgdatalen)
	{
	
	}
	callback
	{
		onConnected();
		onDisconnected();
		onRecvPackage(void* data, int len)
		{
			    sessionID =
				applicationID
				找到applicationID,session对应的task,
				bool done = task->handPackage(data, dagtalen);
				if (!done) {
					创建新的任务，放入队列
				}
		}
		onError();
	}
}
1.BCP_MQTT.connect(ip/port,subscribe,this);
2.初始化tasks队列
3.创建事件循环，从tasks队列取任务执行.
//////////////////////////////////////////////////////////////////////////
Task 使用FIFIO维护
  A01手机虚拟钥匙离开时自动锁车;sessionID 1
  A02...:sessionID 2
//////////////////////////////////////////////////////////////////////////
BCP BeeCloud Protocle

BCP_MQTT{
	
	///
	bool connect(ip / port,subscript,callback);
	bool sendPackage(void* data, int len);
	void disconnect();
}

BCP_WIFI
{

}

BCP_CAN
{

}
//////////////////////////////////////////////////////////////////////////
MQTT
调用MQTTClient.c的功能
#endif // MQTT_GUARD_BCPReceiver_h__
