#include "./RemoteControl.h"

RemoteControlTask::RemoteControlTask(u32 appId):Task(appId,true)
{

}

Task* RemoteControlTask::Create(u32 appId)
{
	return bc_new RemoteControlTask(appId);
}

void RemoteControlTask::doTask()
{

}
