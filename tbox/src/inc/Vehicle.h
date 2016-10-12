#ifndef GUARD_Vehicle_h__
#define GUARD_Vehicle_h__
#include "./dep.h"
class Vehicle
{
	friend class Application;
public:
	enum State
	{
		Unauthed,
		Authing,
		Authed
	};
public:
	Vehicle() :authed(Unauthed) {}
private:
	State authed;
};

#endif // GUARD_Vehicle_h__
