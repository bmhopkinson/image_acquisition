
#include "system/PerfMonitor.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

char perf_interval[] = "1000"; 
using namespace std;
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

	char pathstr[200];
	strcpy(pathstr, logdir);
	strcat(pathstr, filestr);
	
	//start tegrastats
	char syscallstr[200];
	strcpy(syscallstr, "tegrastats --interval ");
	strcat(syscallstr, perf_interval);
	strcat(syscallstr, "  --logfile ");
	strcat(syscallstr, pathstr);
	strcat(syscallstr," &");
	std::cout << syscallstr << std::endl;
	std::system(syscallstr);
	std::this_thread::sleep_for(std::chrono::milliseconds(atoi(perf_interval)));	//allow tegrastats to get started

   //open tegrastats logfile to append time info - this mostly seems to work - for some reason the time might ocassionaly overwrite the tegrastats data but some tests indicated the dual writing to a single file approach isn't terribly wront
   int fid_tg = open(pathstr, O_WRONLY);
	if(fid_tg == -1){
		std::cout << "error opening tegrastats file: " << pathstr << std::endl;
	}


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
		strftime(timestr,30,"%Y-%m-%d_%H:%M:%S\n",now);
		
		// temp data
		t2_sysfs >> t2;
		t4_sysfs >> t4;
		t6_sysfs >> t6;

		//write out data to custom logfile
		log_stream << i <<  "\t" <<  timestr <<  "\t" << t2 <<"\n";

		//write time to tegrastats file
		lseek(fid_tg, 0, SEEK_END); // go to end of file
		size_t wr_bytes = write(fid_tg, timestr, strlen(timestr));  //write time

		//pause
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));	
		i++;
	}
	std::system("tegrastats --stop");
	log_stream.close();
	close(fid_tg);

}
