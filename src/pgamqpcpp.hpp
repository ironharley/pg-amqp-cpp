#ifndef PGAMQPCPP_H
#define PGAMQPCPP_H
#include <ctime>
#include <chrono>
#include <iostream>
#include <string>
#include <amqpcpp.h>
#include "logger.hpp"
#include "handler.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include <postgres.h>
#include <funcapi.h>
#include <fmgr.h>
#include <miscadmin.h>
#include <pgstat.h>
#include <executor/spi.h>
#include <storage/lwlock.h>
#include <storage/shmem.h>
#include <storage/ipc.h>
#include <access/xact.h>
#include <utils/memutils.h>
#include <utils/builtins.h>
#include <stdio.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC
;
#endif

struct brokerinfo {
	int broker_id;
	char *host, *vhost, *user, *pass;
	uint16_t port;

	int idx = 0;
};
PG_FUNCTION_INFO_V1(pgamqpcpp_exchange_declare);
PG_FUNCTION_INFO_V1(pgamqpcpp_exchange_delete);
PG_FUNCTION_INFO_V1(pgamqpcpp_queue_declare);
PG_FUNCTION_INFO_V1(pgamqpcpp_queue_delete);
PG_FUNCTION_INFO_V1(pgamqpcpp_publish);
PG_FUNCTION_INFO_V1(pgamqpcpp_queue_bind);
PG_FUNCTION_INFO_V1(pgamqpcpp_queue_unbind);

#ifdef __cplusplus
}
#endif

class PgAmqpCpp {
public:
	enum class Action
		: unsigned short {
			DeclareExchange = 0x01,
		DeleteExchange = 0x02,
		DeclareQueue = 0x04,
		DeleteQueue = 0x08,
		BindQueue = 0x16,
		UnBindQueue = 0x32,
		PublishMessage = 0x64,
		PublishArrayOfMessages = 0x128,
		ListenQueue = 0x256
	};

	PgAmqpCpp();
	~PgAmqpCpp();

	void connect(brokerinfo *bs);

	int
	proxy(PgAmqpCpp::Action flag, const char* exchange, const char* type, bool passive,
			bool durable, bool auto_delete);
	int
	proxy(PgAmqpCpp::Action flag, const char* queue, bool passive, bool durable,
			bool exclusive, bool auto_delete);

	int
	proxy(PgAmqpCpp::Action flag, const char* exchange, const char* queue, const char* rkey);

	int
	proxy(PgAmqpCpp::Action flag, const char* exchange, const char* rkey, const char* message,
			const int delivery_mode, const char* content_type,
			const char* type);

private:
	std::time_t timestamp();

	AMQP::ExchangeType
	exchangeTypeResolve(std::string type);

	int
	amqpExchangeDelete(const char* exchange, bool ifunused);

	int
	amqpExchangeDeclare(const char* exchange, const char* type, bool passive,
			bool durable, bool auto_delete);

	int
	amqpQueueDelete(const char* queue);

	int
	amqpQueueDeclare(const char* queue, bool passive, bool durable,
			bool exclusive, bool auto_delete);

	int
	amqpPublishMessage(const char* exchange, const char* rkey, const char* message,
			const int delivery_mode, const char* content_type,
			const char* type);

	int
	bindQueue(const char* exchange, const char* queue, const char* rkey);

	int
	unBindQueue(const char* exchange, const char* queue, const char* rkey);

	void close();
	LibEventHandlerMyError *_handler;
	AMQP::TcpConnection *_connection = nullptr;
	AMQP::TcpChannel *_channel = nullptr;
};
#endif
