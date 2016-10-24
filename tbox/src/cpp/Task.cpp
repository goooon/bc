#include "../inc/Task.h"
#include "../inc/TaskList.h"
#include "../inc/Event.h"

Task::Task(u16 appId, bool async) :
	prev(NULLPTR),
	next(NULLPTR),
	appID(appId),
	seqID(1),
	isAsync(async)
{

}

Task::~Task()
{
	MessageQueue::Args args;
	//clear message restored in the queue
	while (msgQueue.out(args)) {
		if (args.e == AppEvent::HandlePackage) {
			if (args.data != NULL) {
				bcp_packet_destroy((bcp_packet_t*)args.data);
			}
		}
	}
	LOG_I("Task(%d,%lld) released", appID, seqID);
}

bool Task::handlePackage(bcp_packet_t* pkg)
{
	msgQueue.post(AppEvent::HandlePackage, 0, 0, (void*)pkg);
	return false;
}

void Task::onEvent(AppEvent::e e, u32 param1, u32 param2, void* data)
{
	msgQueue.post(e, param1, param2, data);
}

void Task::run()
{
	doTask();
	while(!PostEvent(AppEvent::DelTask,0,0,this)){}
}

bool MessageQueue::post(AppEvent::e e, u32 param1, u32 param2, void* data)
{
	bool ret = eventArgs.in(e, param1, param2, data);
	if (!ret) {
		LOG_E("eventArgs.in() failed");
		return false;
	}
	if (ThreadEvent::PostOk != event.post()) {
		LOG_E("event.post() failed");
		return false;
	}
	return true;
}

bool MessageQueue::out(Args& args)
{
	return eventArgs.out(args.e, args.param1, args.param2, args.data);
}

bool MessageQueue::out(AppEvent::e& e, u32& param1, u32& param2, void*& data)
{
	return eventArgs.out(e, param1, param2, data);
}

ThreadEvent::WaitResult MessageQueue::wait(u32 millSecond)
{
	return event.wait(millSecond);
}
