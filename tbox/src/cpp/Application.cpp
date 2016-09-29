#include "../inc/Application.h"

void Application::init(int argc, char** argv)
{
	LOG_I("Application::init(%d)", argc);
	DebugCode( for (int i = 0; i < argc; ++i) {
		LOG_I("    %s", argv[i]);
	})
	config.parse(argc, argv);
}

void Application::run()
{
	while (1) {
		e.wait(-1);
	}
}
