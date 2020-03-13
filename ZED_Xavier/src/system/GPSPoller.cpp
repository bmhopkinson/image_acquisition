// GPS Poller Class member functions
#include "system/GPSPoller.h"
#include "libgpsmm.h"
#include <thread>
#include <string>
#include <iostream>

//Default Constructor
using namespace std;
GPSPoller::GPSPoller() : gpsDev("localhost", DEFAULT_GPSD_PORT)
{
 // gpsmm gpsDev2("localhost", DEFAULT_GPSD_PORT);
  if(gpsDev.stream(WATCH_ENABLE|WATCH_JSON) == NULL){
       cout << "No GPSD running!"<<endl;
    }
    
    updateThread = std::thread(&GPSPoller::poll_device, this);
}
 
GPSPoller::~GPSPoller()
{ 
   update = false;
   updateThread.join();
}

void GPSPoller::poll_device(void)
{     
    while(update){ 
        if(!gpsDev.waiting(wait_time)){ 
           continue;
        }
        struct gps_data_t* newdata;
        if((newdata = gpsDev.read()) == NULL){
            cout << "Read error" << endl;
        } else {
            currentdata = newdata;       
            update_data(newdata);
      }  
    }//end while
}

void GPSPoller::update_data(struct gps_data_t* gpsdata){
    if(gpsdata->set & TIME_SET){
        char  isoTime[128];
        (void)unix_to_iso8601(gpsdata->fix.time, isoTime, sizeof(isoTime));
        cout << isoTime << "\t";
    }
  
    if(gpsdata->set & LATLON_SET){
        cout.setf(ios_base::fixed, ios_base::floatfield);
        cout.precision(6);
        cout << "lat: " << gpsdata->fix.latitude << "\t" << "long: " << gpsdata->fix.longitude <<"\t" ;\
        lat =  gpsdata->fix.latitude;
        lon = gpsdata->fix.longitude;
    }
  
    if(gpsdata->set & MODE_SET){
        cout << "mode: " << gpsdata->fix.mode << "\t" ;
        mode =  gpsdata->fix.mode;
    } 
    cout << "sats used: " << gpsdata->satellites_used << "\t";
    cout << "sats visible: " << gpsdata->satellites_visible << "\t";
    cout << endl;
}



//  END GPSPoller Class
