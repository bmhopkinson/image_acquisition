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
     //type? time;

  public:
      GPSPoller(); //constructor
     ~GPSPoller(); //destructor  
      void stopUpdate()  {update = false;} 
      struct gps_data_t* getCurrentData() {return currentdata;}

    double get_lat() const;
    double get_lon() const;
    int get_mode() const;
    //type? get_time() const;

};

