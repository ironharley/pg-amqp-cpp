#include <iostream>
#include <functional>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <event2/event.h>
#include <amqpcpp/libevent.h>

class LibEventHandlerMyError: public AMQP::LibEventHandler {
public:
	LibEventHandlerMyError(struct event_base* evbase) :
			LibEventHandler(evbase), evbase_(evbase) {
	}
	void onError(AMQP::TcpConnection *connection, const char *message) override
	{
		ApplicationLogger(ApplicationLogger::TypeLog::Error)
				<< "On connection: " << message;
		event_base_loopbreak(evbase_);
	}
	void onConnected(AMQP::TcpConnection *connection) {
		connected = true;
		ApplicationLogger(ApplicationLogger::TypeLog::Error)
				<< "\tconnected";
	}

	void onClosed(AMQP::TcpConnection *connection) {
		connected = false;
		ApplicationLogger(ApplicationLogger::TypeLog::Error)
				<< "\tconnection closed";
	}

	void loopbreak() {
		event_base_loopbreak(evbase_);
	}

	void dispatch() {
		event_base_dispatch(evbase_);

	}

	void free() {
		event_base_free(evbase_);
	}

	void exit() {
		event_base_loopexit(evbase_, nullptr);
	}

private:
	struct event_base* evbase_ { nullptr };
	bool connected = false;
};

/*
 class ConnHandler {
 public:
 using EventBasePtrT = std::unique_ptr<struct event_base, std::function<void(struct event_base*)> >;
 using EventPtrT = std::unique_ptr<struct event, std::function<void(struct event*)> >;

 ConnHandler() :
 evbase_(event_base_new(), event_base_free), stdin_event_(
 event_new((event_base*) evbase_.get(), STDIN_FILENO,
 EV_READ, stop, (void*) evbase_.get()), event_free), evhandler_(
 evbase_.get()) {
 event_add((event*) stdin_event_.get(), nullptr);
 }

 void Start() {
 event_base_dispatch((event_base*) evbase_.get());
 }
 void Stop() {
 event_base_loopbreak((event_base*) evbase_.get());
 }

 operator AMQP::TcpHandler*() {
 return &evhandler_;
 }

 private:
 static void stop(evutil_socket_t fd, short what, void *evbase) {
 event_base_loopbreak(reinterpret_cast<event_base*>(evbase));
 }
 EventBasePtrT evbase_;
 EventPtrT stdin_event_;
 LibEventHandlerMyError evhandler_;
 };
 */

