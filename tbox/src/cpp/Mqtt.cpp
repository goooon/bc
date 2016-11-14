#include <string.h>
#include "../inc/Mqtt.h"
#include "../inc/Event.h"
#include "../../../dep/paho/src/MQTTAsync.h"
#include "../inc/Application.h"
#include "../tasks/VKeyActiveTask.h"
#undef TAG
#define TAG "MQTT"
void trace_callback(enum MQTTASYNC_TRACE_LEVELS level, char* message)
{
	switch (level)
	{
	case MQTTASYNC_TRACE_MAXIMUM:
		LOG_P("T:MAXIMUM %s\r\n", message);
		break;
	case MQTTASYNC_TRACE_MEDIUM:
		LOG_P("T:MEDIUM  %s\r\n", message);
		break;
	case MQTTASYNC_TRACE_MINIMUM:
		LOG_P("T:MINIMUM %s\r\n", message);
		break;
	case MQTTASYNC_TRACE_PROTOCOL:
		LOG_P("T:PROTOCOL %s\r\n", message);
		break;
	case MQTTASYNC_TRACE_ERROR:
		LOG_P("T:ERROR  %s\r\n", message);
		break;
	case MQTTASYNC_TRACE_SEVERE:
		LOG_P("T:SEVERE  %s\r\n", message);
		break;
	case MQTTASYNC_TRACE_FATAL:
		LOG_P("T:FATAL  %s\r\n", message);
		break;
	default:
		break;
	}
}

bool MqttClient::onDebugCommand(const char* cmd)
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
	if (!strcmp(cmd, "connMqtt")) {
		return true;
	}
	if (!strcmp(cmd, "discMqtt")) {
		return true;
	}
	return false;
}

static MqttClient* g_client;
MqttClient& MqttClient::getInstance()
{
	return *g_client;
}

MqttClient::MqttClient() :state(Disconnected), client(0)
{
	MQTTAsync_nameValue* info = MQTTAsync_getVersionInfo();
	MQTTAsync_setTraceCallback(trace_callback);
	LOG_I("Mqtt name:%s value:%s", info->name, info->value);
	MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_MAXIMUM);
	g_client = this;
}

MqttClient::~MqttClient()
{
	if (client != NULL) {
		MQTTAsync_destroy(&client);
		client = 0;
	}
}

static void Client_connectionLost(void* context, char* cause)
{
	MqttClient* mh = (MqttClient*)context;
	LOG_I("MQTT Client_connectionLost: %s",cause ? cause : "unknown");
	mh->onDisconnected();
}

static int Client_messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
{
	MqttClient* mh = (MqttClient*)context;
	LOG_I("MQTT Client_messageArrived: %s", topicName);
	int ret = mh->onRecvPackage(message->payload,message->payloadlen);
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	return ret;
}

static void Client_deliveryComplete(void* context, MQTTAsync_token token)
{
	MqttClient* mh = (MqttClient*)context;
	LOG_I("MQTT Client_deliveryComplete:token %d", token);
	mh->onDeliveryComplete();
}

void Mqtt_onSubscribFailed(void* context, MQTTAsync_failureData* response)
{
	MqttClient* c = (MqttClient*)context;
	
	if (response) {
		LOG_W("Mqtt_onConnectFailed %p %d %d %s", c,response->code,response->token,response->message);
		c->onError(MqttClient::SubscribFailed, response->message);
	}
	else {
		LOG_W("Mqtt_onConnectFailed %p", c);
		c->onError(MqttClient::SubscribFailed, "Mqtt_onConnectFailed");
	}
}

void Mqtt_onSubscribed(void* context, MQTTAsync_successData* response)
{
	MqttClient* c = (MqttClient*)context;

	LOG_I("Mqtt_onSubscribed qos %d", response->alt.qos);

	c->onSubscribed();
}

void Mqtt_onConnectFailed(void* context, MQTTAsync_failureData* response)
{
	MqttClient* c = (MqttClient*)context;
	LOG_W("Mqtt_onConnectFailed %p", c);
	c->onConnected(false);
	if (response) {
		c->onError(response->code, response->message);
	}
	else {
		c->onError(0, "Mqtt_onConnectFailed");
	}
}
void Mqtt_onConnected(void* context, MQTTAsync_successData* response)
{
	MqttClient* c = (MqttClient*)context;
	c->onConnected(true);
}

void MqttClient::setConfig(Config *c)
{
	topicName = c->pub_topic;
}

bool MqttClient::reqConnect(char* url, char* topic,int qos,int keepAliveInterval,const char* clientId)
{
	if (!changeState(Connecting)) {
		return false;
	}

	topicName = topic;
	LOG_I("MqttHandler::reqConnect(%s,%s,%d,%d)",url,topic,qos, keepAliveInterval);

	MQTTAsync_connectOptions opts = MQTTAsync_connectOptions_initializer;

	int rc = MQTTAsync_create(&client, url, clientId,
		MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);
	
	if (rc != MQTTASYNC_SUCCESS)
	{
		LOG_E("MQTTClient_create Failed %d", rc);
		MQTTAsync_destroy(&client);
		client = NULL;
		changeState(Disconnected);
		return false;
	}
	rc = MQTTAsync_setCallbacks(client, this, Client_connectionLost, Client_messageArrived, Client_deliveryComplete);

	opts.keepAliveInterval = keepAliveInterval;
	opts.username = "test";
	opts.password = "test123";
	opts.MQTTVersion = 0;

	opts.onFailure = Mqtt_onConnectFailed;
	opts.context = this;

	opts.cleansession = true;
	opts.onSuccess = Mqtt_onConnected;
	rc = MQTTAsync_connect(client, &opts);

	if (MQTTASYNC_SUCCESS != rc) {
		LOG_E("MQTTClient_connect failed");
		changeState(Disconnected);
		return false;
	}
	return true;
}

ThreadEvent::WaitResult MqttClient::reqSendPackage(const char* publish, void* payload, int payloadlen, int qos, int millSec)
{
	int retained = 0;
	MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
	LOG_I(">>>>reqSendPackage \"%s\" qos:%d Token was %d", publish, qos, ropts.token);
	int rc = MQTTAsync_send(client, publish, payloadlen, payload, qos, retained, &ropts);
	if (MQTTASYNC_SUCCESS != rc) {
		LOG_E("reqSendPackage() failed %d", rc);
		return ThreadEvent::Errors;
	}
	//LOG_I("MQTTAsync_waitForCompletion ...");
	rc = MQTTAsync_waitForCompletion(client, ropts.token, millSec);
	//LOG_I("MQTTAsync_waitForCompletion done");
	if (MQTTASYNC_SUCCESS != rc) {
		rc = MQTTAsync_isComplete(client, ropts.token);
		if (MQTTASYNC_TRUE != rc) {
			LOG_W("MQTTAsync_waitForCompletion() timeout %d", rc);
			return ThreadEvent::TimeOut;
		}
		/*else {
			LOG_W("MQTTAsync_waitForCompletion() failed %d", rc);
			return ThreadEvent::Errors;
		}*/
	}
	//LOG_I("MQTTAsync_waitForCompletion ret");
	return ThreadEvent::EventOk;
}

void SendPackageAsync_onSuccess(void* context, MQTTAsync_successData* response)
{
	typedef void(*OnResult)(bool);
	OnResult r = (OnResult)context;
	r(true);
}

void SendPackageAsync_onFailure(void* context, MQTTAsync_failureData* response)
{
	typedef void(*OnResult)(bool);
	OnResult r = (OnResult)context;
	r(false);
}

bool MqttClient::reqSendPackageAsync(void* payload, int payloadlen, int qos, void(*onResult)(bool))
{
	if (!isConnected()) {
		LOG_E("mqtt is disconnected");
		return false;
	}
	int retained = 0;
	MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
	ropts.onSuccess = SendPackageAsync_onSuccess;
	ropts.onFailure = SendPackageAsync_onFailure;
	ropts.context = (void*)onResult;
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
	MqttClient* c = (MqttClient*)context;
	LOG_I("Mqtt_onDisconnected %p", c);
	c->onDisconnected();
}

void Mqtt_onDisconnectFailed(void* context, MQTTAsync_failureData* response)
{
	MqttClient* c = (MqttClient*)context;
	LOG_I("Mqtt_onDisconnectFailed %p", c);
	if (response) {
		c->onError(response->code, response->message);
	}
	else {
		c->onError(0, "test8_onFailure");
	}
}

bool MqttClient::reqDisconnect()
{
	if (!changeState(Disconnecting)) {
		return false;
	}
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
	return true;
}

bool MqttClient::isConnected()
{
	return 1 == MQTTAsync_isConnected(client);
}

const char* MqttClient::getTopicName() const
{
	return topicName;
}

bool MqttClient::changeState(State next)
{
	const static bool st[MqttClient::Size][MqttClient::Size] = {
	//Disconnected,Connecting,Connected,Unsubscribed,Subscribing,Subscribed,Disconnecting,
		{false,    true,      /*false,*/    false,       false,      false,     false},
		{true,     false,     /*true, */    true,        false,      false,     false},
		//{true,     false,     /*false,*/    false,       false,      false,     true },
		{true,     false,     /*false,*/    true,		 true,       false,     true },
		{true,     false,     /*false,*/    true,		 false,      true,      true },
		{true,     false,     /*false,*/    true,		 false,      false,     true },
		{true,     false,     /*false,*/    false,       false,      false,     false}
	};
	const char* ss[] = {"Disconnected","Connecting",/*"Connected",*/"Unsubscribed","Subscribing","Subscribed","Disconnecting"};
    if (st[state][next]) {
		if (state != next) {
			LOG_I("Mqtt:%s -> %s", ss[state], ss[next]);
			State prev = state;
			state = next;
			PostEvent(AppEvent::MqttStateChanged, prev, state, 0);
		}
		return true;
	}
	else {
		LOG_W("Mqtt:%s +> %s", ss[state], ss[next]);
		return false;
	}
}

void MqttClient::onConnected(bool succ)
{
	//LOG_I("MqttHandler::onConnected()");
	if (succ) {
		if (changeState(Unsubscribed)) {
			if (!changeState(Subscribing)) {
				LOG_W("changeState failed");
				return;
			}
			MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
			int rc;
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
	else {
		changeState(Disconnected);
	}
}

void MqttClient::onError(u32 ecode,const char* emsg)
{
	LOG_E("Error:%d,%s", ecode, emsg);
}

bool MqttClient::onRecvPackage(void* data, int len)
{
	u16 sessionID = 0;
	u16 applicationID = 0;
	Task* task = NULL;
	bcp_packet_t *p;
	if (bcp_packet_unserialize((u8*)data, (u32)len, &p) < 0) {
		LOG_E("bcp_packet_unserialize failed");
		return false;
	}
	//找到applicationID, session对应的task,
	bcp_message_t *m = NULL;
	bcp_element_t *e = NULL;

	//bcp_messages_foreach(p, bcp_message_foreach_callback, NULL);

	while ((m = bcp_next_message(p, m)) != NULL) {
		applicationID = m->hdr.id;
		u64 seqId = m->hdr.sequence_id;
		task = Application::getInstance().findTask(applicationID);
		if (task != NULL) {
			bool done = task->handlePackage(p);
			if (!done) {
				//创建新的任务，放入队列
				::PostEvent(AppEvent::AbortTasks, applicationID, 0, p);
			}
		}
		else {
			if(applicationID != APPID_AUTHENTICATION)
				::PostEvent(AppEvent::InsertTask, 0, 0, TaskCreate(applicationID,p));
		}
	}
	return true;
}

void MqttClient::onDeliveryComplete()
{
	LOG_I("onDeliveryComplete");
}

void MqttClient::onDisconnected()
{
	LOG_I("MqttHandler::onDisconnected()");
	if (!changeState(Disconnected)) {
		LOG_W("changeState failed");
	}
}

void MqttClient::onSubscribed()
{
	//LOG_I("MqttHandler::onSubscribed()");
	if (changeState(Subscribed)) {}
	else {
		LOG_W("changeSubstate failed");
	}
}
