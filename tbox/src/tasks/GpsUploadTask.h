#ifndef GUARD_GpsUploadTask_h__
#define GUARD_GpsUploadTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "./BCMessage.h"
#include "./TaskTable.h"
#include "./GPSDataQueue.h"
class GpsUploadTask_NTF : public Task
{
public:
	GpsUploadTask_NTF();
	static Task* Create();
protected:
	virtual void doTask();
private:
	bool getGps(void* p,void* s,GPSData& data);
	bool ntfGps(GPSData& data);
private:
	Timestamp fire;
};
#endif // GUARD_GpsUploadTask_h__
