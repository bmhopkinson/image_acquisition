#ifndef LOGGER_H_
#define LOGGER_H_

#include <time.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <sl/Camera.hpp>
#include <yaml-cpp/yaml.h>
#include "system/GPSPoller.h"
#include "system/Arduino.h"
#include "system/Seven_seg.h"

class Logger{
    public:
        Logger(sl::Camera &zed_, YAML::Node config_file_);
        void initialize_recording(struct tm * start_time);
        void record_start(std::chrono::high_resolution_clock::time_point  tref_);
        void record_stop();
        
    private:
        //major components
        sl::Camera& zed;
        GPSPoller gps;
        Arduino ard;
        Seven_seg disp;
        YAML::Node config_file;

        //flags
        bool b_record = false;

        //data containers, files
        std::ofstream data_file;
        std::chrono::high_resolution_clock::time_point  tref;
        time_t rawtime;

        //threads and thread functions
        void record(); 
        std::thread rec_thread;
};

#endif //include guard
