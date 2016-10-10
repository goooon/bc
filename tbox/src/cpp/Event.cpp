#include "../inc/Event.h"
#include "../inc/Application.h"
bool PostEvent(AppEvent e, u32 param, void* data, int len)
{
	return Application::getInstance().setAppEvent(e, param, data, len);
}
