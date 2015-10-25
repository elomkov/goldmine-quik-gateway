
#include "log.h"

#include "ui/mainwindow.h"
#include "core/eventloop.h"
#include <zmq.hpp>

int main(int argc, char** argv)
{
	log_init("goldmine-quik-gateway.log", true);
	LOG(INFO) << "Goldmine quik gateway started";

	zmq::context_t ctx;
	EventLoop evloop(ctx, "tcp://127.0.0.1:5516", "inproc://tick-pipeline");
	evloop.start();

	MainWindow wnd;
	wnd.show(argc, argv);
	auto rc = Fl::run();
	evloop.stop();
}

