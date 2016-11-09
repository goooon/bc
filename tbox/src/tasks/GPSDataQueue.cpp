#include "./GPSDataQueue.h"
#include "../inc/Config.h"
GPSDataQueue::GPSDataQueue() :pkgs(Config::getInstance().getGPSQueueSize())
{

}

GPSDataQueue& GPSDataQueue::getInstance()
{
	static GPSDataQueue q;
	return q;
}

