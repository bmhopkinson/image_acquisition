#ifndef AVTCAM_TEST_H_
#define AVTCAM_TEST_H_

#include "VimbaCPP/Include/VimbaCPP.h"
#include <yaml-cpp/yaml.h>

namespace AVT{
namespace VmbAPI{
class AVTCamera{
    public:
        AVTCamera();
       ~AVTCamera();
        int initialize_camera();
        int set_camera_features(YAML::Node settings);
        int start_acquisition();
        int stop_acquisition();

    private: 
        VimbaSystem&    sys;
        CameraPtr pCamera;

        //helper functions
        void set_acquisition(YAML::Node);
        void set_balance(YAML::Node);
        void set_exposure(YAML::Node);
        void set_gain(YAML::Node);
        void set_trigger(YAML::Node);


};
}}// AVT, VmbAPI namespace end

#endif // include guard
