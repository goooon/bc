#include "../inc/Mqtt.h"
#include "../../../dep/paho/src/MQTTAsync.h"
#undef TAG
#define TAG "MQTT"
void trace_callback(enum MQTTASYNC_TRACE_LEVELS level, char* message)
{
	switch (level)
	{
	case MQTTASYNC_TRACE_MAXIMUM:
		LOG_P("%s\r\n", message);
		break;
	case MQTTASYNC_TRACE_MEDIUM:
		LOG_P("%s\r\n", message);
		break;
	case MQTTASYNC_TRACE_MINIMUM:
		LOG_P("%s\r\n", message);
		break;
	case MQTTASYNC_TRACE_PROTOCOL:
		LOG_P("%s\r\n", message);
		break;
	case MQTTASYNC_TRACE_ERROR:
		LOG_P("%s\r\n", message);
		break;
	case MQTTASYNC_TRACE_SEVERE:
		LOG_P("%s\r\n", message);
		break;
	case MQTTASYNC_TRACE_FATAL:
		LOG_P("%s\r\n", message);
		break;
	default:
		break;
	}
}

bool MqttHandler::onDebugCommand(char* cmd)
{
	if (!strcmp(cmd, "MAXIMUM")) {
		MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_MAXIMUM);
		return true;
	}
	if (!strcmp(cmd, "MEDIUM")) {
		MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_MEDIUM);
		return true;
	}
	if (!strcmp(cmd, "MINIMUM")) {
		MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_MINIMUM);
		return true;
	}
	if (!strcmp(cmd, "PROTOCOL")) {
		MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_PROTOCOL);
		return true;
	}
	if (!strcmp(cmd, "ERROR")) {
		MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_ERROR);
		return true;
	}
	return false;
}

MqttHandler::MqttHandler() :state(Disconnected), subState(Unsubscribed),client(0), topicName("async test topic")
{
	MQTTAsync_nameValue* info = MQTTAsync_getVersionInfo();
	MQTTAsync_setTraceCallback(trace_callback);
	LOG_I("Mqtt name:%s value:%s", info->name, info->value);
	MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_MAXIMUM);
}

MqttHandler::~MqttHandler()
{
	if (client != NULL) {
		MQTTAsync_destroy(&client);
		client = 0;
	}
}
static void Client_connectionLost(void* context, char* cause)
{
	MqttHandler* mh = (MqttHandler*)context;
	LOG_I("MQTT Client_connectionLost: %s",cause ? cause : "unknown");
	mh->onDisconnected();
}
static int Client_messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
{
	MqttHandler* mh = (MqttHandler*)context;
	LOG_I("MQTT Client_messageArrived: %s %d", topicName,topicLen);
	return mh->onRecvPackage(message->payload,message->payloadlen);
}
static void Client_deliveryComplete(void* context, MQTTAsync_token token)
{
	MqttHandler* mh = (MqttHandler*)context;
	LOG_I("MQTT Client_deliveryComplete:token %d", token);
	mh->onDeliveryComplete();
}
void Mqtt_onSubscribFailed(void* context, MQTTAsync_failureData* response)
{
	MqttHandler* c = (MqttHandler*)context;
	
	if (response) {
		LOG_W("Mqtt_onConnectFailed %p %d %d %s", c,response->code,response->token,response->message);
		c->onError(MqttHandler::ErrorCode::SubscribFailed, response->message);
	}
	else {
		LOG_W("Mqtt_onConnectFailed %p", c);
		c->onError(MqttHandler::ErrorCode::SubscribFailed, "Mqtt_onConnectFailed");
	}
}
void Mqtt_onSubscribed(void* context, MQTTAsync_successData* response)
{
	MqttHandler* c = (MqttHandler*)context;

	LOG_I("Mqtt_onSubscribed qos %d", response->alt.qos);

	c->onSubscribed();
}
void Mqtt_onConnectFailed(void* context, MQTTAsync_failureData* response)
{
	MqttHandler* c = (MqttHandler*)context;
	LOG_W("Mqtt_onConnectFailed %p", c);
	if (response) {
		c->onError(response->code, response->message);
	}
	else {
		c->onError(0, "Mqtt_onConnectFailed");
	}
}
void Mqtt_onConnected(void* context, MQTTAsync_successData* response)
{
	MqttHandler* c = (MqttHandler*)context;
	c->onConnected();
}

bool MqttHandler::reqConnect(char* url, char* topic,int qos)
{
	LOG_I("MqttHandler::reqConnect(%s,%s,%d)",url,topic,qos);
	if (state == Connected) {
		LOG_W("MqttHandler state not right: Already Connected");
		return false;
	}

	MQTTAsync_connectOptions opts = MQTTAsync_connectOptions_initializer;

	int rc = MQTTAsync_create(&client, url, "async test topic",
		MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);
	
	if (rc != MQTTASYNC_SUCCESS)
	{
		LOG_E("MQTTClient_create Failed %d", rc);
		MQTTAsync_destroy(&client);
		client = NULL;
		return false;
	}
	rc = MQTTAsync_setCallbacks(client, this, Client_connectionLost, Client_messageArrived, Client_deliveryComplete);

	opts.keepAliveInterval = 20;
	opts.username = "testuser";
	opts.password = "testpassword";
	opts.MQTTVersion = 0;

	opts.onFailure = Mqtt_onConnectFailed;
	opts.context = this;

	opts.cleansession = 1;
	opts.onSuccess = Mqtt_onConnected;
	rc = MQTTAsync_connect(client, &opts);

	if (MQTTASYNC_SUCCESS != rc) {
		LOG_E("MQTTClient_connect failed");
		return false;
	}
	return true;
}

ThreadEvent::WaitResult MqttHandler::reqSendPackage(void* payload, int payloadlen, int qos, int millSec)
{
	int retained = 0;
	MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
	int rc = MQTTAsync_send(client, topicName, payloadlen, payload, qos, retained, &ropts);
	if (MQTTASYNC_SUCCESS != rc) {
		LOG_E("reqSendPackage() failed %d", rc);
		return ThreadEvent::WaitResult::Errors;
	}
	LOG_I("Token was %d", ropts.token);
	rc = MQTTAsync_waitForCompletion(client, ropts.token, millSec);
	if (MQTTASYNC_SUCCESS != rc) {
		rc = MQTTAsync_isComplete(client, ropts.token);
		if (MQTTASYNC_TRUE != rc) {
			LOG_W("MQTTAsync_waitForCompletion() timeout %d", rc);
			return ThreadEvent::WaitResult::TimeOut;
		}
		else {
			LOG_W("MQTTAsync_waitForCompletion() failed %d", rc);
			return ThreadEvent::WaitResult::Errors;
		}
	}
	return ThreadEvent::WaitResult::EventOk;
}

bool MqttHandler::reqSendPackageAsync(void* payload, int payloadlen, int qos)
{
	int retained = 0;
	MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
	int rc = MQTTAsync_send(client, topicName, payloadlen, payload, qos, retained, &ropts);
	if (MQTTASYNC_SUCCESS != rc) {
		LOG_E("reqSendPackage() failed %d", rc);
		return false;
	}
	LOG_I("Token was %d", ropts.token);
	return true;
}

void Mqtt_onDisconnected(void* context, MQTTAsync_successData* response)
{
	MqttHandler* c = (MqttHandler*)context;
	LOG_I("Mqtt_onDisconnected %p", c);
	c->onDisconnected();
}
void Mqtt_onDisconnectFailed(void* context, MQTTAsync_failureData* response)
{
	MqttHandler* c = (MqttHandler*)context;
	LOG_I("Mqtt_onDisconnectFailed %p", c);
	if (response) {
		c->onError(response->code, response->message);
	}
	else {
		c->onError(0, "test8_onFailure");
	}
}
void MqttHandler::reqDisconnect()
{
	MQTTAsync_disconnectOptions dopts = MQTTAsync_disconnectOptions_initializer;
	int timeout = 0;
	/* disconnect immediately without completing the commands */
	dopts.timeout = 0;
	dopts.onSuccess = Mqtt_onDisconnected;
	dopts.onFailure = Mqtt_onDisconnectFailed;
	dopts.context = this;
	int rc = MQTTAsync_disconnect(client, &dopts); /* now there should be incomplete commands */
	if (MQTTASYNC_SUCCESS != rc) {
		LOG_E("reqDisconnect() failed %d", rc);
	}
}

bool MqttHandler::isConnected()
{
	return true == MQTTAsync_isConnected(client);
}

const char* MqttHandler::getTopicName() const
{
	return topicName;
}

bool MqttHandler::changeSubstate(SubState next)
{
	const char* ss[] = { "Unsubscribed","Subscribed" };
	LOG_I("Mqtt:%s->%s", ss[subState], ss[next]);
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
	const char* ss[] = { "Connected","Disconnected" };
	LOG_I("Mqtt:%s->%s", ss[state], ss[next]);
	switch (next)
	{
	case MqttHandler::Connected:
		if (state != Disconnected) {
			LOG_W("MqttHandler state(%d) not right", state);
			return false;
		}
		state = Connected;
		return true;
		break;
	case MqttHandler::Disconnected:
		if (state != Connected) {
			LOG_W("MqttHandler state(%d) not right", state);
			return false;
		}
		state = Disconnected;
		return true;
		break;
	default:
		break;
	}
	return false;
}

void MqttHandler::onConnected()
{
	//LOG_I("MqttHandler::onConnected()");
	if (changeState(Connected)) {
		MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
		int rc;
		//LOG_I("Mqtt_onConnected");
		opts.onSuccess = Mqtt_onSubscribed;
		opts.onFailure = Mqtt_onSubscribFailed;
		opts.context = this;
		rc = MQTTAsync_subscribe(client, topicName, 2, &opts);
		if (rc != MQTTASYNC_SUCCESS)
		{
			LOG_W("MQTTAsync_subscribe Fail %d", rc);
			onError(rc, "MQTTAsync_subscribe");
		}
	}
	else {
		LOG_W("changeState failed");
	}
}

void MqttHandler::onError(u32 ecode,char* emsg)
{
	LOG_E("Error:%d,%s", ecode, emsg);
}

bool MqttHandler::onRecvPackage(void* data, int len)
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
	return true;
}

void MqttHandler::onDeliveryComplete()
{
	LOG_I("onDeliveryComplete");
}

void MqttHandler::onDisconnected()
{
	LOG_I("MqttHandler::onDisconnected()");
	if (changeState(Disconnected)) {}
	else {
		LOG_W("changeState failed");
	}
}

void MqttHandler::onSubscribed()
{
	//LOG_I("MqttHandler::onSubscribed()");
	if (changeSubstate(Subscribed)) {}
	else {
		LOG_W("changeSubstate failed");
	}
}
