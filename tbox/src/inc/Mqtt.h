#ifndef GUARD_Mqtt_h__
#define GUARD_Mqtt_h__
#include "./dep.h"
#include "./Task.h"
#include "Config.h"
class IMqttHandler
{
public:
	virtual void onConnected(bool) {}
	virtual void onSubscribed(){}
	virtual void onDisconnected() {}
	virtual bool onRecvPackage(void* data, int len) { return true; }
	virtual void onDeliveryComplete(){}
	virtual void onError(u32 ecode, const char* emsg){}
};
class MqttClient : public IMqttHandler
{
public:
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
	static MqttClient& getInstance();
	MqttClient();
	~MqttClient();
	void setConfig(Config *c);
	bool reqConnect(char* url, char* topic,int qos,int keepAliveInterval,const char* clientId = "cliend id");;
	ThreadEvent::WaitResult reqSendPackage(const char* publish,void* payload, int payloadlen, int qos,int millSec);
	bool reqSendPackageAsync(void* payload, int payloadlen, int qos,void (*onResult)(bool));
	bool reqDisconnect();
	bool isConnected();
	const char* getTopicName()const;
	bool onDebugCommand(const char* cmd);
private:
	bool changeState(State next);
public:
	virtual void onConnected(bool succ)OVERRIDE;
	virtual void onSubscribed()OVERRIDE;
	virtual void onDisconnected()OVERRIDE;
	virtual bool onRecvPackage(void* data, int len)OVERRIDE;
	virtual void onDeliveryComplete()OVERRIDE;
	virtual void onError(u32 ecode, const char* emsg)OVERRIDE;
private:
	State state;
	void* client;
	char *topicName;
};
#endif // GUARD_Mqtt_h__
