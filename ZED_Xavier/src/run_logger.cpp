#include "system/GPIO.h"
#include "system/PerfMonitor.h"
#include "system/LogUtils.h"
#include "system/Helpers.h"
#include "Logger.h"
#include <sl/Camera.hpp>
#include <iostream>
#include<chrono>
#include<stdlib.h>
#include <time.h>
#include <yaml-cpp/yaml.h>

//double DUR_THRESHOLD = 2000; //threshold for long button press, milliseconds

using namespace sl;
using namespace std;
int main(int argc, char **argv) {
	// Record timestamp
	time_t rawtime;
	time(&rawtime);
	struct tm * now = localtime(&rawtime);
	
	// Make log, year, and month+day directories as needed
	char logdir[150];
	strftime(logdir,150,"/home/nvidia/Documents/image_acquisition/ZED_Xavier/bin/log/%Y/%m-%d",now);
	mkdir_recursive(logdir, 0x1FF);

	//load configuration information
	YAML::Node config_file;
	config_file = YAML::LoadFile("/home/nvidia/Documents/image_acquisition/ZED_Xavier/config/zed_xavier_config_1.yaml");
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

	//create performance monitor
	PerfMonitor pm(logdir);

	gpio_in.monitor_button();
	bool b_active = true;
    bool b_record = false;

    //create Logger
   Logger sys_xav(config_file); 
   
	std::cout << "entering active loop" << std::endl;
	while(b_active){

		if(gpio_in.get_activity() && !b_record){ // button pressed -  //start recording or shutdown
			std::cout << "activity detected in active loop" <<std::endl;
			if(gpio_in.get_duration() < DUR_THRESHOLD){ //start recording - short button press
			    gpio_in.clear_activity();

				// Enable  recording
				time(&rawtime);
				struct tm * start_time = localtime(&rawtime);
                sys_xav.initialize_recording(start_time);

                std::chrono::high_resolution_clock::time_point  sys_tref = std::chrono::high_resolution_clock::now();//system time
	            sys_xav.record_start(sys_tref);
                b_record = true;
 
			} else if(gpio_in.get_duration() > DUR_THRESHOLD){    // terminate program - long button press
				 gpio_in.clear_activity();
				 b_active = false;
			}
		} // end if on activity and not recording 
	
		if(gpio_in.get_activity() && b_record){  //stop recording - possibly shutdown
            std::cout << "activity detected in active loop" <<std::endl;
		    b_record = false;
            sys_xav.record_stop();

			if(gpio_in.get_duration() > DUR_THRESHOLD){  //check whether to exit program
				b_active = false;   
			}
			gpio_in.clear_activity();
		}// end if on activity and recording 
        
	} //active loop

	return 0;
}

