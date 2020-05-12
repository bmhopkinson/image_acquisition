#include "system/Arduino.h"
#include<termios.h>
#include<cstring>
#include<fcntl.h>
#include<unistd.h>
#include<iostream>
#include<chrono>
#include<thread>

Arduino::Arduino()    //autodetect port
{
} 

Arduino::Arduino(std::string port_str) // user defined port
{
   port = port_str;
}   
        
int Arduino::open_uart(int speed){
    uart_speed = speed;

    if((client = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC)) <0){
        perror("UART: failed to open the file\n");
        return -1;
   }
   struct termios options;
   memset(&options, 0, sizeof options);
   if(tcgetattr(client, &options) == -1){
        perror("UART: failed to get attributes\n");
        return -1;
   }
   
   if(uart_speed == 1){
       cfsetospeed (&options, (speed_t)B9600);
       cfsetispeed (&options, (speed_t)B9600);
   }
   if(uart_speed == 2){
       cfsetospeed (&options, (speed_t)B38400);
       cfsetispeed (&options, (speed_t)B38400);
   }
   if(uart_speed == 3){
       cfsetospeed (&options, (speed_t)B115200);
       cfsetispeed (&options, (speed_t)B115200);
   }
   
   /* Setting other Port Stuff */
    options.c_cflag     &=  ~PARENB;           // Make 8n1
    options.c_cflag     &=  ~CSTOPB;
    options.c_cflag     &=  ~CSIZE;
    options.c_cflag     |=  CS8;

    options.c_cflag     &=  ~CRTSCTS;          // no flow control
    options.c_cc[VMIN]   =  1;                  // read doesn't block
    options.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
    options.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

    /* Make raw */
    cfmakeraw(&options);

    /* Flush Port, then applies attributes */
    tcflush(client, TCIFLUSH );
    if ( tcsetattr ( client, TCSANOW, &options) != 0) {
       std::cout << "Error " << errno << " from tcsetattr" << std::endl;
    }
     std::this_thread::sleep_for(std::chrono::seconds(1)); //wait for arduino to reboot - would be better to send a message from arduino and wait for it here, but haven't implemented a read function yet
/* alternate formulation i found 
   //set speed = not sure if there's a better way to do this
   if(uart_speed == 1){
       options.c_cflag = B9600    | CS8 | CREAD | CLOCAL;
   } else if (uart_speed == 2){
       options.c_cflag = B38400  | CS8 | CREAD | CLOCAL;
   } else if (uart_speed == 3){
       options.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
   } else {        
        perror("UART: speed not set\n");
        return -1;
    }

   options.c_iflag = IGNPAR | ICRNL;
   tcflush(client, TCIOFLUSH); 

   if(tcsetattr(client, TCSANOW, &options) == -1){
        perror("UART: failed to set attributes\n");
        return -1;
   }
*/
   
   return 1; 

}

int Arduino::send(std::string str) // send string - implement other types as  needed
{
    char* cstr = new char[str.length()+1];
    std::strcpy(cstr, str.c_str());
    int str_size = strlen(cstr);
    int wsize;
    wsize = write(client, cstr,  str_size);
   // std::cout << "for str: " << str << " write size:" << wsize << std::endl;
    delete[] cstr; 

    return wsize;
}


std::string Arduino::receive_str() //receive string - implement other types as neede 
{
}
