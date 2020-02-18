#include "libgpsmm.h"
#include <thread>
#include <string>
#include "jetsonGPIO_Orbitty.h"
#include "Adafruit_BNO055_port.h"
#include "jetsonGPIO_Orbitty.h"
#include <mutex>

class GPSPoller
{
  private:
     struct gps_data_t* currentdata = NULL;  // pointer so hopefully thread will update this memory correctly - need to use mutex to access and update
     gpsmm  gpsDev; 
     std::thread updateThread;
     bool   update = true;  //start true can turn off later if needed. 
  public:
      GPSPoller(); //constructor
     ~GPSPoller(); //destructor 
      void updateData(void);
      void stopUpdate()  {update = false;} 
      struct gps_data_t* getCurrentData() {return currentdata;}

};

struct ZEDTime
{
 unsigned long long ts; //timestamp
 std::mutex mutexTS; // mutex for ts

};

void GPSlogger(GPSPoller &gps, bool &recdStatus, std::string gpsFile);
void IMUlogger(BNO055 &imuDev, ZEDTime &ZEDts, bool &recdStatus, std::string outFile);
void ZEDrecorder(sl::Camera &zed, ZEDTime &ZEDts, std::string svo_path, bool &recdStatus);
void switchPoller(jetsonGPIONumber &switchID);
void waitForEdge(jetsonGPIONumber &switchID);
int pushDuration(jetsonGPIONumber &switchID);
void setSystemTime(GPSPoller &gps);
