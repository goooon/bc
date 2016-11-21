#include "../inc/CanBus.h"
#include "../inc/Application.h"
CanBus& CanBus::getInstance()
{
	return Application::getInstance().getCanBus();
}

Operation::Result CanBus::reqActiveDoor(bool active)
{
	if (Config::getInstance().getIsAutoCanBus()) {
		PostEvent(AppEvent::AutoEvent, active ? Vehicle::ActiveDoorResult : Vehicle::DeactiveDoorResult, 1, 0);
	}
	LOG_I("Blocking by can bus command(%d) ...",active);
	return Operation::S_Blocking;
}

Operation::Result CanBus::reqEnterReadyToIgnite(bool ready)
{
	if (Config::getInstance().getIsAutoCanBus()) {
		PostEvent(AppEvent::AutoEvent, ready ? Vehicle::EnterReadToIgnit : Vehicle::LeaveReadToIgnit, 1, 0);
	}
	LOG_I("Blocking by can bus command(%d) ...", ready);
	return Operation::S_Blocking;
}
