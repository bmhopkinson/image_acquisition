
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <thread>
#include "jetsonGPIO_Orbitty.h"
using namespace std;

int main(int argc, char *argv[]){

  cout << "Testing the GPIO Pins" << endl;
  jetsonGPIONumber sysLED = gpio0; //indicates system (this program) is running
  jetsonGPIONumber recLED = gpio1; //indicates data is being recorded.
  jetsonGPIONumber inSwch = gpio2; //input switch

  gpioExport(sysLED);
  gpioExport(recLED);
  gpioExport(inSwch);

  gpioSetDirection(sysLED, outputPin);
  gpioSetDirection(recLED, outputPin);
  gpioSetDirection(inSwch,  inputPin);
 

  // Flash the LEDs 5 times
  for(int i=0; i<5; i++){
      cout << "Setting the LED on" << endl;        
      gpioSetValue(sysLED, off);
      gpioSetValue(recLED, on);
      usleep(500*1000);         // on for 500ms
      cout << "Setting the LED off" << endl;
      gpioSetValue(sysLED, on);
      gpioSetValue(recLED, off);
      usleep(500*1000);         // off for 500ms
   }

  for(int j = 0; j <25; j++){   //read the input 5 times
     unsigned int inVal[1];
     gpioGetValue( inSwch, inVal ) ;
     cout << "Button value is: " << *inVal << endl;
     usleep(1000*1000);
  }


   gpioUnexport(sysLED);     // unexport the system LED
   gpioUnexport(recLED);     // unexport the record LED
   gpioUnexport(inSwch);     // unexport the record LED
  
  return 0;

}
