#ifndef GUARD_Operation_h__
#define GUARD_Operation_h__

struct Operation
{
	enum Result
	{
		Succ = 0,					 //ø…“‘∆Ù∂Ø
		S_Blocking,
		W_Aborted,
		E_Code,
		E_Auth,
		E_Permission,
		E_DoorOpened,
		E_Driving,
		E_Ignited,
		E_Brake,
		E_Door,
		E_State
	};
};
#endif // GUARD_Operation_h__
