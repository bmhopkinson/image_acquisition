CPPFLAGS=-std=c++11 -g 
LDFLAGS=-g
LDLIBS=
GPIO_trial: GPIO_trial.o  jetsonGPIO_Orbitty.o
	g++ $(LDFLAGS) -o GPIO_trial GPIO_trial.o jetsonGPIO_Orbitty.o $(LDLIBS)

GPIO_trial.o: GPIO_trial.cpp
	g++ $(CPPFLAGS) -c GPIO_trial.cpp
	
jetsonGPIO_Orbitty.o: jetsonGPIO_Orbitty.c
	g++ $(CPPFLAGS) -c jetsonGPIO_Orbitty.c

