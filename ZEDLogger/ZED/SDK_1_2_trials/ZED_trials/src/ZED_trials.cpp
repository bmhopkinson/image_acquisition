#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>
#include <zed/Camera.hpp>

using namespace std;

typedef struct SaveParamStruct {
    sl::POINT_CLOUD_FORMAT PC_Format;
    sl::DEPTH_FORMAT Depth_Format;
    std::string saveName;
    bool askSavePC;
    bool askSaveDepth;
    bool stop_signal;
} SaveParam;

int ConfidenceThreshold = 80; // lower value is higher confidence

int main(int argc, char *argv[]){
  sl::zed::Camera *zed = new sl::zed::Camera(sl::zed::VGA, 10.0);
  sl::zed::InitParams parameters;
  parameters.mode =  sl::zed::MODE::MEDIUM;
  parameters.unit = sl::zed::UNIT::MILLIMETER;
  parameters.verbose = 1;
  zed->init(parameters);
  std::cout  << "Hello World! ZED is running" << std::endl;
      
  int depth_clamp = 5000;
  zed->setDepthClampValue(depth_clamp);
  float max_value = std::numeric_limits<unsigned short int>::max();
  float scale_factor = max_value / zed->getDepthClampValue();

//start recording STILL WORKING ON RECORDING
  std::string videoFile = "test_video.svo";
  zed->enableRecording(videoFile);
  std::chrono::duration<double> rcdDur = std::chrono::seconds(20); //seconds to record
  chrono::time_point<std::chrono::system_clock> start, now, previous;
  start = chrono::system_clock::now();

  bool rcdStatus = true;
  while(rcdStatus)
  {
    zed->grab(sl::zed::SENSING_MODE::STANDARD,1,1);
    zed->record();
    now = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = now - start;
    if(elapsed_seconds > rcdDur) {rcdStatus = false;}
    this_thread::sleep_for(chrono::milliseconds(200));
  }
  zed->stopRecording();

/*  
  sleep(5);
  SaveParam *svPar;
  svPar = new SaveParam();
  svPar->Depth_Format = sl::DEPTH_FORMAT::PNG;
  svPar->saveName = "test_02282017";
  svPar->PC_Format= sl::POINT_CLOUD_FORMAT::PLY;

  zed->setConfidenceThreshold(ConfidenceThreshold);
  zed->grab(sl::zed::SENSING_MODE::STANDARD,1,1);
  std::cout << "Saving Depth Map and Point Cloud " << svPar->saveName <<  " ..." << flush;
  sl::writeDepthAs(zed, svPar->Depth_Format,svPar->saveName, scale_factor);
  sl::writePointCloudAs(zed, svPar->PC_Format, svPar->saveName, true, false);
  std::cout << "done" << endl;
*/



  delete zed;
}
