CPPFLAGS=-std=c++11 -g -I/usr/local/zed/include -I/usr/local/cuda/include
LDFLAGS=-g
LDLIBS= -lpthread -lgps -L/usr/local/zed/lib -lsl_core -lsl_zed  -lcudpp
Threading_test: Threading_test3.o  Adafruit_BNO055_port.o jetsonGPIO_Orbitty.o
	g++ $(LDFLAGS) -o Threading_test Threading_test3.o Adafruit_BNO055_port.o jetsonGPIO_Orbitty.o  $(LDLIBS)

Threading_test3.o: Threading_test3.cpp
	g++ $(CPPFLAGS) -c Threading_test3.cpp

Adafruit_BNO055_port.o: Adafruit_BNO055_port.cpp
	g++ $(CPPFLAGS) -c Adafruit_BNO055_port.cpp

jetsonGPIO_Orbitty.o: jetsonGPIO_Orbitty.c
	g++ $(CPPFLAGS) -c jetsonGPIO_Orbitty.c

	
