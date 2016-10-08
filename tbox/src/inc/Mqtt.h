#ifndef GUARD_Mqtt_h__
#define GUARD_Mqtt_h__
#include "./dep.h"
#include "./Task.h"
class IMqttHandler
{
protected:
	virtual void onConnected() {}
	virtual void onSubscribed(){}
	virtual void onDisconnected() {}
	virtual void onRecvPackage(void* data, int len) {}
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
	MqttHandler():state(Disconnected),subState(Unsubscribed){}
	bool reqConnect(u32 ip, u32 port, char* subscription);;
	bool reqSendPackage(void* data, int len);
	void reqDisconnect();
private:
	bool changeSubstate(SubState next);
	bool changeState(State next);
protected:
	virtual void onConnected()override;
	virtual void onSubscribed()override;
	virtual void onDisconnected()override;
	virtual void onRecvPackage(void* data, int len)override;
	virtual void onError(u32 ecode, char* emsg)override;
private:
	State state;
	SubState subState;
};
#endif // GUARD_Mqtt_h__
