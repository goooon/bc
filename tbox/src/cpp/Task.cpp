#include "../inc/Task.h"
#include "../inc/TaskList.h"
#include "../inc/Event.h"

void Task::onEvent(AppEvent::e e, u32 param1, u32 param2, void* data)
{

}

void Task::run()
{
	doTask();
	while(!PostEvent(AppEvent::DelTask,0,0,this)){}
}
