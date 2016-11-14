#include "../inc/Config.h"
#include "../inc/Application.h"
#include "../inc/Vehicle.h"
Config& Config::getInstance()
{
	return Application::getInstance().getConfig();
}

u32 Config::getGpsInterval()
{
	return Vehicle::getInstance().isDriving() ? gpsIntervalDriving : Vehicle::getInstance().isMovingInAbnormal() ? gpsIntervalAbormal : gpsIntervalStation;
}

