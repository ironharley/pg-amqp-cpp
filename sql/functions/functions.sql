CREATE FUNCTION @extschema@.autonomous_publish(
    broker_id integer
    , exchange varchar
    , routing_key varchar
    , message varchar
    , delivery_mode integer default null
    , content_type varchar default null
    , reply_to varchar default null
    , correlation_id varchar default null
    , type varchar default null            --  Advantum customization
)

RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_autonomous_publish'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.autonomous_publish(integer, varchar, varchar, varchar, integer, varchar, varchar, varchar, varchar) IS
'Works as amqp.publish does, but the message is published immediately irrespective of the
current transaction state.  PostgreSQL commit and rollback at a later point will have no
effect on this message being sent to AMQP.';

CREATE FUNCTION @extschema@.disconnect(broker_id integer)
RETURNS void AS 'pgamqpcpp.so', 'pgamqpcpp_disconnect'
LANGUAGE C IMMUTABLE STRICT;

COMMENT ON FUNCTION @extschema@.disconnect(integer) IS
'Explicitly disconnect the specified (broker_id) if it is current connected. Broker
connections, once established, live until the PostgreSQL backend terminated.  This
allows for more precise control over that.
select amqp.disconnect(broker_id) from amqp.broker
will disconnect any brokers that may be connected.';

CREATE FUNCTION @extschema@.exchange_declare(
    broker_id integer
    , exchange varchar
    , exchange_type varchar
    , passive boolean
    , durable boolean
    , auto_delete boolean DEFAULT false
)
RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_exchange_declare'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.exchange_declare(integer, varchar, varchar, boolean, boolean, boolean) IS
'Declares an exchange (broker_id, exchange_name, exchange_type, passive, durable, auto_delete)
auto_delete should be set to false (default) as unexpected errors can cause disconnect/reconnect which
would trigger the auto deletion of the exchange.';

CREATE FUNCTION @extschema@.publish(
    broker_id integer
    , exchange varchar
    , routing_key varchar
    , message varchar
    , delivery_mode integer default null
    , content_type varchar default null
    , reply_to varchar default null
    , correlation_id varchar default null
    , type varchar default null                            
)
RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_publish'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.publish(integer, varchar, varchar, varchar, integer, varchar, varchar, varchar, varchar) IS
'Publishes a message (broker_id, exchange, routing_key, message [, property...]).
The message will only be published if the containing PostgreSQL transaction successfully commits.
Under certain circumstances, the AMQP commit might fail.  In this case, a WARNING is emitted.
The last five parameters are optional and set the following message properties:
delivery_mode (either 1 or 2), content_type, reply_to, correlation_id and type (this one used 4 java type.class setting).

Publish returns a boolean indicating if the publish command was successful.  Note that as
AMQP publish is asynchronous, you may find out later it was unsuccessful.';

CREATE FUNCTION @extschema@.exchange_delete(
    broker_id integer
    , exchange varchar
    , if_unused_only boolean default false
    , nowait boolean default true
)
RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_exchange_delete'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.exchange_delete(integer, varchar, boolean, boolean) IS
'Delete an exchange (broker_id, exchange_name, if_unused, nowait)';

CREATE FUNCTION @extschema@.queue_declare(
    broker_id integer
    , queue varchar
    , passive boolean DEFAULT false
    , durable boolean DEFAULT false
    , exclusive boolean DEFAULT false
    , auto_delete boolean DEFAULT false
)
RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_queue_declare'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.queue_declare(integer, varchar, boolean, boolean, boolean, boolean) IS
'Declares a queue (broker_id, queue_name, passive, durable, exclusive, auto_delete)
auto_delete should be set to false (default) as unexpected errors can cause disconnect/reconnect which
would trigger the auto deletion of the queue.';

CREATE FUNCTION @extschema@.queue_bind(
    broker_id integer
    , queue varchar
    , exchange varchar
    , routing_key varchar
)
RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_queue_bind'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.queue_bind(integer, varchar, varchar, varchar) IS
'Binds a queue (broker_id, queue_name, exchange, routing_key) to exchange with routing key.';

CREATE FUNCTION @extschema@.queue_unbind(
    broker_id integer
    , queue varchar
    , exchange varchar
    , routing_key varchar
)
RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_queue_unbind'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.queue_unbind(integer, varchar, varchar, varchar) IS
'Removes routing_key-ed bind 4 queue_name to exchange.';
