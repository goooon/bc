#include "../inc/Message.h"
#include "../inc/dep.h"
#include "./CmdParser.h"
#include "../inc/Application.h"
#include "../inc/RemoteUnlockTask.h"
int main(int argc, char* argv[]) {
	void* lib = initMeLib();

	Application app;
	app.init(argc, argv);
	app.startTask(bc_new RemoteUnlockTask(0,1));
	app.run();

	uninitMeLib(lib);
	return 0;
}