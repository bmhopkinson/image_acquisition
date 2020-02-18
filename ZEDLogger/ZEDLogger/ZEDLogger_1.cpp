#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <condition_variable>
#include "libgpsmm.h"
#include "Adafruit_BNO055_port.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sl/Camera.hpp>
#include "jetsonGPIO_Orbitty.h"
#include "ZEDLogger_1.hpp"
#include <chrono>
#include <ctime>



static int WAITING_TIME = 500*1000;  //waiting time in usec
using namespace std;



int main(void){


// SETUP GPIO

  jetsonGPIONumber sysLED = gpio0; //indicates system (this program) is running
  jetsonGPIONumber recLED = gpio1; //indicates data is being recorded.
  jetsonGPIONumber inSwch = gpio2; //input switch, wired internally with positive pull up resistor, so 1 = not pressed, 0 = pressed.

  gpioExport(sysLED);
  gpioExport(recLED);
  gpioExport(inSwch);

  gpioSetDirection(sysLED, outputPin);
  gpioSetDirection(recLED, outputPin);
  gpioSetDirection(inSwch,  inputPin);

  gpioSetValue(sysLED, on);
  gpioSetValue(recLED, off);
   
// STARTUP DEVICES
//  IMU - STARTUP
  BNO055 imu = BNO055( BNO055_ADDRESS);
  int I2CBus = 1;
  imu.begin(I2CBus);
  
//  GPS - STARTUP

  GPSPoller gpsAda;
  std::this_thread::sleep_for(std::chrono::seconds(2));//allow the poller to get up and running before attempting to access data

// use GPS to set system clock time
   thread TIMEthread(setSystemTime, std::ref(gpsAda));

//  ZED -  STARTUP  
  sl::Camera zed;
  sl::InitParameters initParameters;
  initParameters.camera_fps = 15; 
  initParameters.camera_resolution = sl::RESOLUTION_HD720;
  initParameters.depth_mode = sl::DEPTH_MODE_NONE;
//associate ZED timestamp structure
  ZEDTime ZEDts; 


 // Open the ZED
  sl::ERROR_CODE err = zed.open(initParameters);
  if (err != sl::SUCCESS) {
      cout << sl::errorCode2str(err) << endl;
      zed.close();
  }

   cout << "opened the ZED" << endl;
   
  // main loop on recording
  bool progStatus = true;
  bool recdStatus = false;
  thread IMUthread;
  thread ZEDthread; 
  thread GPSthread;

  int nDataSet = 1; // data set number
  srand(time(NULL)); //seed for random number generator - use time so its not repeated. 
  int idDataSet = rand(); // random number to identify data sets

  while(progStatus){
     waitForEdge(inSwch); //wait for button press
     cout << "detected button press" << endl;
     int duration = pushDuration(inSwch); // duration of button press in 50 ms units.
     cout << "duration of press (50ms units): "<<duration <<endl;
     
     if(duration > 5 && duration < 60){  //short button press
        if(recdStatus == false){  // start recording
            cout << "start recording"<< endl;
            recdStatus = true;
            gpioSetValue(recLED, on);
  	    string svo_path = "/mnt/usb-Kingston_DataTraveler_3.0_B8AEEDBAF20EB140299855A3-0:0-part1/record_trial_output"; // need to make sure system has permission to write and create on media directory
            svo_path = svo_path + "_"+ to_string(idDataSet) + "_" + to_string(nDataSet) + ".svo";
	    ZEDthread = thread(ZEDrecorder, std::ref(zed),std::ref(ZEDts), svo_path, std::ref(recdStatus));

	    string imuFile = "/home/ubuntu/Documents/ZEDLogger_data/IMU/imudata";
            imuFile = imuFile + "_"+ to_string(idDataSet) + "_" + to_string(nDataSet) + ".txt";
            IMUthread = thread(IMUlogger  , std::ref(imu), std::ref(ZEDts), std::ref(recdStatus), imuFile );
 
            string gpsFile = "/home/ubuntu/Documents/ZEDLogger_data/GPS/gpsdata";
            gpsFile = gpsFile + "_"+ to_string(idDataSet) + "_" + to_string(nDataSet) + ".txt";
            GPSthread = thread(GPSlogger, std::ref(gpsAda), std::ref(recdStatus), gpsFile);
            
            nDataSet++;
        }
        
        else if(recdStatus == true){ // stop recording
            //SIGNAL TO STOP THREADS
            cout << "stop recording"<< endl;
            recdStatus = false;  //signals threads to stop logging data
            ZEDthread.join();
            IMUthread.join();
            GPSthread.join();
            gpioSetValue(recLED, off);
         //   continue; //wait for next button press
        }
     
     }  //end if on short button press
     
     else if(duration >= 60){ //long button press
      // would like to reset the system here after exiting while loop
        if(recdStatus == true){  //should package everything below into a "stop_record" function
            cout << "stop recording"<< endl;
            recdStatus = false;  //signals threads to stop logging data
            ZEDthread.join();
            IMUthread.join();
            GPSthread.join();
            gpioSetValue(recLED, off);
        }
        progStatus = false;
     }// end if on long button press
      

  }//end progStatus While lloop

 //SHUTDOWN///

   // CLOSE  DEVICES

   // GPS - CLOSE
   gpsAda.stopUpdate();
   
   // IMU - CLOSE
   
   
   // ZED - CLOSE
   zed.close();

   //clean up GPIOs
   gpioUnexport(sysLED);     // unexport the system LED
   gpioUnexport(recLED);     // unexport the record LED
   gpioUnexport(inSwch);     // unexport the record LED

   // join timeSet thread
   TIMEthread.join();
   //restart computer?

   return 0;

}


// GPS Poller Class member functions

//Default Constructor
GPSPoller::GPSPoller() : gpsDev("localhost", DEFAULT_GPSD_PORT)
{
 // gpsmm gpsDev2("localhost", DEFAULT_GPSD_PORT);
  if(gpsDev.stream(WATCH_ENABLE|WATCH_JSON) == NULL){
       cout << "No GPSD running!"<<endl;
    }
    
    updateThread = std::thread(&GPSPoller::updateData, this);
}
 
GPSPoller::~GPSPoller()
{
   updateThread.join();
}

void GPSPoller::updateData(void)
{     
    while(update){ 
      if(!gpsDev.waiting(WAITING_TIME)){ 
         continue;
      }
    struct gps_data_t* newdata;
    if((newdata = gpsDev.read()) == NULL){
        cout << "Read error" << endl;
      } else {
        currentdata = newdata;  // THIS IS THE ACTION     
      }  
    }//end while
}



//  END GPSPoller Class


void GPSlogger(GPSPoller &gps, bool &recdStatus, string gpsFile){
  ofstream ofile;
  ofile.open(gpsFile.c_str());
  while(recdStatus){ 
    struct gps_data_t* gpsdata;
    gpsdata = gps.getCurrentData();

    if((gpsdata->set & TIME_SET) && (gpsdata->set & LATLON_SET)){
          char  isoTime[128];
          (void)unix_to_iso8601(gpsdata->fix.time, isoTime, sizeof(isoTime));
          int mode = -1;
          if(gpsdata->set & MODE_SET){
               mode = gpsdata->fix.mode;
          } 
              	
          ofile << isoTime << "\t" << mode << "\t" << setprecision(9) << gpsdata->fix.latitude << "\t" << gpsdata->fix.longitude << endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  } //end while on recdStatus
 
  ofile.close();
}


void IMUlogger(BNO055 &imuDev,  ZEDTime &ZEDts, bool &recdStatus, string imuFile)
{
  ofstream ofile;  //create and then open output file
  ofile.open(imuFile.c_str());
  //header line
  ofile << "GPS time" <<"\t" << "ZED timestamp" <<"\t"<< "Temp (C)" << "\t" <<  "Sys_cal" << "\t"<< "accel_cal" << "\t" << "gyro_cal" << "\t" << "mag_cal"<< "\t" <<
        "w_quat"<< "\t" << "x_quat"<< "\t" << "y_quat" << "\t" << "z_quat" << "\t" << "x_euler" << "y_euler" << "\t" << "z_euler" << endl;
 while(recdStatus){
   imu::Quaternion Q;
   imu::Vector<3> xyz;
   
   //acquire critical IMU data: orientation (Quat and Euler) and timestamps 
   Q = imuDev.getQuat();
   xyz = imuDev.getEuler();     
   chrono::time_point<chrono::system_clock> imuTime;
   imuTime = chrono::system_clock::now(); // acquire timestamp - checked that total time to get Quat, Euler, and Timestamp is ~1ms - for my purposes these data are effectively synchronous 
   ZEDts.mutexTS.lock();
   unsigned long long tsZed = ZEDts.ts; // timestamp of most recent ZED frame
   ZEDts.mutexTS.unlock();

   //acquire supporting IMU information: temperature and calibration state
   int8_t T;
   T = imuDev.getTemp();

   uint8_t sys, gyro, accel, mag;
   uint8_t* psys    = &sys;
   uint8_t* pgyro   = &gyro;
   uint8_t* paccel  = &accel; 
   uint8_t* pmag    = &mag;
   imuDev.getCalibration( psys, pgyro, paccel, pmag);
   
   // refine and save IMU data   
   double w, x, y, z;
   w = Q.w(); x = Q.x(), y = Q.y(); z = Q.z();
   
   chrono::duration<double> imuEpochTime = imuTime.time_since_epoch();
   char  isoTime[128];
  (void)unix_to_iso8601(imuEpochTime.count(), isoTime, sizeof(isoTime));  //from gpsd library
   
   ofile << isoTime << "\t"<< tsZed <<"\t"<< static_cast<int>(T) << "\t"<<  static_cast<int>(sys) << "\t" << static_cast<int>(accel) << "\t" << static_cast<int>(gyro) << "\t"<< static_cast<int>(mag)
         << "\t" << setprecision(6) << w << "\t" << x  << "\t" << y  << "\t" << z << "\t" << xyz[0]  << "\t" << xyz[1]  << "\t" << xyz[2] << endl;
   std::this_thread::sleep_for(std::chrono::milliseconds(50)); // will give imu data at roughly 20Hz - similar to video framerate
  }// end while loop

  ofile.close();
}//end IMUlogger


void ZEDrecorder(sl::Camera &zed, ZEDTime &ZEDts, string svo_path, bool &recdStatus){

  //SETUP TO RECORD
   sl::ERROR_CODE err = zed.enableRecording(svo_path.c_str(), sl::SVO_COMPRESSION_MODE::SVO_COMPRESSION_MODE_LOSSY);
   if (err != sl::SUCCESS) {
         std::cout << "Error while recording. " << errorCode2str(err) << " " << err << std::endl;
         if (err == sl::ERROR_CODE_SVO_RECORDING_ERROR) {
           std::cout << " Note : This error mostly comes from a wrong path or missing writting permissions..." << std::endl;
         }
         zed.close();
    }

  //MAIN RECORDING LOOP
    sl::Mat view;
     
  ofstream ofile;  //create and then open output file
  ofile.open("ZEDtimestampdata.txt");
   int frameNum = 0;
   while(recdStatus){
        if (!zed.grab()) {
         frameNum++;
        //timestamp
        unsigned long long ts = zed.getCameraTimestamp();  // time in ns since computer start.
        ZEDts.mutexTS.lock();
        ZEDts.ts = ts;
        ZEDts.mutexTS.unlock();
         ofile << frameNum << "\t" << ts << endl;
        // Get the side by side image
        zed.retrieveImage(view, sl::VIEW_SIDE_BY_SIDE);
        zed.record(); // record
     
       } else sl::sleep_ms(1);
    }  //end main recording loop

    ofile.close();
   zed.disableRecording(); 

}//end ZEDrecorder


void switchPoller(jetsonGPIONumber &switchID)
{
   bool pressed = false;
  cout << "switchID: " << switchID << endl;
  // while(!pressed){
   for(int i = 0; i<20; i++){
     unsigned int inVal[1];
     gpioGetValue( switchID, inVal ) ;
     cout << "switch value: "<< inVal <<endl;

     std::this_thread::sleep_for (std::chrono::seconds(1));
  }//end for loop
   //} //end while loop

}

void waitForEdge(jetsonGPIONumber &switchID)
{
   bool pressed = false;
   while(!pressed){
     unsigned int inVal[1];
     gpioGetValue( switchID, inVal ) ;
     if(inVal[0] == 0){
         pressed = true;
     }
     
   std::this_thread::sleep_for (std::chrono::milliseconds(100)); 
   } //end while  
 

} // end waitForEdge function

int pushDuration(jetsonGPIONumber &switchID){
     bool released = false;
     int counts = 0;
     while(!released){
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        counts++;
        unsigned int inVal[1];
        gpioGetValue( switchID, inVal ) ;
        if(inVal[0] == 1){
          released = true;
        }
        
     } // end on released while  
   return counts;
} // end pushDuration function     

void setSystemTime(GPSPoller &gps)  //sets system time from GPS since there is no realtime clock
{
   bool timeSet = false;
   struct gps_data_t* gpsdata = NULL;
   
   while(!timeSet){
      gpsdata = gps.getCurrentData(); 
      
      if(gpsdata != NULL){
       if(gpsdata->set & TIME_SET){
          //set system time
          char  isoTime[128];
          (void)unix_to_iso8601(gpsdata->fix.time, isoTime, sizeof(isoTime));
          char command[300];
          snprintf(command, 299, "echo \"ubuntu\" | sudo -S date -s %s --utc", isoTime);
          cout << "command to set time?: " << command << endl;
          system(command);
          timeSet = true;
       }
     }
  
     std::this_thread::sleep_for(std::chrono::seconds(1));


   } // end while on timeSet
}

