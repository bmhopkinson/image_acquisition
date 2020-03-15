
#include "system/PerfMonitor.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <cstdlib>
#include <string.h>

//PerfMonitor::PerfMonitor(std::string fname){
PerfMonitor::PerfMonitor(char* logdir){
	perfThread = std::thread(&PerfMonitor::logperf, this, logdir);
}

PerfMonitor::~PerfMonitor(){
    log_data = false;
	perfThread.join();
}

//void PerfMonitor::logperf(std::string fname){
void PerfMonitor::logperf(char* logdir){
	//create output filename based on time
	time_t rawtime;
	time(&rawtime);
	struct tm * now = localtime(&rawtime);
    char filestr[30];
	strftime(filestr,30,"/tegrastats_%H-%M-%S.txt",now);

	char pathstr[80];
	strcpy(pathstr, logdir);
	strcat(pathstr, filestr);

	char syscallstr[80];
	strcpy(syscallstr, "tegrastats --interval 2000  --logfile ");
	strcat(syscallstr, pathstr);
	strcat(syscallstr," &");
	std::cout << syscallstr << std::endl;
	std::system(syscallstr);

	std::ofstream log_stream;
	log_stream.open("test_perfmon.txt", std::ios::out);
	
	std::string t2;
	std::ifstream t2_sysfs("/sys/class/thermal/thermal_zone2/temp");
	std::string t4;
	std::ifstream t4_sysfs("/sys/class/thermal/thermal_zone4/temp");
	std::string t6;
	std::ifstream t6_sysfs("/sys/class/thermal/thermal_zone6/temp");


	int i = 0;
	while(log_data){
		//time 
		time_t rawtime;
		time(&rawtime);
		struct tm * now = localtime(&rawtime);
    	char timestr[30];
		strftime(timestr,30,"%Y-%m-%d_%H:%M:%S",now);
		
		// temp data
		t2_sysfs >> t2;
		t4_sysfs >> t4;
		t6_sysfs >> t6;

		//write out data
		log_stream << i <<  "\t" <<  timestr <<  "\t" << t2 <<"\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));	
		i++;
	}
	std::system("tegrastats --stop");
	log_stream.close();

}
