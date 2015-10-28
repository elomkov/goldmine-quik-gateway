
#include "log.h"

#include "ui/mainwindow.h"
#include "core/eventloop.h"
#include <zmq.hpp>
#include "goldmine/data.h"

int main(int argc, char** argv)
{
	log_init("goldmine-quik-gateway.log", false);
	LOG(INFO) << "Goldmine quik gateway started";

	zmq::context_t ctx;
	EventLoop evloop(ctx, "tcp://*:5516", "inproc://tick-pipeline");
	evloop.start();

	MainWindow wnd;
	wnd.show(argc, argv);
	auto rc = Fl::run();
	evloop.stop();
}

