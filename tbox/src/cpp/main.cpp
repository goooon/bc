#include "../inc/Message.h"
#include "../inc/dep.h"
#include "./CmdParser.h"
#include "../inc/Application.h"
#include "../inc/RemoteUnlockTask.h"
int main(int argc, char* argv[]) {
	void* lib = initMeLib();

	Application app;

	app.init(argc, argv);
	app.startTask(bc_new RemoteUnlockTask(1, 2, true),true);
	app.startTask(bc_new RemoteUnlockTask(2, 1, false),true);
	app.startTask(bc_new RemoteUnlockTask(3, 2, true), false);
	app.startTask(bc_new RemoteUnlockTask(4, 1, false), false);
	app.loop();

	uninitMeLib(lib);
	return 0;
}