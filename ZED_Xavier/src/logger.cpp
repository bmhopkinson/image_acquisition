#include "system/GPIO.h"
#include "system/GPSPoller.h"
#include "system/PerfMonitor.h"
#include "system/LogUtils.h"
#include "system/Helpers.h"
#include <sl/Camera.hpp>
#include <iostream>
#include <fstream>
#include<chrono>
#include<thread>
#include<stdlib.h>
#include <time.h>
#include <yaml-cpp/yaml.h>

//to do - create performance logger that records cpu/gpu usage, cpu temp, fan speed, etc over time
// - run on battery and take around - perhaps set camera in front of car, hook gps to roof

//double DUR_THRESHOLD = 2000; //threshold for long button press, milliseconds

using namespace sl;
using namespace std;
int main(int argc, char **argv) {
    // Record timestamp
	time_t rawtime;
	time(&rawtime);
	struct tm * now = localtime(&rawtime);
    
    // Make log, year, and month+day directories as needed
    char logdir[15];
	strftime(logdir,15,"log/%Y/%m-%d",now);
	mkdir_recursive(logdir, 0x1FF);

    //load configuration information
	YAML::Node config_file;
    config_file = YAML::LoadFile("../config/zed_xavier_config.yaml");
    YAML::Node button_ss = config_file["gpio.button_startstop"];
    unsigned int pin1 = button_ss["pin_id"].as<unsigned int>();
	double DUR_THRESHOLD = button_ss["longthresh"].as<double>();

   //setup GPIO
   // unsigned int pin1  =std::atoi(argv[1]);
    std::cout << "Reading input on pin "<< pin1 << std::endl;
    GPIO gpio_in;
    bool result = gpio_in.export_pin(pin1, GPIO::INPUT, GPIO::BOTH, true);
    std::cout << "export in pin? " << result << "\n";
   std::cout << std::endl;

    // Create a ZED camera object
    Camera zed;
    // Set initial parameters
	YAML::Node zed_config = config_file["zed"];
    InitParameters init_params;
	init_params.camera_resolution = str_to_resolution(zed_config["resolution"].as<string>());
	init_params.camera_fps = zed_config["fps"].as<int>();
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
 
    //create GPSPoller
    GPSPoller gps;

	//create performance monitor
	PerfMonitor pm(logdir);

    gpio_in.monitor_button();
    bool b_active = true;
    bool b_record = false;

    std::ofstream data_file;  //gps data and relevant timestamps will be written here
   
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

                //open data file
                char datafname[50];
    			strftime(datafname,50,"./log/%Y/%m-%d/%F_%H-%M-%S_data.txt",start_time);
                data_file.open(datafname, std::ios::out);
                data_file << "i\t" <<"sys_time_local\t"<< "sys_time_rel (s)\t" << "frame_timestamp (ns)\t" << "gps_time\t" << "lat\t" << "lon\t"<< "qx\t"<< "qy\t"<< "qz\t" << "qw\n";

                b_record = true;
              //  gpio_led.set_value(1);
            } else if(gpio_in.get_duration() > DUR_THRESHOLD){    // terminate program - long button press
                gpio_in.clear_activity();
                b_active = false;
            }
        }
        //record video data
        //double init_time = static_cast<double>(zed.getTimestamp(TIME_REFERENCE::TIME_REFERENCE_CURRENT)) / 1E9;
        std::chrono::high_resolution_clock::time_point  sys_tref = std::chrono::high_resolution_clock::now();//system time
        int i = 0;
        while (b_record) {
            // Grab a new frame
            if (zed.grab() == SUCCESS) {
                
                // calculate a bunch of times and timestamps for debugging and synchronizing
				// high resolution relative time
                std::chrono::high_resolution_clock::time_point  sys_tcur = std::chrono::high_resolution_clock::now();//system time
                chrono::duration<double> trel_sys = chrono::duration_cast<chrono::duration<double>>(sys_tcur - sys_tref); //elapsed time

				// coarse system time (absolute, local)
				time(&rawtime);
				struct tm * now = localtime(&rawtime);
    			char curtime[30];
				strftime(curtime,30,"%Y-%m-%d_%H:%M:%S",now);

                timeStamp frame_timestamp = zed.getTimestamp(TIME_REFERENCE_IMAGE);
			   // zed.retrieveImage(zed_image_l, VIEW_LEFT_GRAY);
			   // zed.retrieveImage(zed_image_r, VIEW_RIGHT_GRAY);

				//retrieve IMU data
				IMUData zed_imu;
				zed.getIMUData(zed_imu, sl::TIME_REFERENCE::TIME_REFERENCE_IMAGE);
				Orientation quat = zed_imu.getOrientation();

                // Record the grabbed frame in the video file
                zed.record();
                //write auxilary data to file
                data_file << i << "\t" << curtime << "\t" << trel_sys.count() << "\t" 
                             <<  frame_timestamp << "\t" << gps.get_time() << "\t"
                             << fixed <<setprecision(6) << gps.get_lat() << "\t"
                            << fixed <<setprecision(6) << gps.get_lon() << "\t"
							<< quat(0) << "\t" << quat(1) << "\t"<< quat(2) << "\t"<< quat(3) << std::endl;
                i++;
            }
            if(gpio_in.get_activity()){
                b_record = false;
           //     gpio_led.set_value(0);
                zed.disableRecording();
                std::cout << "Video has been saved ..." << std::endl;
                data_file.close();
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

