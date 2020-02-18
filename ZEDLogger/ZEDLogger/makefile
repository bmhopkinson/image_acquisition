CPPFLAGS=-std=c++11 -g -I/usr/local/zed/include -I/usr/local/cuda/include
LDFLAGS=-g
LDLIBS= -lpthread -lgps -L/usr/local/zed/lib -lsl_core -lsl_zed  -lcudpp
ZEDLogger: ZEDLogger_1.o  Adafruit_BNO055_port.o jetsonGPIO_Orbitty.o
	g++ $(LDFLAGS) -o ZEDLogger ZEDLogger_1.o Adafruit_BNO055_port.o jetsonGPIO_Orbitty.o  $(LDLIBS)

ZEDLogger_1.o: ZEDLogger_1.cpp
	g++ $(CPPFLAGS) -c ZEDLogger_1.cpp

Adafruit_BNO055_port.o: Adafruit_BNO055_port.cpp
	g++ $(CPPFLAGS) -c Adafruit_BNO055_port.cpp

jetsonGPIO_Orbitty.o: jetsonGPIO_Orbitty.c
	g++ $(CPPFLAGS) -c jetsonGPIO_Orbitty.c

	
