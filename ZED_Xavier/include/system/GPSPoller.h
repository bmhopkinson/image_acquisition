#ifndef GPSPOLLER_H_
#define GPSPOLLER_H_

#include "libgpsmm.h"
#include <thread>
#include <string>

class GPSPoller
{
  public:
      GPSPoller(); //constructor
     ~GPSPoller(); //destructor  
      void stopUpdate()  {update = false;} 
      struct gps_data_t* getCurrentData() {return currentdata;}

    double get_lat() const { return lat; };
    double get_lon() const { return lon; };
    double get_lat_err() const {return lat_err;};
    double get_lon_err() const {return lon_err;};
    double get_alt() const {return alt;};
    
    int get_mode() const { return mode; };
    int get_status() const { return status; };
    int get_nsats() const {return sats_used; };
    std::string get_time() const { return iso_time; };
    //type? get_time() const;

  private:
     //underlying data
     struct gps_data_t* currentdata = NULL;  // pointer so hopefully thread will update this memory correctly - need to use mutex to access and update
     gpsmm  gpsDev; 
     std::thread updateThread;
     bool   update = true;  //start true can turn off later if needed. 
     int wait_time = 2000*1000;   //us to wait for new gps data (2 s)
     void update_data(struct gps_data_t* gpsdata);
     void poll_device();

     // user relevant data
     double lat;
     double lon;
     double lat_err; //error in meters (95% CI)
     double lon_err; //error in meters (95% CI)
     double alt  = -999;   //if not in 3d fix mode will not be set (e.g. -999)
     int mode;
     int status;  // 0 = no fix, 1 = normal, 2= dgps
     int sats_used;
     std::string iso_time;
     //type? time;
};

#endif  //include guard

