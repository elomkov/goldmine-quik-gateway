
#include "log.h"

#include "ui/mainwindow.h"
#include "core/eventloop.h"

int main(int argc, char** argv)
{
	log_init("goldmine-quik-gateway.log", true);
	LOG(INFO) << "Goldmine quik gateway started";

	EventLoop evloop;
	evloop.start();

	MainWindow wnd;
	wnd.show(argc, argv);
	return Fl::run();
}

