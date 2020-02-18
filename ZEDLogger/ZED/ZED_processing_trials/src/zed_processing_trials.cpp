#include <sl/Camera.hpp>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char **argv) {
    sl::String svo_path = "/mnt/usb-Kingston_DataTraveler_3.0_B8AEEDBAF20EB140299855A3-0:0-part1/record_trial_output3.svo"; // need to make sure system has permission to write and create on media directory
    sl::Camera zed;
    sl::InitParameters initParameters;
    initParameters.svo_input_filename = svo_path;


 // Open the ZED
    sl::ERROR_CODE err = zed.open(initParameters);
    if (err != sl::SUCCESS) {
        cout << sl::errorCode2str(err) << endl;
        zed.close();
        return 1; // Quit if an error occurred
    }

   cout << "opened the ZED" << endl;

   int nFrames = zed.getSVONumberOfFrames();
   for(int i = 0; i<nFrames; i++){
     zed.grab();
     unsigned long long ts = zed.getCameraTimestamp();  // time in ns since computer start.
     cout << "frame num: " << i <<" timestamp: " << ts << endl;


   }

   zed.close();


}
