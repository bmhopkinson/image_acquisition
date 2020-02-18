CPPFLAGS=-std=c++11 -g -I/usr/include 
LDFLAGS=-g
LDLIBS=-L/usr/lib -lgps
gpsd_test: gpsd_test.o
	g++ $(LDFLAGS) -o gpsd_test gpsd_test.o $(LDLIBS)

gpsd_test.o: gpsd_test.cpp
	g++ $(CPPFLAGS) -c gpsd_test.cpp

