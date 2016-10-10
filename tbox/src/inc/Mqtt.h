#ifndef GUARD_Mqtt_h__
#define GUARD_Mqtt_h__
#include "./dep.h"
#include "./Task.h"
class IMqttHandler
{
public:
	virtual void onConnected(bool) {}
	virtual void onSubscribed(){}
	virtual void onDisconnected() {}
	virtual bool onRecvPackage(void* data, int len) { return true; }
	virtual void onDeliveryComplete(){}
	virtual void onError(u32 ecode, char* emsg){}
};
class MqttHandler : public IMqttHandler
{
	enum State{
		Disconnected = 0,
		Connecting,
		Unsubscribed,
		Subscribing,
		Subscribed,
		Disconnecting,
		Size
	};
public:
	enum ErrorCode
	{
		SubscribFailed
	};
public:
	MqttHandler();
	~MqttHandler();
	bool reqConnect(char* url, char* topic,int qos);;
	ThreadEvent::WaitResult reqSendPackage(void* payload, int payloadlen, int qos,int millSec);
	bool reqSendPackageAsync(void* payload, int payloadlen, int qos);
	bool reqDisconnect();
	bool isConnected();
	const char* getTopicName()const;
	bool onDebugCommand(char* cmd);
private:
	bool changeState(State next);
public:
	virtual void onConnected(bool succ)override;
	virtual void onSubscribed()override;
	virtual void onDisconnected()override;
	virtual bool onRecvPackage(void* data, int len)override;
	virtual void onDeliveryComplete()override;
	virtual void onError(u32 ecode, char* emsg)override;
private:
	State state;
	void* client;
	const char* topicName;
};
#endif // GUARD_Mqtt_h__
