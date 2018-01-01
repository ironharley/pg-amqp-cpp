There's postgresql extention for amqp message brokers incorporation. 
MIT license here.
Postgresql(c) must be upper 9.1, pg_config must be in path.
AMQP-CPP(c) client from github (https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git) here. Look the AMQP-CPP inside root of distribution. If not found   

    `cd pg-amqp-cpp/`
    `git clone https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git`

else 

    `cd pg-amqp-cpp/AMQP-CPP`
    `git pull`
    `cd ..`


Than run `make` and `sudo make install`

Uncomment and put value at your postgresql.conf line:

    `shared_preload_libraries = 'pgamqpcpp.so'`
    
and restart postgresql  server.

If you have previous amqp scheme at database to back up amqp.broker table and drop old amqp extension. Than login as postgres and do next:

    `bash%> plsql`
    `postgres=> \c your_database;`
    `postgres=> create extension pgamqpcpp;`

Restore backed up broker table and work.

Enough. 
Detailed docs at docs folder after `make doc`

