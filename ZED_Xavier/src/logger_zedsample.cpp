#include <sl/Camera.hpp>
using namespace sl;
int main(int argc, char **argv) {
    // Create a ZED camera object
    Camera zed;
    // Set initial parameters
    InitParameters init_params;
    init_params.camera_resolution = RESOLUTION_HD720; // Use HD720 video mode (default fps: 60)
    init_params.coordinate_units = UNIT_METER; // Set units in meters
    // Open the camera
    ERROR_CODE err = zed.open(init_params);
    if (err != SUCCESS) {
        std::cout << toString(err) << std::endl;
        exit(-1);
    }
    // Enable video recording
    err = zed.enableRecording("myVideoFile.svo", SVO_COMPRESSION_MODE_LOSSLESS);
    if (err != SUCCESS) {
        std::cout << toString(err) << std::endl;
        exit(-1);
    }
    // Grab data during 500 frames
    int i = 0;
    while (i < 500) {
        // Grab a new frame
        if (zed.grab() == SUCCESS) {
            // Record the grabbed frame in the video file
            zed.record();
            i++;
        }
    }
    zed.disableRecording();
    std::cout << "Video has been saved ..." << std::endl;
    zed.close();
    return 0;
}
