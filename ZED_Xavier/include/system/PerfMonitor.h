#ifndef PERFMONITOR_H_
#define PERFMONITOR_H_

#include <thread>
#include <string>


class PerfMonitor 
{
	private: 
	  //std::ofstream log_stream;
      std::thread perfThread;
	  void logperf(char* logdir);
	  //void logperf(std::string fname);
	  bool log_data = true;  //start true turn off at exit

	public: 
	  //PerfMonitor(std::string fname); //constructor
	  PerfMonitor(char* logdir); //constructor
    ~PerfMonitor(); //destructor - join worker thread, close log file

};

#endif


