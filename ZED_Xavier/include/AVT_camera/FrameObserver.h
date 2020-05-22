#ifndef FRAME_OBSERVER_H_
#define FRAME_OBSERVER_H_

#include "VimbaCPP/Include/VimbaCPP.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <chrono>

namespace AVT{
namespace VmbAPI{

struct ImageVmb{
   public:
       uint32_t height;
       uint32_t width;
       uint32_t n_bytes;
       double ts; //time_stamp
       //uint8_t buffer[];  //must be last member of struct
      std::vector<uint8_t> buffer;
};

class FrameObserver : public IFrameObserver {
    public:
        FrameObserver(CameraPtr pCamera); //constructor
        void FrameReceived(const FramePtr pFrame); // handle received frames

    private:
        CameraPtr pCamera_;
        int i =0; //number of frames taken by camera
         std::chrono::high_resolution_clock::time_point time_ref; //reference time - currently set in constructor but in ZED logger would be synchronized with ref_time of other instruments

        void write_img(ImageVmb img);  // write image function to call asynchronously

};
}}//end namespaces
#endif  //include guard

