#include <iostream>
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

int main(int argc, char *argv[]){
  sl::zed::Camera *zed = new sl::zed::Camera(sl::zed::HD720);
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
  
  sleep(5);
  SaveParam *svPar;
  svPar = new SaveParam();
  svPar->Depth_Format = sl::DEPTH_FORMAT::PNG;
  svPar->saveName = "test_02282017";
  svPar->PC_Format= sl::POINT_CLOUD_FORMAT::PLY;

  zed->grab(sl::zed::SENSING_MODE::STANDARD,1,1);
  std::cout << "Saving Depth Map and Point Cloud " << svPar->saveName <<  " ..." << flush;
  sl::writeDepthAs(zed, svPar->Depth_Format,svPar->saveName, scale_factor);
  sl::writePointCloudAs(zed, svPar->PC_Format, svPar->saveName, true, false);
  std::cout << "done" << endl;
  delete zed;
}
