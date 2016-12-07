#ifndef MQTT_GUARD_Config_h__
#define MQTT_GUARD_Config_h__

#include <string.h>
#include "./dep.h"
#include "../../../fundation/src/inc/fundation.h"

class Config
{
public:
	static Config& getInstance();
	void showCmdLine() {
		printf("-ip xx.xx.xx.xx:port\r\n");
		printf("-dto doorActivationTimeOut");
		printf("-pub publish_topic\r\n");
		printf("-sub subscription\r\n");
		printf("-cid clientid\r\n");
		printf("-vin vehicle vin\r\n");
		printf("-imei imei code,utf8 string,15 length\r\n");
		printf("-iccid iccid code,utf8 string,20 length\r\n");
		printf("-tsn tbox serial number code,utf8 string,20 length\r\n");
		printf("-gps on/off notify gps info\r\n");
		printf("-channels start channel for vicp\r\n");
		printf("-startOnAndroid start on android system for vicp\r\n");
	}
	void setDoorActivationTimeOut(u32 to) {
		doorActivationTimeOut = to;
	}
	void setDoorDeactiveTimeOut(u32 to) {
		doorDeactiveTimeOut = to;
	}
	void setIgntActivationTimeOut(u32 to){
		igntActivationTimeOut = to;
	}
	void setStateUploadExpireTime(u32 to) {
		stateUploadExpireTime = to;
	}
	void setGpsIntervalInDriving(u32 it) {
		gpsIntervalDriving = it;
	}
	void setGpsIntervalInStation(u32 it) {
		gpsIntervalStation = it;
	}
	void setAbnormalMovingDuration(u32 it) {
		gpsIntervalAbormal = it;
	}
	u32 getAbnormalMovingDuration() {
		return gpsIntervalAbormal;
	}
	u32 getDoorActivationTimeOut() {
		return doorActivationTimeOut;
	}
	u32 getDoorDeactiveTimeOut() {
		return doorDeactiveTimeOut;
	}
	u32 getIgntActivationTimeOut() {
		return igntActivationTimeOut;
	}
	u32 getStateUploadExpireTime() {
		return stateUploadExpireTime;
	}
	u32 getMqttReConnInterval() {
		return mqttReConnInterval;
	}
	u32 getAuthRetryInterval() {
		return authRetryInterval;
	}
	u32 getGPSQueueSize() {
		return gpsQueueSize;
	}
	u32 getAbnormalMovingStartTimeLimit() {
		return AbnormalMovingStartTimeLimit;
	}
	void setAbnormalMovingStartTimeLimit(u32 s) {
		AbnormalMovingStartTimeLimit = s;
	}
	u32 getAbnormalMovingStartDistanceLimit() {
		return AbnormalMovingStartDistanceLimit;
	}
	void setAbnormalMovingStartDistanceLimit(u32 d) {
		AbnormalMovingStartDistanceLimit = d;
	}
	u32 getAbnormalMovingStopTimeLimit() {
		return AbnormalMovingStopTimeLimit;
	}
	void setAbnormalMovingStopTimeLimit(u32 s) {
		AbnormalMovingStopTimeLimit = s;
	}
	void setAbnormalMovingStopDistanceLimit(u32 d) {
		AbnormalMovingStopDistanceLimit = d;
	}
	u32 getAbnormalMovingStopDistanceLimit() {
		return AbnormalMovingStopDistanceLimit;
	}
	u32 getCheckShakingInterval() {
		return checkShakingInterval;
	}
	void setCheckShakingInterval(u32 c) {
		checkShakingInterval = c;
	}
	bool isGpsTaskAtStartup() {
		return gpsTaskAtStartup;
	}
	/*u32 getAbnormalMovingDist() {
		return abnormalMovingDist;
	}*/
	/*u32 getDurationEnterNormal() {
		return durationEnterNormal;
	}*/
	u32 getMqttSendTimeOut() {
		return mqttSendTimeOut;
	}
	u32 getUnIgnitionNotifyDelay() {
		return unIgnitionNotifyDelay;
	}
	bool getIsGpsDataValid() {
		return isGpsDataValid;
	}
	void setIsGpsDataValid(bool b) {
		isGpsDataValid = b;
	}
	u32 getMqttDefaultQos() {
		return mqttQos;
	}
	bool getIsAutoCanBus() {
		return canAuto;
	}
	//u32 getAbnormalMovingInterval() {
	//	return abnormalMovingInterval;
	//}
	u32  getGpsInterval();
	u32  getAuthToken() { return authToken; }
	void setAuthToken(u32 t) { authToken = t; }
	bool isStartChannels() { return startChannel; }
	bool isStartAndroidServer() { return startAndroidServer; }
	bool isStartAndroidClient() { return startAndroidClient; }
public:
	bool parse(int argc, char** argv);
	const char* getPublishTopic() { return pub_topic; }
private:
public:
	u32  ip;
	u16  port;
	u32  mqttSendTimeOut;
	int  keepAliveInterval;
	bool isServer;
	char pub_topic[64];
	char sub_topic[64];
	char mqttServerIp[256];
	char vin[17];	//utf-8 string
	char tsn[20];
	char imei[15];
	char iccid[20];
	u32  authToken;
	bool startChannel;
	bool startAndroidServer;
	bool startAndroidClient;
	//////////////////////////////////////////////////////////////////////////
public:
	bool isGpsDataValid;
	bool gpsTaskAtStartup;
	bool canAuto;
	
	u32 unIgnitionNotifyDelay;
	u32 gpsQueueSize;
	u32 mqttQos;
	u32 mqttReConnInterval;
	u32 authRetryInterval;
	u32 gpsIntervalDriving;
	u32 gpsIntervalStation;
	u32 gpsIntervalAbormal;

	//u32 durationEnterNormal;
	u32 doorDeactiveTimeOut;
	u32 doorActivationTimeOut;
	u32 igntActivationTimeOut;
	u32 stateUploadExpireTime;

	u32 checkShakingInterval;

	u32 AbnormalMovingStartTimeLimit;
	u32 AbnormalMovingStartDistanceLimit;
	u32 AbnormalMovingStopTimeLimit;
	u32 AbnormalMovingStopDistanceLimit;
};
#endif // GUARD_Config_h__
