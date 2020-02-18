CPPFLAGS=-std=c++11 -g 
LDFLAGS=-g
LDLIBS=
adaport_test: adaport_test.o  Adafruit_BNO055_port.o
	g++ $(LDFLAGS) -o adaport_test adaport_test.o Adafruit_BNO055_port.o $(LDLIBS)

adaport_test.o: adaport_test.cpp
	g++ $(CPPFLAGS) -c adaport_test.cpp
	
Adafruit_BNO055_port.o: Adafruit_BNO055_port.cpp
	g++ $(CPPFLAGS) -c Adafruit_BNO055_port.cpp

