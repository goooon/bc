#include "../inc/Mqtt.h"
#include "../../../dep/paho/src/MQTTClient.h"

MqttHandler::MqttHandler() :state(Disconnected), subState(Unsubscribed),client(0)
{
	MQTTClient_nameValue* info = MQTTClient_getVersionInfo();
	LOG_I("Mqtt name:%s value:%s", info->name, info->value);
	
}

MqttHandler::~MqttHandler()
{
	if (client != NULL) {
		MQTTClient_destroy(&client);
		client = 0;
	}
}
static void Client_connectionLost(void* context, char* cause)
{
	MqttHandler* mh = (MqttHandler*)context;
	LOG_I("MQTT Client_connectionLost");
	mh->onDisconnected();
}
static int Client_messageArrived(void* context, char* topicName, int topicLen, MQTTClient_message* message)
{
	MqttHandler* mh = (MqttHandler*)context;
	if (message == NULL) {
		LOG_E("Client_messageArrived Param Error");
		return false;
	}
	LOG_I("MQTT Client_messageArrived");
	mh->onRecvPackage(message->payload, message->payloadlen);
	return true;
}
static void Client_deliveryComplete(void* context, MQTTClient_deliveryToken dt)
{
	MqttHandler* mh = (MqttHandler*)context;
	LOG_I("MQTT Client_deliveryComplete");
	mh->onDeliveryComplete();
}
void trace_callback(enum MQTTASYNC_TRACE_LEVELS level, char* message)
{
	LOG_I("Trace : %d, %s\n", level, message);
}
bool MqttHandler::reqConnect(char* url, char* topic,int qos)
{
	LOG_I("MqttHandler::reqConnect(%s,%s,%d)",url,topic,qos);

	MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
	MQTTClient_willOptions wopts = MQTTClient_willOptions_initializer;

	int rc = MQTTClient_create(&client, url, "single_threaded_test",MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);
	
	if (rc != MQTTCLIENT_SUCCESS)
	{
		LOG_E("MQTTClient_create Failed %d", rc);
		MQTTClient_destroy(&client);
		client = NULL;
		return false;
	}
	
	opts.keepAliveInterval = 20;
	opts.cleansession = 1;
	opts.username = "testuser";
	opts.password = "testpassword";
	opts.MQTTVersion = MQTTVERSION_DEFAULT;
	/*if (options.haconnections != NULL)
	{
		opts.serverURIs = options.haconnections;
		opts.serverURIcount = options.hacount;
	}*/

	opts.will = &wopts;
	opts.will->message = "will message";
	opts.will->qos = 1;
	opts.will->retained = 0;
	opts.will->topicName = "will topic";
	opts.will = NULL;

	int ret = MQTTClient_connect(client, &opts);
	if (MQTTCLIENT_SUCCESS != ret) {
		LOG_E("MQTTClient_connect failed");
		return false;
	}

	ret = MQTTClient_setCallbacks(client,this, Client_connectionLost,Client_messageArrived, Client_deliveryComplete);

	ret = MQTTClient_subscribe(client, topic, qos);
	if (MQTTCLIENT_SUCCESS != ret) {
		LOG_E("MQTTClient_subscribe failed");
		return false;
	}
	return true;
}

bool MqttHandler::reqSendPackage(void* payload, int payloadlen,int qos)
{
	MQTTClient_deliveryToken dt;
	int ret;
	int retained = 0;
	ret = MQTTClient_publish(client, topicName, payloadlen, payload, qos, retained, &dt);
	if (MQTTCLIENT_SUCCESS != ret) {
		LOG_E("MQTTClient_publish failed %d",ret);
		ret = false;
	}

	if (qos > 0){
		ret = MQTTClient_waitForCompletion(client, dt, 5000L);
		if (MQTTCLIENT_SUCCESS != ret) {
			LOG_E("MQTTClient_waitForCompletion failed %d", ret);
			//return false;
			ret = false;
		}
	}
	return ret;
}

void MqttHandler::reqDisconnect()
{
	int timeout = 0;
	int ret = MQTTClient_disconnect(client, timeout);
	if (MQTTCLIENT_SUCCESS != ret) {
		LOG_E("MQTTClient_disconnect failed %d", ret);
	}
}

bool MqttHandler::isConnected()
{
	return true == MQTTClient_isConnected(client);
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
	LOG_E("Error:%d,%s", ecode, emsg);
}

void MqttHandler::onRecvPackage(void* data, int len)
{
	u16 sessionID = 0;
	u16 applicationID = 0;
	Task* task = nullptr;
	//	找到applicationID, session对应的task,
	//task = find(applicationID, sessionID);
	if (task != nullptr) {
		bool done = task->handlePackage(data, len);
		if (!done) {
			//创建新的任务，放入队列
		}
	}
}

void MqttHandler::onDeliveryComplete()
{

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
