all:
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/receivedframe.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/flags.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/deferredget.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/tcpconnection.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/field.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/deferredconsumerbase.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/channelimpl.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/deferredcancel.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/array.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/table.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/deferredconsumer.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/connectionimpl.cpp
	g++ -c -O3 -fPIC -std=c++11  -I. -I/src/ src/watchable.cpp
		
	