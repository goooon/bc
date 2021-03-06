#ifndef GUARD_GpsUploadTask_h__
#define GUARD_GpsUploadTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "./BCMessage.h"
#include "./TaskTable.h"
#include "./GPSDataQueue.h"
#include "../inc/Vehicle.h"
class GpsUploadTask_NTF : public Task
{
public:
	GpsUploadTask_NTF(u32 appId);
	static Task* Create(u32 appId);
protected:
	virtual void doTask();

	void sendGpsData(GPSDataQueue::GPSInfo info);

private:
	bool getGps(void* p, GPSDataQueue::GPSInfo& data,Vehicle::RawGps& rawGps);
	bool ntfGps(GPSDataQueue::GPSInfo& info);
	bool ntfEnterAbnormal();
	bool ntfExitAbnormal();
	bool needSendAbnormalGps(Vehicle::RawGps& rawGps);
	double calcDistance(double long1, double lat1, Vehicle::RawGps& rawGps);
	void sendGps(GPSDataQueue::GPSInfo& info, Vehicle::RawGps & raw);
private:
	Timestamp normalToFire;
	double longPrev;
	double latiPrev;
	Timestamp abnormalPrev;
};

class GpsUploadTask : public Task
{
public:
	GpsUploadTask(u32 appId);
	static Task* Create(u32 appId);
protected:
	virtual void doTask();
private:
	bool ntfGps(Vehicle::RawGps& rawGps);
};
#endif // GUARD_GpsUploadTask_h__
