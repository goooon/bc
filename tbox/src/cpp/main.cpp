//#include "../inc/Message.h"
#include "../inc/dep.h"
#include "./CmdParser.h"
#include "../inc/Application.h"
#include "../tasks/RemoteUnlockTask.h"

class MainLoop : public Thread
{
	virtual void run()OVERRIDE
	{
		app.loop();
	}
public:
	MainLoop(Application& app):app(app){}
private:
	Application& app;
};

int main(int argc, char* argv[]) {
	void* lib = initDebugLib();

	LoopCallback lc = debugMain(argc, argv);

	Application app;
	app.init(argc, argv);
	
	if (lc) {
		MainLoop loop(app);
		loop.start();
		lc(getCommand, onCommand);
	}
	else {
		app.loop();
	}

	uninitDebugLib(lib);
	return 0;
}