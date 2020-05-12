#ifndef SYS_SEVEN_SEG_H
#define SYS_SEVEN_SEG_H
#include <map>
#include <thread>
#include <mutex>
#include "Arduino.h"

struct Seven_seg_data{
    int duration; //milliseconds
    std::string msg;
};

class Seven_seg {
    public: 
        Seven_seg(Arduino* conn);  //constructor -> pointer to device (currently only arduino implemented);
        ~Seven_seg(); //destructor joins periodicThread if it's still running
        //should probably have a destructor to stop periodic display and join thread if still running;
       // void display_now(std::string str); //not yet implemented
        void add_periodic(std::string item, Seven_seg_data sd);
        int update_periodic(std::string item, Seven_seg_data sd);
        int remove_periodic(std::string item);
        void clear_periodic();
        void start_periodic();
        void stop_periodic();

    private: 
        std::map<std::string, Seven_seg_data> periodic_data;
        std::mutex pdata_struct_lock;   //lock for adding elements to map
        std::mutex pdata_val_lock;     //lock for updating value of exisitng elements
        Arduino* connection;   
        bool run_periodic = false;
        std::thread periodicThread;
        //thread, async functions
        void periodic_update();
        void _add_periodic(std::string item, Seven_seg_data sd); 
        int   _remove_periodic(std::string item);
        void _clear_periodic();
        

};

#endif

