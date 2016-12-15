#include "../inc/RunTime.h"
#include "../inc/Vehicle.h"

RunTime::RunTime() :stateItemCount(1), debugCollide(1)
{
	diagEcuCount = 0;
	memset(diagDTCCount, 0, sizeof(diagDTCCount));
	memset(collideLevel, 0, sizeof(collideLevel));
}

void RunTime::control(u8 id, u8 arg)
{
	switch (id)
	{
	case 13:
		PostEvent(AppEvent::AutoEvent, arg ? Vehicle::WindOpened : Vehicle::WindClosed, 0, 0);
		break;
	case 14:
		PostEvent(AppEvent::AutoEvent, arg ? Vehicle::WindOpened : Vehicle::WindClosed, 2, 0);
		break;
	case 15:
		PostEvent(AppEvent::AutoEvent, arg ? Vehicle::WindOpened : Vehicle::WindClosed, 1, 0);
		break;
	case 16:
		PostEvent(AppEvent::AutoEvent, arg ? Vehicle::WindOpened : Vehicle::WindClosed, 3, 0);
		break;
	case 11:
		PostEvent(AppEvent::AutoEvent, arg ? Vehicle::DoorOpened : Vehicle::DoorClosed, 4, 0);
		break;
	case 17:
		PostEvent(AppEvent::AutoEvent, arg ? Vehicle::WindOpened : Vehicle::WindClosed, 5, 0);
		break;
	default:
		controlarg[id] = arg;
		break;
	}
}

bool RunTime::getControlResult(u8 id, u8& result)
{
	return true;
}

void RunTime::reqDiagnose(DTCReqDesc* req)
{
	diagEcuCount = req->ecuNumb;
	for (int i = 0; i < diagEcuCount; ++i) {
		diagEcuIndex[i] = req->ecuIndex[i];
	}
}

