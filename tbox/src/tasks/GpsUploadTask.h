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
	GpsUploadTask_NTF();
	static Task* Create();
protected:
	virtual void doTask();

	void sendGpsData(GPSDataQueue::GPSInfo info);

private:
	bool getGps(void* p,void* s, void *ch, GPSDataQueue::GPSInfo& data,Vehicle::RawGps& rawGps);
	bool ntfGps(GPSDataQueue::GPSInfo& info);
	bool ntfExitAbnormal();
	bool needSendAbnormalGps(Vehicle::RawGps& rawGps);
	double calcDistance(double long1, double lat1, Vehicle::RawGps& rawGps);
private:
	Timestamp normalToFire;
	double longPrev;
	double latiPrev;
	Timestamp abnormalPrev;
};

class GpsUploadTask : public Task
{
public:
	GpsUploadTask();
	static Task* Create();
protected:
	virtual void doTask();
private:
	bool ntfGps(Vehicle::RawGps& rawGps);
};
#endif // GUARD_GpsUploadTask_h__
