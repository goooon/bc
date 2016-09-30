#include "../inc/Task.h"
#include "../inc/TaskList.h"

void Task::run()
{
	doTask();
	refList->out(this);
	bc_del this;
}
