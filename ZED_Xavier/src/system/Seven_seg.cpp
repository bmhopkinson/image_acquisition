#include "system/Seven_seg.h"
#include <chrono>
#include <future>
#include <iostream>

Seven_seg::Seven_seg(){
}

Seven_seg::~Seven_seg(){
   if(periodicThread.joinable()){  //if stop_periodic was not explicitly called.
       run_periodic = false;
       periodicThread.join();
   }
}

void Seven_seg::set_connection(Arduino* conn){
    connection = conn;
}

void Seven_seg::display_now(std::string str){
     std::string ard_msg = "#7seg," + str + "\n"; 
     connection->send(ard_msg);
}

void Seven_seg::add_periodic(std::string item, Seven_seg_data sd){
   std::async(std::launch::async, &Seven_seg::_add_periodic, this, item, sd);  //pdata_struct_lock is released infrequently so modify asynchronously
}

void Seven_seg::_add_periodic(std::string item, Seven_seg_data sd){
   std::lock_guard<std::mutex> lock(pdata_struct_lock);
   periodic_data[item] = sd;
}

int Seven_seg::update_periodic(std::string item, Seven_seg_data sd){
    std::lock_guard<std::mutex> lock_data(pdata_val_lock);
 
    std::map<std::string, Seven_seg_data>::iterator it; 
    it = periodic_data.find(item);   //check to ensure item exists in map ;//should really acquire structure lock
    if(it != periodic_data.end()){
         periodic_data[item] = sd;
        return 0;
    }
    else {return -1;}
}

int Seven_seg::remove_periodic(std::string item){
 std::async(std::launch::async, &Seven_seg::_remove_periodic, this, item);  //pdata_struct_lock is released infrequently so modify asynchronously
}

int Seven_seg::_remove_periodic(std::string item){
    std::lock_guard<std::mutex> lock(pdata_struct_lock);

    std::map<std::string, Seven_seg_data>::iterator it; 
    it = periodic_data.find(item);   //check to ensure item exists in map; 
    if(it != periodic_data.end()){
        periodic_data.erase(item);
        return 0;
    }
    else {return -1;}
}

void Seven_seg::start_periodic(){
    run_periodic = true;
    periodicThread = std::thread(&Seven_seg::periodic_update, this);
}

void Seven_seg::stop_periodic()
{
   run_periodic = false;
   periodicThread.join();
}

void Seven_seg::clear_periodic()
{
    std::async(std::launch::async, &Seven_seg::_clear_periodic, this); 
}

void Seven_seg::_clear_periodic(){
    std::lock_guard<std::mutex> lock(pdata_struct_lock);
    periodic_data.clear();
}

void Seven_seg::periodic_update(){
    std::map<std::string, Seven_seg_data>::iterator it; 
    while(run_periodic){
        pdata_struct_lock.lock();
        for(it = periodic_data.begin(); it !=periodic_data.end(); it++){
            pdata_val_lock.lock();
            Seven_seg_data tdata = it->second;
            pdata_val_lock.unlock();

            std::string ard_msg = "#7seg," + tdata.msg + "\n"; 
            connection->send(ard_msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(tdata.duration));
        }
        pdata_struct_lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); //allow periodic_data to stay unlocked for other threads to update
   }
}
