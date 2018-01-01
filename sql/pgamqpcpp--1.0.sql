CREATE TABLE @extschema@.broker (
  broker_id serial NOT NULL,
  host text NOT NULL,
  port integer NOT NULL DEFAULT 5672,
  vhost text,
  username text,
  password text,
  PRIMARY KEY (broker_id, host, port)
);


CREATE OR REPLACE FUNCTION @extschema@.exchange_declare(
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

CREATE OR REPLACE FUNCTION @extschema@.exchange_delete(
    broker_id integer
    , exchange varchar
    , if_unused_only boolean default false
    , nowait boolean default true
)
RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_exchange_delete'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.exchange_delete(integer, varchar, boolean, boolean) IS
'Delete an exchange (broker_id, exchange_name, if_unused, nowait)';

CREATE OR REPLACE FUNCTION @extschema@.queue_declare(
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

CREATE OR REPLACE FUNCTION @extschema@.queue_delete(
    broker_id integer
    , queue varchar
)
RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_queue_delete'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.queue_delete(integer, varchar) IS
'Delete a queue (broker_id, queue_name).';

CREATE OR REPLACE FUNCTION @extschema@.publishcpp(
    broker_id integer,
    exchange character varying,
    routing_key character varying,
    message character varying,
    type character varying DEFAULT ''::character varying,
    delivery_mode integer DEFAULT -1::integer,
    content_type character varying DEFAULT ''::character varying,
    reply_to character varying DEFAULT ''::character varying,
    correlation_id character varying DEFAULT ''::character varying
    )
    RETURNS boolean
    LANGUAGE 'c'

    COST 1
    IMMUTABLE 

RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_publish'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.publishcpp(integer, varchar, varchar, varchar, varchar, integer, varchar, varchar, varchar) IS
'This is true publisher';

-- ---------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION @extschema@.publish(
    broker_id integer
    , exchange varchar
    , routing_key varchar
    , message varchar
    , delivery_mode integer default -1
    , content_type varchar default ''
    , reply_to varchar default ''
    , correlation_id varchar default ''
    , type varchar default ''            
)
RETURNS boolean LANGUAGE 'plpgsql'

    COST 100
    VOLATILE 
AS $BODY$
DECLARE
    v_bool boolean;
BEGIN
	select  @extschema@.publishcpp(broker_id, exchange, routing_key, message, type, delivery_mode, content_type, reply_to, correlation_id) into v_bool;
	return v_bool;
END;
$BODY$;

COMMENT ON FUNCTION @extschema@.publish(integer, varchar, varchar, varchar, integer, varchar, varchar, varchar, varchar) IS
'Publishes a message (broker_id, exchange, routing_key, message [, property...]).
The message will only be published if the containing PostgreSQL transaction successfully commits.
Under certain circumstances, the AMQP commit might fail.  In this case, a WARNING is emitted.
The last five parameters are optional and set the following message properties:
delivery_mode (either 1 or 2), content_type, reply_to, correlation_id and type (this one used 4 java type.class setting).';

CREATE OR REPLACE FUNCTION @extschema@.autonomous_publish(
    broker_id integer
    , exchange varchar
    , routing_key varchar
    , message varchar
    , delivery_mode integer default -1
    , content_type varchar default ''
    , reply_to varchar default ''
    , correlation_id varchar default ''
    , type varchar default ''                            
)
RETURNS boolean LANGUAGE 'plpgsql'

    COST 100
    VOLATILE 
AS $BODY$
DECLARE
    v_bool boolean;
BEGIN
    select  @extschema@.publishcpp(broker_id, exchange, routing_key, message, type, delivery_mode, content_type, reply_to, correlation_id) into v_bool;
    return v_bool;
END; 
$BODY$;

CREATE OR REPLACE FUNCTION @extschema@.queue_bind(
    broker_id integer
    , queue varchar
    , exchange varchar
    , routing_key varchar
)
RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_queue_bind'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.queue_bind(integer, varchar, varchar, varchar) IS
'Binds a queue (broker_id, queue_name, exchange, routing_key) to exchange with routing key.';

CREATE OR REPLACE FUNCTION @extschema@.queue_unbind(
    broker_id integer
    , queue varchar
    , exchange varchar
    , routing_key varchar
)
RETURNS boolean AS 'pgamqpcpp.so', 'pgamqpcpp_queue_unbind'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.queue_unbind(integer, varchar, varchar, varchar) IS
'Removes routing_key-ed bind 4 queue_name to exchange.';


CREATE OR REPLACE FUNCTION @extschema@.disconnect(broker_id integer)
RETURNS void LANGUAGE 'plpgsql'

    COST 100
    VOLATILE 
AS $BODY$
DECLARE
    v_bool boolean;
BEGIN
    RAISE EXCEPTION 'deprecated';     
END; 
$BODY$;

COMMENT ON FUNCTION @extschema@.disconnect(integer) IS
'deprecated in cpp version';

