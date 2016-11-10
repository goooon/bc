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
	bool getGps(void* p,void* s, AutoLocation& data);
	bool ntfGps(AutoLocation& data);
private:
	Timestamp fire;
};

class GpsUploadTask : public Task
{
public:
	GpsUploadTask();
	static Task* Create();
protected:
	virtual void doTask();
private:
	bool ntfGps(AutoLocation& data);
};
#endif // GUARD_GpsUploadTask_h__
