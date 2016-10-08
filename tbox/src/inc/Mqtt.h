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
	virtual void onRecvPackage(void* data, int len) {}
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
	MqttHandler();
	~MqttHandler();
	bool reqConnect(char* url, char* topic,int qos);;
	bool reqSendPackage(void* data, int len,int qos);
	void reqDisconnect();
	bool isConnected();
private:
	bool changeSubstate(SubState next);
	bool changeState(State next);
public:
	virtual void onConnected()override;
	virtual void onSubscribed()override;
	virtual void onDisconnected()override;
	virtual void onRecvPackage(void* data, int len)override;
	virtual void onDeliveryComplete()override;
	virtual void onError(u32 ecode, char* emsg)override;
private:
	State state;
	SubState subState;
	void* client;
	const char* topicName;
};
#endif // GUARD_Mqtt_h__
