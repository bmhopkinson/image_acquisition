#include "libgpsmm.h"
//#include "gps.h"
#include <iostream>

using namespace std;
static int WAITING_TIME = 5000000;  //waiting time in usec

int main(void){

    gpsmm gps_rec("localhost", DEFAULT_GPSD_PORT);
    
    if(gps_rec.stream(WATCH_ENABLE|WATCH_JSON) == NULL){
       cout << "No GPSD running!"<<endl;
       return 1;
    }
    
    for(;;){
      if(!gps_rec.waiting(WAITING_TIME)){ 
         continue;
      }
      struct gps_data_t* newdata;
      if((newdata = gps_rec.read()) == NULL){
        cout << "Read error" << endl;
        return 1;
      } else { 
        cout << "probably successful data acquisition"<<endl;
        (void)fprintf(stdout, "flags (0x%04x) %s\n",
             (unsigned int)newdata->set, gps_maskdump(newdata->set));
        cout << "satellites used " << newdata->satellites_used << endl; 
        if(newdata->set & TIME_SET){
          (void)fprintf(stdout, "TIME: %lf\n", newdata->fix.time);
          char  isoTime[128];
          (void)unix_to_iso8601(newdata->fix.time, isoTime, sizeof(isoTime));
          (void)fprintf(stdout, "TIME ISO: %s\n", isoTime);
        } 
        if(newdata->set & LATLON_SET){
           (void)fprintf(stdout, "Lat/Lon: %lf %lf\n", 
                         newdata->fix.latitude, newdata->fix.longitude);
        }
      }
    }//end for loop


}

