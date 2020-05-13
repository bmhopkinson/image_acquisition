#ifndef SYS_ARDUINO_H
#define SYS_ARDUINO_H

#include<string>

class Arduino {
    public:
        //constructors 
        Arduino();  //autodetect port
  //      Arduino(std::string port_str); // user defined port
        
        //member functions
        int open_uart(std::string port_str, int speed);
        int send(std::string str); // send string - implement other types as  needed
        std::string receive_str(); //receive string - implement other types as neede 

    private: 
        int client;
        int uart_speed = 1;  // 1= 9600, 2 = 38,400, 3 = 115,200
        std::string port;

};

#endif  // include guard
