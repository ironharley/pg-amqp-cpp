DROP TABLE IF EXISTS @extschema@.broker;
CREATE TABLE @extschema@.broker (
  broker_id bigint NOT NULL,
  host text NOT NULL,
  port integer NOT NULL DEFAULT 5672,
  vhost text,
  username text,
  password text,
  PRIMARY KEY (broker_id, host, port)
);
