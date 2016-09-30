#include "../inc/Application.h"

void Application::init(int argc, char** argv)
{
	LOG_I("Application::init(%d)", argc);
	DebugCode( for (int i = 0; i < argc; ++i) {
		LOG_I("    %s", argv[i]);
	})
	config.parse(argc, argv);
}

bool Application::startTask(Task* task)
{
	if (taskQueue.push(task)) {
		ThreadEvent::PostResult pr = e.post();
		if (ThreadEvent::PostOk == pr) {
			LOG_I("startTask(%d,%d)", task->getApplicationId(), task->getSessionId());
			return true;
		}
		else {
			LOG_E("post event failed %d", pr);
			return false;
		}
	}
	else {
		LOG_E("Application::startTask() taskQueue.push() failed ");
		return false;
	}
}

void Application::run()
{
	while (true) {
		LOG_I("Application run loop...");
		ThreadEvent::WaitResult wr = e.wait(5000);
		if (wr == ThreadEvent::EventOk) {
			if (!taskQueue.isEmpty())
			{
				Task* task = taskQueue.pop();
				if (task != nullptr) {
					task->run();
				}
				else {
					LOG_W("task should no be null,something wrong");
				}
			}
		}
		else if(wr == ThreadEvent::TimeOut){

		}
		else {
			LOG_E("wrong wait result %d", wr);
		}
	}
}
