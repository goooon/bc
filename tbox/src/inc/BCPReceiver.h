#ifndef MQTT_GUARD_BCPReceiver_h__
#define MQTT_GUARD_BCPReceiver_h__

#include "../../../fundation/src/inc/fundation.h"
class �ֻ�����Կ���뿪ʱ�Զ����� : task {
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
	FIFO Tasks[] = {new A01�ֻ�����Կ���뿪ʱ�Զ�����(appID,sessionID),new A02 };
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
				�ҵ�applicationID,session��Ӧ��task,
				bool done = task->handPackage(data, dagtalen);
				if (!done) {
					�����µ����񣬷������
				}
		}
		onError();
	}
}
1.BCP_MQTT.connect(ip/port,subscribe,this);
2.��ʼ��tasks����
3.�����¼�ѭ������tasks����ȡ����ִ��.
//////////////////////////////////////////////////////////////////////////
Task ʹ��FIFIOά��
  A01�ֻ�����Կ���뿪ʱ�Զ�����;sessionID 1
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
����MQTTClient.c�Ĺ���
#endif // MQTT_GUARD_BCPReceiver_h__
