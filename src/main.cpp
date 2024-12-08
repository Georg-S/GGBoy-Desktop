//#include "EmulatorMain.hpp"
#include "MainWindow.hpp"

// Needed because of SDL
#undef main

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	MainWindow window;
	window.show();
	return app.exec();
	//EmulatorApplication application;

	//return application.run();
	return 0;
}