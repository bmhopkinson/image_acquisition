#include <sl/Camera.hpp>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char **argv) {
    sl::Camera zed;
    sl::InitParameters initParameters;
    initParameters.camera_fps = 15;
    initParameters.camera_resolution = sl::RESOLUTION_HD720;
    initParameters.depth_mode = sl::DEPTH_MODE_NONE;

    string svo_path = "/media/ubuntu/Orbitty_USB/record_trial_output1.svo"; // need to make sure system has permission to write and create on media directory

 // Open the ZED
    sl::ERROR_CODE err = zed.open(initParameters);
    if (err != sl::SUCCESS) {
        cout << sl::errorCode2str(err) << endl;
        zed.close();
        return 1; // Quit if an error occurred
    }

   cout << "opened the ZED" << endl;

  //SETUP TO RECORD
   err = zed.enableRecording(svo_path.c_str(), sl::SVO_COMPRESSION_MODE::SVO_COMPRESSION_MODE_LOSSY);
   if (err != sl::SUCCESS) {
         std::cout << "Error while recording. " << errorCode2str(err) << " " << err << std::endl;
         if (err == sl::ERROR_CODE_SVO_RECORDING_ERROR) std::cout << " Note : This error mostly comes from a wrong path or missing writting permissions..." << std::endl;
         zed.close();
         return 1;
    }


  //MAIN RECORDING LOOP
    int numFrames = 3000;
    sl::Mat view;
    int i=0;
    while(i<numFrames){
        if (!zed.grab()) {

        // Get the side by side image
        zed.retrieveImage(view, sl::VIEW_SIDE_BY_SIDE);
        zed.record(); // record
        i++;
        cout << i << endl;
       } else sl::sleep_ms(1);
    }  //end main recording loop

   zed.disableRecording(); 
   zed.close();


}
