#ifndef GPSPOLLER_H_
#define GPSPOLLER_H_

#include "libgpsmm.h"
#include <thread>
#include <string>

class GPSPoller
{
  private:
     //underlying data
     struct gps_data_t* currentdata = NULL;  // pointer so hopefully thread will update this memory correctly - need to use mutex to access and update
     gpsmm  gpsDev; 
     std::thread updateThread;
     bool   update = true;  //start true can turn off later if needed. 
     int wait_time = 500*1000;   //us to wait for new gps data
     void update_data(struct gps_data_t* gpsdata);
     void poll_device();

     // user relevant data
     double lat;
     double lon;
     int mode;
     int sats_used;
     std::string iso_time;
     //type? time;

  public:
      GPSPoller(); //constructor
     ~GPSPoller(); //destructor  
      void stopUpdate()  {update = false;} 
      struct gps_data_t* getCurrentData() {return currentdata;}

    double get_lat() const { return lat; };
    double get_lon() const { return lon; };
    int get_mode() const { return mode; };
    std::string get_time() const { return iso_time; };
    //type? get_time() const;

};

#endif  //include guard

