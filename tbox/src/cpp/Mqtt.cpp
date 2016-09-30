#include "../inc/Mqtt.h"

bool MqttHandler::reqConnect(u32 ip, u32 port, char* subscript)
{
	return true;
}

bool MqttHandler::reqSendPackage(void* data, int len)
{
	return false;
}

void MqttHandler::reqDisconnect()
{
	return;
}

bool MqttHandler::changeSubstate(SubState next)
{
	LOG_I("Mqtt:%d->%d", subState, next);
	switch (next)
	{
	case MqttHandler::Unsubscribed:
		if (subState == Unsubscribed) {
			LOG_W("subState %d not right", subState);
			return false;
		}
		subState = next;
		return true;
		break;
	case MqttHandler::Subscribed:
		if (subState == Subscribed) {
			LOG_W("subState %d not right", subState);
			return false;
		}
		subState = next;
		return true;
		break;
	default:
		LOG_E("next subState %d not right", next);
		break;
	}
	return false;
}

bool MqttHandler::changeState(State next)
{
	LOG_I("Mqtt:%d->%d", state, next);
	switch (next)
	{
	case MqttHandler::Connected:
		if (state != Disconnected) {
			LOG_W("MqttHandler state(%d) not right", state);
			return false;
		}
		state = Disconnected;
		return true;
		break;
	case MqttHandler::Disconnected:
		if (state != Connected) {
			LOG_W("MqttHandler state(%d) not right", state);
			return false;
		}
		state = Connected;
		return true;
		break;
	default:
		break;
	}
	return false;
}

void MqttHandler::onConnected()
{
	LOG_I("MqttHandler::onConnected()");
	if (changeState(Connected)) {}
	else {}
}

void MqttHandler::onError(u32 ecode,char* emsg)
{

}

void MqttHandler::onRecvPackage(void* data, int len)
{
	u16 sessionID = 0;
	u16 applicationID = 0;
	Task* task = nullptr;
	//	找到applicationID, session对应的task,
	if (task != nullptr) {
		bool done = task->handlePackage(data, len);
		if (!done) {
			//创建新的任务，放入队列
		}
	}
}

void MqttHandler::onDisconnected()
{
	LOG_I("MqttHandler::onDisconnected()");
	if (changeState(Connected)) {}
	else {}
}

void MqttHandler::onSubscribed()
{
	LOG_I("MqttHandler::onSubscribed()");
	if (changeSubstate(Subscribed)) {}
	else {}
}
