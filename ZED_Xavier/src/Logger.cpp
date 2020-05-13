#include "Logger.h"
#include "system/Seven_seg.h"
#include "system/Helpers.h"
#include <iostream>
#include <iomanip>

Logger::Logger(YAML::Node config_file_):config_file(config_file_)
{

   //initialize zed
   	YAML::Node zed_config = config_file["zed"];
	sl::InitParameters init_params;
	init_params.camera_resolution = str_to_resolution(zed_config["resolution"].as<std::string>());
	init_params.camera_fps = zed_config["fps"].as<int>();
	init_params.coordinate_units = sl::UNIT_METER; // Set units in meters
	// Open the camera
	sl::ERROR_CODE err = zed.open(init_params);
	if (err != sl::SUCCESS) {
		std::cout << sl::toString(err) << std::endl;
		exit(-1);
	}

    //start display
   YAML::Node ard_config = config_file["arduino"];

   ard.open_uart(ard_config["port"].as<std::string>(), ard_config["speed"].as<int>());
   disp.set_connection(&ard);
   Seven_seg_data drec, dgps;
   drec.msg ="rec0";
   drec.duration = 750;
   dgps.msg = "gps" + std::to_string(gps.get_mode());  //display gps status
   dgps.duration = 750;
   disp.add_periodic("rec", drec);
   disp.add_periodic("gps", dgps);
   disp.start_periodic();  //begin rotating message display
    
}

Logger::~Logger(){
	zed.close();  
}

void Logger::initialize_recording(struct tm * start_time)
{

	// Enable ZED video recording
	char  svofile[200] = "/home/nvidia/Documents/image_acquisition/ZED_Xavier/bin/log/";
    char svofile_time[100];
    strftime(svofile_time,100,"%Y/%m-%d/%F_%H-%M-%S_video.svo",start_time);
	strcat(svofile, svofile_time);
	std::cout << "svo file: " << svofile << std::endl;

    sl::ERROR_CODE err;
    YAML::Node zed_config = config_file["zed"];
	err = zed.enableRecording(svofile, str_to_compression(zed_config["compression"].as<std::string>()));
	std::cout << "enabled svo recording" << std::endl;
	if (err != sl::SUCCESS) {
	    std::cout << sl::toString(err) << std::endl;
	    exit(-1);
	}

	//open data file
	char datafname[150];
	strftime(datafname,150,"/home/nvidia/Documents/image_acquisition/ZED_Xavier/bin/log/%Y/%m-%d/%F_%H-%M-%S_data.txt",start_time);
	data_file.open(datafname, std::ios::out);
	std::cout << "enabled custom data recording" << std::endl;
	data_file << "i\t" <<"sys_time_local\t"<< "sys_time_rel (s)\t" << "frame_timestamp (ns)\t" 
                              << "gps_time\t" << "lat\t" << "lat_err (m)\t"<< "lon\t" << "lon_err (m)\t" << "alt\t"<<"gps_mode\t" <<"gps_status\t" << "gps_nsats\t" 
                               << "qx\t"<< "qy\t"<< "qz\t" << "qw\t" << "acc_x\t" << "acc_y\t" <<"acc_z\t" << "angv_x\t" << "angv_y\t" <<"angv_z\n";


}

void Logger::record_start(std::chrono::high_resolution_clock::time_point  tref_)
{
    tref = tref_;
	b_record = true;

    rec_thread = std::thread(&Logger::record, this);   //lauch record thread

    //update 7seg display
    Seven_seg_data drec;
    drec.msg = "rec1";
    drec.duration = 750;
    disp.update_periodic("rec", drec);

}

void Logger::record_stop()
{
    b_record = false;
    rec_thread.join();

    zed.disableRecording();
	std::cout << "Video has been saved ..." << std::endl;

	data_file.close();

    //update 7seg display
   Seven_seg_data drec;
   drec.msg ="rec0";
   drec.duration = 750;
   disp.update_periodic("rec", drec);
}
 
void Logger::record()
{  
    int i = 0;
    Seven_seg_data dgps;
    dgps.duration = 750;

    while (b_record) {
	    // Grab a new frame
	    if (zed.grab() == sl::SUCCESS) {
                
		// calculate a bunch of times and timestamps for debugging and synchronizing
		// high resolution relative time
		std::chrono::high_resolution_clock::time_point  sys_tcur = std::chrono::high_resolution_clock::now();//system time
		std::chrono::duration<double> trel_sys = std::chrono::duration_cast<std::chrono::duration<double>>(sys_tcur - tref); //elapsed time

		// coarse system time (absolute, local)
		time(&rawtime);
		struct tm * now = localtime(&rawtime);
		char curtime[30];
		strftime(curtime,30,"%Y-%m-%d_%H:%M:%S",now);

		sl::timeStamp frame_timestamp = zed.getTimestamp(sl::TIME_REFERENCE_IMAGE);

		//retrieve IMU data
		sl::IMUData zed_imu;
	    zed.getIMUData(zed_imu, sl::TIME_REFERENCE::TIME_REFERENCE_IMAGE);
		sl::Orientation quat = zed_imu.getOrientation();
		sl::float3 imu_acc = zed_imu.linear_acceleration;
		sl::float3 imu_ang = zed_imu.angular_velocity;
		// Record the grabbed frame in the video file
		zed.record();

        //periodically update display
        if( i % 25 == 0){
            dgps.msg = "gps" + std::to_string(gps.get_mode());
            disp.update_periodic("gps", dgps);
         }

		//write auxilary data to file - would be better to convert this to xml or something similar
		data_file << i << "\t" << curtime << "\t" << trel_sys.count() << "\t" 
						<<  frame_timestamp << "\t" << gps.get_time() << "\t"
						<< std::fixed << std::setprecision(6) << gps.get_lat() << "\t"  << gps.get_lat_err() << "\t"
						<< std::fixed << std::setprecision(6) << gps.get_lon() << "\t"  << gps.get_lon_err() << "\t"
                        << std::fixed << std::setprecision(6) << gps.get_alt() << "\t"
                        << gps.get_mode() << "\t" <<gps.get_status() << "\t" << gps.get_nsats() << "\t"
						<< quat(0) << "\t" << quat(1) << "\t"<< quat(2) << "\t"<< quat(3) <<"\t"
						<< imu_acc[0]  << "\t" << imu_acc[1]  << "\t" << imu_acc[2]  << "\t" 
						<< imu_ang[0]  << "\t" << imu_ang[1]  << "\t"<< imu_ang[2]  << std::endl;
		i++;
		}// end frame grab success
    }//end while loop

}

