#include <thread>
#include <iostream>
#include "libgpsmm.h"
#include "Adafruit_BNO055_port.h"
#include <unistd.h>
#include <sl/Camera.hpp>
#include "jetsonGPIO_Orbitty.h"


static int WAITING_TIME = 5000000;  //waiting time in usec
using namespace std;

void GPSlogger()
{
  cout << "in a new thread!\n";
  gpsmm gps_rec("localhost", DEFAULT_GPSD_PORT);
    
    if(gps_rec.stream(WATCH_ENABLE|WATCH_JSON) == NULL){
       cout << "No GPSD running!"<<endl;
    }
    
    for(int i = 0 ; i <30; i++){
      if(!gps_rec.waiting(WAITING_TIME)){ 
         continue;
      }
      struct gps_data_t* newdata;
      if((newdata = gps_rec.read()) == NULL){
        cout << "Read error" << endl;
      } else {
        if(newdata->set & TIME_SET){ 
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

}//end GPSlogger

void IMUlogger()
{
  BNO055 imu = BNO055( BNO055_ADDRESS);
  int I2CBus = 1;
  imu.begin(I2CBus);

 for(int i = 0; i < 30 ; i++){
   imu::Quaternion Q;
   Q = imu.getQuat();
   double w, x, y, z;
   w = Q.w(); x = Q.x(), y = Q.y(); z = Q.z();
   cout << "Quaternion: w: " << w << " x: "<< x << " y: " << y << " z: " << z << endl;
   imu::Vector<3> xyz;
   xyz = imu.getEuler();     
   cout << "Euler: x: " << xyz[0] << " y: "<< xyz[1] <<" z: " << xyz[2] << endl;
   usleep(500*1000);
  }
}//end IMUlogger

void ZEDrecorder(){

   sl::Camera zed;
    sl::InitParameters initParameters;
    initParameters.camera_fps = 15;
    initParameters.camera_resolution = sl::RESOLUTION_HD720;
    initParameters.depth_mode = sl::DEPTH_MODE_NONE;

    string svo_path = "/media/ubuntu/Orbitty_USB/record_trial_output1.svo"; // need to make sure system has permission to write and create on media directory

 // Open the ZED
    sl::ERROR_CODE err = zed.open(initParameters);
    if (err != sl::SUCCESS) {
        cout << sl::errorCode2str(err) << endl;
        zed.close();
    }

   cout << "opened the ZED" << endl;

  //SETUP TO RECORD
   err = zed.enableRecording(svo_path.c_str(), sl::SVO_COMPRESSION_MODE::SVO_COMPRESSION_MODE_LOSSY);
   if (err != sl::SUCCESS) {
         std::cout << "Error while recording. " << errorCode2str(err) << " " << err << std::endl;
         if (err == sl::ERROR_CODE_SVO_RECORDING_ERROR) {
           std::cout << " Note : This error mostly comes from a wrong path or missing writting permissions..." << std::endl;
         }
         zed.close();
    }


  //MAIN RECORDING LOOP
    int numFrames = 300;
    sl::Mat view;
    int i=0;
    while(i<numFrames){
        if (!zed.grab()) {

        // Get the side by side image
        zed.retrieveImage(view, sl::VIEW_SIDE_BY_SIDE);
        zed.record(); // record
        i++;
        cout << i << endl;
       } else sl::sleep_ms(1);
    }  //end main recording loop

   zed.disableRecording(); 
   zed.close();

}//end ZEDrecorder



int main(void){

  jetsonGPIONumber sysLED = gpio0; //indicates system (this program) is running
  jetsonGPIONumber recLED = gpio1; //indicates data is being recorded.
  jetsonGPIONumber inSwch = gpio2; //input switch, wired internally with positive pull up resistor, so 1 = not pressed, 0 = pressed.

  gpioExport(sysLED);
  gpioExport(recLED);
  gpioExport(inSwch);

  gpioSetDirection(sysLED, outputPin);
  gpioSetDirection(recLED, outputPin);
  gpioSetDirection(inSwch,  inputPin);

   // Flash the LEDs 5 times
  for(int i=0; i<5; i++){
      cout << "Setting the LED on" << endl;        
      gpioSetValue(sysLED, off);
      gpioSetValue(recLED, on);
      usleep(500*1000);         // on for 500ms
      cout << "Setting the LED off" << endl;
      gpioSetValue(sysLED, on);
      gpioSetValue(recLED, off);
      usleep(500*1000);         // off for 500ms
   }

  for(int j = 0; j <25; j++){   //read the input 5 times
     unsigned int inVal[1];
     gpioGetValue( inSwch, inVal ) ;
     cout << "Button value is: " << *inVal << endl;
     usleep(1000*1000);
  }


//   thread GPSthread(GPSlogger);
   thread IMUthread(IMUlogger);
   thread ZEDthread(ZEDrecorder);
   cout << "started all the threads" <<endl;
  // GPSthread.join();
   IMUthread.join();
   ZEDthread.join();

   gpioUnexport(sysLED);     // unexport the system LED
   gpioUnexport(recLED);     // unexport the record LED
   gpioUnexport(inSwch);     // unexport the record LED
   return 0;

}
