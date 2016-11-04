#include "./PackageQueue.h"
#include "../inc/Application.h"
PackageQueue& PackageQueue::getInstance()
{
	return Application::getInstance().getPackageQueue();
}
