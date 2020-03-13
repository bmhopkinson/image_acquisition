#include "system/GPIO.h"
#include "system/LogUtils.h"
#include <sl/Camera.hpp>
#include <iostream>
#include<chrono>
#include<thread>
#include<stdlib.h>
#include <time.h>

double DUR_THRESHOLD = 2000; //threshold for long button press, milliseconds

using namespace sl;
int main(int argc, char **argv) {
    // Record timestamp
	time_t rawtime;
	time(&rawtime);
	struct tm * now = localtime(&rawtime);
    
    // Make log, year, and month+day directories as needed
    char logdir[15];
	strftime(logdir,15,"log/%Y/%m-%d",now);
	mkdir_recursive(logdir, 0x1FF);
   
   //setup GPIO
    unsigned int pin1  =std::atoi(argv[1]);
    std::cout << "Reading input on pin "<< pin1 << std::endl;
    GPIO gpio_in;
    bool result = gpio_in.export_pin(pin1, GPIO::INPUT, GPIO::BOTH, true);
    std::cout << "export in pin? " << result << "\n";
/*
     unsigned int pin2  =std::atoi(argv[2]);
    GPIO gpio_led;
    result = gpio_led.export_pin(pin2, GPIO::OUTPUT);
    std::cout << "export led pin? " << result << " "<<pin2 << "\n";
      std::chrono::seconds sleep_dur(1);
      for(int i = 0; i < 10; i++){
       std::cout <<"high\t" << std::flush;
       gpio_led.set_value(1);
       std::this_thread::sleep_for(sleep_dur);
       std::cout <<"low\t" << std::flush;
       gpio_led.set_value(0);
       std::this_thread::sleep_for(sleep_dur);
    }
*/
   std::cout << std::endl;
    // Create a ZED camera object
    Camera zed;
    // Set initial parameters
    InitParameters init_params;
    init_params.camera_resolution = RESOLUTION_HD720; // Use HD720 video mode (default fps: 60)
    init_params.coordinate_units = UNIT_METER; // Set units in meters
    // Open the camera
    ERROR_CODE err = zed.open(init_params);
    if (err != SUCCESS) {
        std::cout << toString(err) << std::endl;
        exit(-1);
    }

   	// Create ZED stereo grayscale images
	Mat zed_image_l(zed.getResolution(), MAT_TYPE_8U_C1);
	Mat zed_image_r(zed.getResolution(), MAT_TYPE_8U_C1);
  

    gpio_in.monitor_button();
    bool b_active = true;
    bool b_record = false;
    std::cout << "entering active loop" << std::endl;
    while(b_active){
        if(gpio_in.get_activity()){ // button pressed
            std::cout << "activity detected in active loop" <<std::endl;
            if(gpio_in.get_duration() < DUR_THRESHOLD){ //start recording - short button press
                gpio_in.clear_activity();

                // Enable ZED video recording
                char svofile[50];
                time(&rawtime);
                struct tm * start_time = localtime(&rawtime);
	            strftime(svofile,50,"./log/%Y/%m-%d/%F_%H-%M-%S_video.svo",start_time);
                err = zed.enableRecording(svofile, SVO_COMPRESSION_MODE_AVCHD);
                if (err != SUCCESS) {
                    std::cout << toString(err) << std::endl;
                    exit(-1);
                }
                b_record = true;
              //  gpio_led.set_value(1);
            } else if(gpio_in.get_duration() > DUR_THRESHOLD){    // terminate program - long button press
                gpio_in.clear_activity();
                b_active = false;
            }
        }
        //record video data
        double init_time = static_cast<double>(zed.getTimestamp(TIME_REFERENCE::TIME_REFERENCE_CURRENT)) / 1E9;
        int i = 0;
        while (b_record) {
            // Grab a new frame
            if (zed.grab() == SUCCESS) {
                // Record the grabbed frame in the video file
                double timestamp =  (static_cast<double>(zed.getTimestamp(TIME_REFERENCE::TIME_REFERENCE_CURRENT)) / 1E9) - init_time;
			    zed.retrieveImage(zed_image_l, VIEW_LEFT_GRAY);
			    zed.retrieveImage(zed_image_r, VIEW_RIGHT_GRAY);
                zed.record();
                i++;
            }
            if(gpio_in.get_activity()){
                b_record = false;
           //     gpio_led.set_value(0);
                zed.disableRecording();
                std::cout << "Video has been saved ..." << std::endl;
                if(gpio_in.get_duration() > DUR_THRESHOLD){
                    b_active = false;   
                }
                gpio_in.clear_activity();
            }
        }  //record loop
        
    } //active loop


    zed.close();
    return 0;
}

