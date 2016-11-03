#include "../inc/Task.h"
#include "../inc/TaskList.h"
#include "../inc/Event.h"

Task::Task(u16 appId, bool async) :
	prev(NULLPTR),
	next(NULLPTR),
	appID(appId),
	seqID(0),
	isAsync(async)
{

}

Task::~Task()
{
	LOG_I("Task(%d,%lld) releasing ...", appID, seqID);
	MessageQueue::Args args;
	//clear message restored in the queue
	while (msgQueue.out(args)) {
		if (args.e == AppEvent::PackageArrived) {
			if (args.data != NULL) {
				switch (args.param1) {
				case Package::Mqtt:
					bcp_packet_destroy((bcp_packet_t*)args.data);
					break;
				default:
					LOG_W("Package %d may need some operation", args.param1);
					break;
				}
			}
		}
	}
}

bool Task::handlePackage(bcp_packet_t* pkg)
{
	u32 stepId = -1;
	if (pkg != NULL) {
		bcp_message_t *m = NULL;
		bcp_element_t *e = NULL;
		
		//bcp_messages_foreach(p, bcp_message_foreach_callback, NULL);

		if ((m = bcp_next_message(pkg, m)) != NULL) {
			seqID = m->hdr.sequence_id;
			stepId = m->hdr.step_id;
		}
		else {
			//seqID = sSeqID++;
		}
	}
	msgQueue.post(AppEvent::PackageArrived, Package::Mqtt,stepId , (void*)pkg);
	return true;
}

ThreadEvent::WaitResult Task::waitForEvent(u32 millSeconds)
{
	return msgQueue.wait(millSeconds);
}

void Task::onEvent(AppEvent::Type e, u32 param1, u32 param2, void* data)
{
	msgQueue.post(e, param1, param2, data);
}

void Task::run()
{
	doTask();
	while(!PostEvent(AppEvent::RemoveTask,0,0,this)){}
}

u64 Task::sSeqID = 0;

bool MessageQueue::post(AppEvent::Type e, u32 param1, u32 param2, void* data)
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

bool MessageQueue::out(AppEvent::Type& e, u32& param1, u32& param2, void*& data)
{
	return eventArgs.out(e, param1, param2, data);
}

ThreadEvent::WaitResult MessageQueue::wait(u32 millSecond)
{
	return event.wait(millSecond);
}
