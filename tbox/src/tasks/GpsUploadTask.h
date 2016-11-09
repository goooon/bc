#ifndef GUARD_GpsUploadTask_h__
#define GUARD_GpsUploadTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "./BCMessage.h"
#include "./TaskTable.h"

class GpsUploadTask : public Task
{
public:
	GpsUploadTask();
	static Task* Create();
protected:
	virtual void doTask();
private:
	void ntfGps(void* p,void* s);
private:
};
#endif // GUARD_GpsUploadTask_h__
