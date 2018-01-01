EXTENSION     := pgamqpcpp
#PACKAGE       := hkooper.postgresql.extension.$(EXTENSION)
EXTVERSION    := $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")
PG_CONFIG     := pg_config

PG_CONFIG_YES := $(shell $(PG_CONFIG) | wc | awk '{split($$0, tokens," "); if (tokens[1] > 0) print "yes";else print "no"}')
ifeq ($(PG_CONFIG_YES),yes)

PGLIBPATH     := $(shell $(PG_CONFIG) | grep "PKGLIBDIR" | \
    awk '{split($$0, tokens,"="); \
    gsub(/^[ \t]+/, "", tokens[2]); \
    gsub(/[ \t]+$$/, "", tokens[2]); print tokens[2]}' )
    
PGEXTPATH     := $(shell $(PG_CONFIG) | grep "SHAREDIR"  | \
    awk '{split($$0, tokens,"="); \
    gsub(/^[ \t]+/, "", tokens[2]); \
    gsub(/[ \t]+$$/, "", tokens[2]); print tokens[2]}' )
PGEXTPATH     := $(PGEXTPATH)/extension
#-----------------------------------------------------------------------------------------
SONAME        := $(EXTENSION).so
SOURCESCXX    := src/$(EXTENSION).cpp 
OBJECTSCXX    := $(EXTENSION).o \
 amqp-cpp/src/receivedframe.o \
 amqp-cpp/src/flags.o \
 amqp-cpp/src/deferredget.o \
 amqp-cpp/src/tcpconnection.o \
 amqp-cpp/src/field.o \
 amqp-cpp/src/deferredconsumerbase.o \
 amqp-cpp/src/channelimpl.o \
 amqp-cpp/src/deferredcancel.o \
 amqp-cpp/src/array.o \
 amqp-cpp/src/table.o \
 amqp-cpp/src/deferredconsumer.o \
 amqp-cpp/src/connectionimpl.o \
 amqp-cpp/src/watchable.o
CXXFLAGS       := -c -O3 -fPIC -std=c++11 
INCLUDES       := -I. -I./amqp-cpp -I./include -I/usr/include/postgresql/server -I/usr/include/postgresql/internal -I/usr/include/libxml2 -D_PACKAGE=\"$(PACKAGE)\"
LLDFLAGS       := -shared -fPIC -Wl,-soname,$(SONAME)
LIBS           := -L/usr/lib -L.
LIBRARIES      :=  -lstdc++ -levent -pthread 
CXX            := g++

PG91  = $(shell $(PG_CONFIG) --version | grep -qE " 8\.| 9\.0" && echo no || echo yes)
ifeq ($(PG91),yes)

all: amqpcpp $(SONAME) sql/$(EXTENSION)--$(EXTVERSION).sql 

$(SONAME): clean_this
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SOURCESCXX) 
	$(CXX) $(LLDFLAGS) $(SOFLAGS) $(OBJECTSCXX) $(LIBS) $(LIBRARIES) -o $(SONAME).$(EXTVERSION)  

sql/$(EXTENSION)--$(EXTVERSION).sql: sql/tables/*.sql sql/functions/*.sql
	cat $^ > $@
		
amqpcpp:
	ln -s -f ./AMQP-CPP ./amqp-cpp
	cp ./amqp-cpp.mk ./amqp-cpp
	make --file=amqp-cpp.mk --directory=./amqp-cpp all
	mv ./amqp-cpp/*.o ./amqp-cpp/src/
	rm -fr ./amqp-cpp/amqp-cpp.mk
	rm -fr ./include
	mkdir -p ./include
	cp -R ./amqp-cpp/include ./include/amqpcpp
	cp ./amqp-cpp/amqpcpp.h ./src
		
clean: clean_this
	make --directory=./amqp-cpp  clean
	ln -s -f ./AMQP-CPP ./amqp-cpp

clean_this:	
	rm -fr src/*.o
	rm -fr *.o
	rm -fr *.so*

doc:
	makeinfo --html docs/docs.texi --output=./docs/pgampqcpp
	
install:
	cp ./sql/$(EXTENSION)--$(EXTVERSION).sql $(PGEXTPATH)
	cp ./$(EXTENSION).control $(PGEXTPATH)	
	cp ./$(SONAME).$(EXTVERSION) $(PGLIBPATH)
	ln -s -f  $(PGLIBPATH)/$(SONAME).$(EXTVERSION) $(PGLIBPATH)/$(SONAME)
	
uninstall:
	rm -fr $(PGEXTPATH)/$(EXTENSION)--$(EXTVERSION).sql
	rm -fr $(PGEXTPATH)/$(EXTENSION).control 	
	rm -fr $(PGLIBPATH)/$(SONAME)*

else
$(error Minimum version of PostgreSQL required is 9.1.0)
endif
else
$(error pg_config have to be in the path)
endif