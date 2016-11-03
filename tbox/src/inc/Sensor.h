#ifndef GUARD_Sensor_h__
#define GUARD_Sensor_h__

#include "./dep.h"
#include "./Operation.h"
#include "./Apparatus.h"

class Sensor
{
	friend class Application;
public:
	enum Event {
		Pedal,
		Other
	};
};
#endif // GUARD_Sensor_h__
