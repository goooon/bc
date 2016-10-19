#include "../inc/Config.h"
#include "../inc/Application.h"
Config& Config::getInstance()
{
	return Application::getInstance().getConfig();
}

