#include "../inc/Event.h"
#include "../inc/Application.h"
bool PostEvent(AppEvent e, u32 param1, u32 param2, void* data)
{
	return Application::getInstance().postAppEvent(e,param1,param2,data);
}
