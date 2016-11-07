//#include "../inc/Message.h"
#include "../inc/dep.h"
#include "./CmdParser.h"
#include "../inc/Application.h"

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

class TestLoop : public Thread
{
	virtual void run()OVERRIDE
	{
		char cmd[512];
		for (;;) {
			if (scanf("%s", cmd)) {
				onCommand(cmd);
			}
		}
	}
};

int main(int argc, char* argv[]) {
	void* lib = initDebugLib();

	LoopCallback lc =  debugMain(argc, argv);
	printf("==================Tbox===================================\r\n");
	printf(" timestamp " __DATE__ " at " __TIME__ "\r\n");
	printf("=========================================================\r\n");
	Application app;
	if (!app.init(argc, argv)) {
		return 0;
	}
	
	if (lc) {
		MainLoop loop(app);
		loop.start();
		lc(getCommand, onCommand,0,0);
	}
	else {
		TestLoop tl;
		Thread::startThread(&tl);
		app.loop();
	}

	uninitDebugLib(lib);
	return 0;
}