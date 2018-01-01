#include "pgamqpcpp.hpp"

PgAmqpCpp::PgAmqpCpp() {
	_handler = new LibEventHandlerMyError(event_base_new());
}

PgAmqpCpp::~PgAmqpCpp() {
	close();
}

AMQP::ExchangeType PgAmqpCpp::exchangeTypeResolve(const std::string type) {
	if (std::string("topic").compare(type) == 0) {
		return AMQP::ExchangeType::topic;

	} else if (std::string("fanout").compare(type) == 0) {
		return AMQP::ExchangeType::fanout;

	} else if (std::string("direct").compare(type) == 0) {
		return AMQP::ExchangeType::direct;

	} else if (std::string("headers").compare(type) == 0) {
		return AMQP::ExchangeType::headers;

	} else if (std::string("x-consistent-hash").compare(type) == 0) {
		return AMQP::ExchangeType::consistent_hash;

	}
	return AMQP::ExchangeType::topic;
}

std::time_t PgAmqpCpp::timestamp() {
	auto now = std::chrono::system_clock::now();
	return std::chrono::system_clock::to_time_t(now);
}

void PgAmqpCpp::connect(brokerinfo *bs) {
	const std::string HOST(bs->host);
	const std::string VHOST(bs->vhost);
	const std::string USER(bs->user);
	const std::string PASS(bs->pass);

	if (_connection == nullptr) {
		_handler = new LibEventHandlerMyError(event_base_new());
		ApplicationLogger(ApplicationLogger::TypeLog::Info)
				<< "--------------------------------------------";
		ApplicationLogger(ApplicationLogger::TypeLog::Info)
				<< "Request connection to " << HOST << std::to_string(bs->port)
				<< "/" << VHOST << " as " << USER << "/" << PASS;
		const AMQP::Login login(USER, PASS);
		const AMQP::Address address(HOST, bs->port, login, VHOST);
		_connection = new AMQP::TcpConnection(_handler, address);

		if (_connection != nullptr) {
			try {
				_channel = new AMQP::TcpChannel(_connection);
				_channel->onError(
						[ this](const char* message)
						{
							ApplicationLogger(ApplicationLogger::TypeLog::Error) << message;
							this->_handler->loopbreak();
						});

			} catch (std::exception& e) {
				ApplicationLogger(ApplicationLogger::TypeLog::Error)
						<< e.what();
			}
		} else {
			ApplicationLogger(ApplicationLogger::TypeLog::Error)
					<< "PgAmqpCpp::connect CONNECTION is null";

		}

	} else {

		ApplicationLogger(ApplicationLogger::TypeLog::Error)
				<< "Request connection with exists connect. Don't correct.";
	}
}

void PgAmqpCpp::close() {
	if (_channel != nullptr) {
		_channel->close();
		_channel = nullptr;
	}
	if (_connection != nullptr) {
		_connection->close();
		_connection = nullptr;
	}
}

int PgAmqpCpp::proxy(PgAmqpCpp::Action flag, const char* exchange,
		const char* type, bool passive, bool durable, bool auto_delete) {
	switch (flag) {
	case PgAmqpCpp::Action::DeclareExchange: {
		return this->amqpExchangeDeclare(exchange, type, passive, durable,
				auto_delete);
	}
		break;
	case PgAmqpCpp::Action::DeleteExchange: {
		return this->amqpExchangeDelete(exchange, passive);
	}
		break;
	default:
		break;
	}
	return 0;
}

int PgAmqpCpp::proxy(PgAmqpCpp::Action flag, const char* queue, bool passive,
		bool durable, bool exclusive, bool auto_delete) {
	switch (flag) {
	case PgAmqpCpp::Action::DeclareQueue: {
		return this->amqpQueueDeclare(queue, passive, durable, exclusive,
				auto_delete);
	}
		break;
	case PgAmqpCpp::Action::DeleteQueue: {
		return this->amqpQueueDelete(queue);
	}
		break;
	default:
		break;
	}
	return 0;

}

int PgAmqpCpp::proxy(PgAmqpCpp::Action flag, const char* exchange,
		const char* queue, const char* rkey) {
	switch (flag) {
	case PgAmqpCpp::Action::BindQueue: {
		return this->bindQueue(exchange, queue, rkey);
	}
		break;
	case PgAmqpCpp::Action::UnBindQueue: {
		return this->unBindQueue(exchange, queue, rkey);
	}
		break;
	}
	return 0;
}

int PgAmqpCpp::proxy(PgAmqpCpp::Action flag, const char* exchange,
		const char* rkey, const char* message, const int delivery_mode,
		const char* content_type, const char* type) {
	switch (flag) {
	case PgAmqpCpp::Action::PublishMessage: {
		return this->amqpPublishMessage(exchange, rkey, message, delivery_mode,
				content_type, type);
	}
		break;
	case PgAmqpCpp::Action::PublishArrayOfMessages: {
		//TODO here
	}
		break;
	default:
		break;
	}
	return 0;
}

int PgAmqpCpp::amqpExchangeDelete(const char* exchange, bool ifunused) {
	int ret = 0;

	const std::string EXCHANGE(exchange);

	ApplicationLogger(ApplicationLogger::TypeLog::Info)
			<< "PgAmqpCpp::amqpExchangeDelete " << EXCHANGE;

	_channel->removeExchange(EXCHANGE, ifunused).onSuccess(
			[ &EXCHANGE]()
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Info) << "Exchange " << EXCHANGE << " removed. OK";
			}).onError(
			[ &ret, &EXCHANGE](const char* message)
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Error) << "Exchange " << EXCHANGE << " deletion error: "<< message;
				ret = 1;
			}).onFinalize([ this]()
	{
		this->_handler->exit();
	});
	_handler->dispatch();
	_handler->free();
	this->close();

	return ret;
}

int PgAmqpCpp::amqpExchangeDeclare(const char* exchange, const char* type,
		bool passive, bool durable, bool auto_delete) {

	int ret = 0;

	const std::string EXCHANGE(exchange);

	ApplicationLogger(ApplicationLogger::TypeLog::Info)
			<< "PgAmqpCpp::amqpExchangeDeclare " << EXCHANGE;

	_channel->declareExchange(EXCHANGE, exchangeTypeResolve(std::string(type)),
			(durable & AMQP::durable) | (passive & AMQP::passive)
					| (auto_delete & AMQP::autodelete)).onSuccess(
			[ &EXCHANGE]()
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Info) << "Exchange " << EXCHANGE << " declared. OK";
			}).onError(
			[ &ret, &EXCHANGE](const char* message)
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Error) << "Exchange |" << EXCHANGE << "| declaration error: "<< message;
				ret = 1;
			}).onFinalize([ this]()
	{
		this->_handler->exit();
	});
	_handler->dispatch();
	_handler->free();
	this->close();

	return ret;
}

int PgAmqpCpp::amqpQueueDelete(const char* queue) {
	int ret = 0;

	const std::string QUEUE(queue);

	ApplicationLogger(ApplicationLogger::TypeLog::Info)
			<< "PgAmqpCpp::amqpQueueDelete " << QUEUE;

	_channel->removeQueue(QUEUE).onSuccess(
			[ &QUEUE]()
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Info) << "Queue " << QUEUE << " removed. OK";
			}).onError(
			[ &ret, &QUEUE](const char* message)
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Error) << "Queue " << QUEUE << " deletion error: "<< message;
				ret = 1;
			}).onFinalize([ this]()
	{
		this->_handler->exit();
	});
	_handler->dispatch();
	_handler->free();
	this->close();

	return ret;
}

int PgAmqpCpp::amqpQueueDeclare(const char* queue, bool passive, bool durable,
		bool exclusive, bool auto_delete) {
	int ret = 0;

	const std::string QUEUE(queue);

	ApplicationLogger(ApplicationLogger::TypeLog::Info)
			<< "PgAmqpCpp::amqpQueueDeclare " << QUEUE;

	_channel->declareQueue(QUEUE,
			(durable & AMQP::durable) | (passive & AMQP::passive)
					| (exclusive & AMQP::exclusive)
					| (auto_delete & AMQP::autodelete)).onSuccess(
			[ &QUEUE]()
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Info) << "Queue " << QUEUE << " declared. OK";
			}).onError(
			[ &ret, &QUEUE](const char* message)
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Error) << "Queue |" << QUEUE << "| declaration error: "<< message;
				ret = 1;
			}).onFinalize([ this]()
	{
		this->_handler->exit();
	});
	_handler->dispatch();
	_handler->free();
	this->close();

	return ret;
}

int PgAmqpCpp::amqpPublishMessage(const char* exchange, const char* rkey,
		const char* message, const int delivery_mode, const char* content_type,
		const char* type) {
	int ret = 0;

	const std::string EXCHANGE(exchange);
	const std::string RKEY(rkey);
	const std::string TYPE(type);
	const std::string C_TYPE(content_type);

	ApplicationLogger(ApplicationLogger::TypeLog::Info)
			<< "PgAmqpCpp::amqpPublishMessage to " << EXCHANGE << "/" << RKEY;

	AMQP::Envelope env(message, strlen(message));
	if (TYPE.length() > 0)
		env.setTypeName(TYPE);
	if (C_TYPE.length() > 0)
		env.setContentType(C_TYPE);
	if (delivery_mode != -1)
		env.setDeliveryMode(delivery_mode);

	_channel->startTransaction();
	_channel->publish(EXCHANGE, RKEY, env);
	_channel->commitTransaction().onSuccess([this]() {
		ApplicationLogger(ApplicationLogger::TypeLog::Info)
		<< "OK";
		this->_handler->exit();
	}).onError([this, &ret](const char* message) {
		ApplicationLogger(ApplicationLogger::TypeLog::Error)
		<< message;
		this->_handler->loopbreak();
		ret = 1;
	});
	;
	_handler->dispatch();
	_handler->free();

	this->close();

	return ret;
}

int PgAmqpCpp::bindQueue(const char* exchange, const char* queue,
		const char* rkey) {
	int ret = 0;

	const std::string EXCHANGE(exchange);
	const std::string QUEUE(queue);
	const std::string RKEY(rkey);

	ApplicationLogger(ApplicationLogger::TypeLog::Info)
			<< "PgAmqpCpp::bindQueue exchange " << EXCHANGE << " with " << QUEUE
			<< " key " << RKEY;

	_channel->bindQueue(EXCHANGE, QUEUE, RKEY).onSuccess(
			[ &EXCHANGE, &QUEUE]()
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Info) << "Exc " <<EXCHANGE << " bind with " << QUEUE << ". OK";
			}).onError(
			[ &ret, &QUEUE](const char* message)
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Error) << "Binding failure: "<< message;
				ret = 1;
			}).onFinalize([ this]()
	{
		this->_handler->exit();
	});
	_handler->dispatch();
	_handler->free();
	this->close();

	return ret;
}

int PgAmqpCpp::unBindQueue(const char* exchange, const char* queue,
		const char* rkey) {
	int ret = 0;

	const std::string EXCHANGE(exchange);
	const std::string QUEUE(queue);
	const std::string RKEY(rkey);

	ApplicationLogger(ApplicationLogger::TypeLog::Info)
			<< "PgAmqpCpp::unBindQueue exchange " << EXCHANGE << " with "
			<< QUEUE << " key " << RKEY;

	_channel->unbindQueue(EXCHANGE, QUEUE, RKEY).onSuccess(
			[ &EXCHANGE, &QUEUE]()
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Info) << "Exc " <<EXCHANGE << " bind with " << QUEUE << ". OK";
			}).onError(
			[ &ret, &QUEUE](const char* message)
			{
				ApplicationLogger(ApplicationLogger::TypeLog::Error) << "Binding failure: "<< message;
				ret = 1;
			}).onFinalize([ this]()
	{
		this->_handler->exit();
	});
	_handler->dispatch();
	_handler->free();
	this->close();

	return ret;
}

static struct brokerinfo* create_bs(int broker_id) {
	struct brokerinfo* bs;
	bs = (brokerinfo*) MemoryContextAllocZero(TopMemoryContext, sizeof(*bs));
	bs->broker_id = broker_id;
	return bs;
}

static struct brokerinfo* get_broker_details(int broker_id) {

	struct brokerinfo *bs = create_bs(broker_id);
	bs->broker_id = broker_id;

	if (SPI_connect() == SPI_ERROR_CONNECT)
		return NULL;

	char sql[1024];
	snprintf(sql, sizeof(sql), "SELECT host, port, vhost, username, password "
			"  FROM amqp.broker "
			" WHERE broker_id = %d "
			" ORDER BY host DESC, port", bs->broker_id);

	if (SPI_OK_SELECT == SPI_execute(sql, true, 100)) {

		if (SPI_processed > 0) {

			Datum port_datum;
			bool is_null;

			bs->idx = (bs->idx + 1) % SPI_processed;
			bs->host = SPI_getvalue(SPI_tuptable->vals[bs->idx],
					SPI_tuptable->tupdesc, 1);
			if (!bs->host)
				bs->host = (char*) "localhost";
			port_datum = SPI_getbinval(SPI_tuptable->vals[bs->idx],
					SPI_tuptable->tupdesc, 2, &is_null);
			if (!is_null)
				bs->port = DatumGetInt32(port_datum);
			bs->vhost = SPI_getvalue(SPI_tuptable->vals[bs->idx],
					SPI_tuptable->tupdesc, 3);
			if (!bs->vhost)
				bs->vhost = (char*) "/";
			bs->user = SPI_getvalue(SPI_tuptable->vals[bs->idx],
					SPI_tuptable->tupdesc, 4);
			if (!bs->user)
				bs->user = (char*) "guest";
			bs->pass = SPI_getvalue(SPI_tuptable->vals[bs->idx],
					SPI_tuptable->tupdesc, 5);
			if (!bs->pass)
				bs->pass = (char*) "guest";

		} else {
			fprintf(stderr, "WARNING: amqp can't find broker %d", broker_id);
		}
	} else {
		fprintf(stderr, "WARNING: amqp broker lookup query failed");
	}
	SPI_finish();
	return bs;
}

/**
 * We're in PG context and use PG memory functions
 */
static char* getText2String(char* result, text* src, char* name) {
	size_t length = VARSIZE(src) - VARHDRSZ;
	result = (char*) MemoryContextAllocZero(TopMemoryContext, length);
	result = pnstrdup(VARDATA(src), length);
	/*
	 fprintf(stderr, "WARNING: length of string %s is %d values %s\n", name,
	 length, result);
	 */
	return result;
}

Datum pgamqpcpp_exchange_declare( PG_FUNCTION_ARGS) {
	int result = 1;
	try {
		if (!PG_ARGISNULL(0)) {
			int broker_id = PG_GETARG_INT32(0);
			struct brokerinfo *bs = get_broker_details(broker_id);
			if (bs) {
				char* exchange_name = getText2String(exchange_name,
						PG_GETARG_TEXT_P_COPY(1), (char*) "exchange_name");
				char* exchange_type = getText2String(exchange_type,
						PG_GETARG_TEXT_P_COPY(2), (char*) "exchange_type");
				bool passive = PG_GETARG_BOOL(3);
				bool durable = PG_GETARG_BOOL(4);
				bool auto_delete = PG_GETARG_BOOL(5);

				PgAmqpCpp *PgCpp_instance = new PgAmqpCpp();
				PgCpp_instance->connect(bs);
				result = PgCpp_instance->proxy(
						PgAmqpCpp::Action::DeclareExchange, exchange_name,
						exchange_type, passive, durable, auto_delete);
			}
		}
	} catch (std::exception &ex) {
		ApplicationLogger(ApplicationLogger::TypeLog::Error) << ex.what();
	}

	PG_RETURN_BOOL(result == 0);
}

Datum pgamqpcpp_exchange_delete( PG_FUNCTION_ARGS) {
	int result = 1;
	try {
		if (!PG_ARGISNULL(0)) {
			int broker_id = PG_GETARG_INT32(0);
			struct brokerinfo *bs = get_broker_details(broker_id);
			if (bs) {

				char* exchange = getText2String(exchange,
						PG_GETARG_TEXT_P_COPY(1), (char*) "exchange_name");
				bool ifunused = PG_GETARG_BOOL(2);

				PgAmqpCpp *PgCpp_instance = new PgAmqpCpp();
				PgCpp_instance->connect(bs);
				result = PgCpp_instance->proxy(
						PgAmqpCpp::Action::DeleteExchange, exchange, "",
						ifunused, false, false);

			}
		}
	} catch (std::exception &ex) {
		ApplicationLogger(ApplicationLogger::TypeLog::Error) << ex.what();
	}

	PG_RETURN_BOOL(result == 0);
}

Datum pgamqpcpp_queue_declare( PG_FUNCTION_ARGS) {
	int result = 1;
	try {
		if (!PG_ARGISNULL(0)) {
			int broker_id = PG_GETARG_INT32(0);
			struct brokerinfo *bs = get_broker_details(broker_id);
			if (bs) {

				char* queue_name = getText2String(queue_name,
						PG_GETARG_TEXT_P_COPY(1), (char*) "queue_name");
				bool passive = PG_GETARG_BOOL(2);
				bool durable = PG_GETARG_BOOL(3);
				bool exclusive = PG_GETARG_BOOL(4);
				bool auto_delete = PG_GETARG_BOOL(5);

				PgAmqpCpp *PgCpp_instance = new PgAmqpCpp();
				PgCpp_instance->connect(bs);
				result = PgCpp_instance->proxy(PgAmqpCpp::Action::DeclareQueue,
						queue_name, passive, durable, exclusive, auto_delete);
			}

		}
	} catch (std::exception &ex) {
		ApplicationLogger(ApplicationLogger::TypeLog::Error) << ex.what();
	}

	PG_RETURN_BOOL(result == 0);
}

Datum pgamqpcpp_queue_delete( PG_FUNCTION_ARGS) {
	int result = 1;
	try {
		if (!PG_ARGISNULL(0)) {
			int broker_id = PG_GETARG_INT32(0);
			struct brokerinfo *bs = get_broker_details(broker_id);
			if (bs) {

				char* queue_name = getText2String(queue_name,
						PG_GETARG_TEXT_P_COPY(1), (char*) "queue_name");

				PgAmqpCpp *PgCpp_instance = new PgAmqpCpp();
				PgCpp_instance->connect(bs);
				result = PgCpp_instance->proxy(PgAmqpCpp::Action::DeleteQueue,
						queue_name, false, false, false, false);
			}
		}
	} catch (std::exception &ex) {
		ApplicationLogger(ApplicationLogger::TypeLog::Error) << ex.what();
	}

	PG_RETURN_BOOL(result == 0);
}

Datum pgamqpcpp_publish( PG_FUNCTION_ARGS) {
	int result = 1;
	try {
		if (!PG_ARGISNULL(0)) {
			int broker_id = PG_GETARG_INT32(0);
			struct brokerinfo *bs = get_broker_details(broker_id);
			if (bs) {

				char* exchange_name = getText2String(exchange_name,
						PG_GETARG_TEXT_P_COPY(1), (char*) "exchange_name");
				char* routing_key = getText2String(routing_key,
						PG_GETARG_TEXT_P_COPY(2), (char*) "routing_key");
				char* message = getText2String(message,
						PG_GETARG_TEXT_P_COPY(3), (char*) "message");
				char* type = getText2String(type, PG_GETARG_TEXT_P_COPY(4),
						(char*) "type");
				int delivery_mode = PG_GETARG_INT32(5);
				char* content_type = getText2String(content_type,
						PG_GETARG_TEXT_PP(6), (char*) "content_type");
				char* reply_to = getText2String(reply_to, PG_GETARG_TEXT_PP(7),
						(char*) "reply_to");
				char* correlation_id = getText2String(correlation_id,
						PG_GETARG_TEXT_PP(8), (char*) "correlation_id");

				PgAmqpCpp *PgCpp_instance = new PgAmqpCpp();
				PgCpp_instance->connect(bs);
				result = PgCpp_instance->proxy(
						PgAmqpCpp::Action::PublishMessage, exchange_name,
						routing_key, message, delivery_mode, content_type,
						type);

			}
		}
	} catch (std::exception &ex) {
		ApplicationLogger(ApplicationLogger::TypeLog::Error) << ex.what();
	}

	PG_RETURN_BOOL(result == 0);

}
Datum pgamqpcpp_queue_bind( PG_FUNCTION_ARGS) {
	int result = 1;
	try {
		if (!PG_ARGISNULL(0)) {
			int broker_id = PG_GETARG_INT32(0);
			struct brokerinfo *bs = get_broker_details(broker_id);
			if (bs) {
				char* queue_name = getText2String(queue_name,
						PG_GETARG_TEXT_P_COPY(1), (char*) "queue_name");
				char* exchange_name = getText2String(exchange_name,
						PG_GETARG_TEXT_P_COPY(2), (char*) "exchange_name");
				char* routing_key = getText2String(routing_key,
						PG_GETARG_TEXT_P_COPY(3), (char*) "routing_key");

				PgAmqpCpp *PgCpp_instance = new PgAmqpCpp();
				PgCpp_instance->connect(bs);
				result = PgCpp_instance->proxy(PgAmqpCpp::Action::BindQueue,
						exchange_name, queue_name, routing_key);

			}
		}
	} catch (std::exception &ex) {
		ApplicationLogger(ApplicationLogger::TypeLog::Error) << ex.what();
	}

	PG_RETURN_BOOL(result == 0);
}

Datum pgamqpcpp_queue_unbind( PG_FUNCTION_ARGS) {
	int result = 1;
	try {
		if (!PG_ARGISNULL(0)) {
			int broker_id = PG_GETARG_INT32(0);
			struct brokerinfo *bs = get_broker_details(broker_id);
			if (bs) {
				char* queue_name = getText2String(queue_name,
						PG_GETARG_TEXT_P_COPY(1), (char*) "queue_name");
				char* exchange_name = getText2String(exchange_name,
						PG_GETARG_TEXT_P_COPY(2), (char*) "exchange_name");
				char* routing_key = getText2String(routing_key,
						PG_GETARG_TEXT_P_COPY(3), (char*) "routing_key");

				PgAmqpCpp *PgCpp_instance = new PgAmqpCpp();
				PgCpp_instance->connect(bs);
				result = PgCpp_instance->proxy(PgAmqpCpp::Action::UnBindQueue,
						exchange_name, queue_name, routing_key);

			}
		}
	} catch (std::exception &ex) {
		ApplicationLogger(ApplicationLogger::TypeLog::Error) << ex.what();
	}

	PG_RETURN_BOOL(result == 0);
}

