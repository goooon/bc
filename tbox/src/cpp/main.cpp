#include "../inc/Message.h"
#include "../inc/dep.h"
#include "./CmdParser.h"
#include "../inc/Application.h"

int main(int argc, char* argv[]) {
	void* lib = initMeLib();

	Application app;
	app.init(argc, argv);

	uninitMeLib(lib);
	return 0;
}