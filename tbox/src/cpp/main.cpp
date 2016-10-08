#include "../inc/Message.h"
#include "../inc/dep.h"
#include "./CmdParser.h"
#include "../inc/Application.h"
#include "../inc/RemoteUnlockTask.h"

static Application* g_papp;
void onCommand(char* cmd) {
	g_papp->onCommand(cmd);
}

int main(int argc, char* argv[]) {
	void* lib = initDebugLib();

	LoopCallback lc = debugMain(argc, argv);

	Application app;
	app.init(argc, argv);
	app.startTask(bc_new RemoteUnlockTask(1, 2, true), true);
	app.startTask(bc_new RemoteUnlockTask(2, 1, false), true);
	app.startTask(bc_new RemoteUnlockTask(3, 2, true), false);
	app.startTask(bc_new RemoteUnlockTask(4, 1, false), false);

	if (lc) {
		g_papp = &app;
		lc(onCommand);
	}
	else {
		app.loop();
	}

	uninitDebugLib(lib);
	return 0;
}