#ifndef GUARD_Mqtt_h__
#define GUARD_Mqtt_h__
#include "./dep.h"
#include "./Task.h"
class IMqttHandler
{
public:
	virtual void onConnected() {}
	virtual void onSubscribed(){}
	virtual void onDisconnected() {}
	virtual bool onRecvPackage(void* data, int len) { return true; }
	virtual void onDeliveryComplete(){}
	virtual void onError(u32 ecode, char* emsg){}
};
class MqttHandler : public IMqttHandler
{
	enum State{
		Connected,
		Disconnected
	};
	enum SubState {
		Unsubscribed,
		Subscribed
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
	void reqDisconnect();
	bool isConnected();
	const char* getTopicName()const;
	bool onDebugCommand(char* cmd);
private:
	bool changeSubstate(SubState next);
	bool changeState(State next);
public:
	virtual void onConnected()override;
	virtual void onSubscribed()override;
	virtual void onDisconnected()override;
	virtual bool onRecvPackage(void* data, int len)override;
	virtual void onDeliveryComplete()override;
	virtual void onError(u32 ecode, char* emsg)override;
private:
	State state;
	SubState subState;
	void* client;
	const char* topicName;
};
#endif // GUARD_Mqtt_h__
