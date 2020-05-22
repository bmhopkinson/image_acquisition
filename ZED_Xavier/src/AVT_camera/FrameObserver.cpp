#include "FrameObserver.h"
#include "VimbaCPP/Include/VimbaCPP.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <future>

namespace AVT{
namespace VmbAPI{

FrameObserver::FrameObserver(CameraPtr pCamera): IFrameObserver(pCamera)
{
    pCamera_ = pCamera;
    time_ref = std::chrono::high_resolution_clock::now();// time reference point
}

void FrameObserver::FrameReceived(const FramePtr pFrame)
{
     // get time stamp right away
    std::chrono::high_resolution_clock::time_point  time_cur = std::chrono::high_resolution_clock::now();//system time
    std::chrono::duration<double> time_stamp = std::chrono::duration_cast<std::chrono::duration<double>>(time_cur - time_ref); //elapsed time used as time_stamp

      // process image
    VmbErrorType  err;
    VmbFrameStatusType status = VmbFrameStatusIncomplete;

    err = pFrame->GetReceiveStatus( status );
    if ( VmbErrorSuccess == err   && VmbFrameStatusComplete == status )
    {
        VmbUint32_t nWidth = 0;
        pFrame->GetWidth( nWidth );
        VmbUint32_t nHeight = 0;
        pFrame->GetHeight( nHeight );
        VmbUint32_t nImageSize = 0; 
        pFrame->GetImageSize( nImageSize );

        VmbUchar_t *pImage = NULL;
        err = pFrame->GetImage( pImage );
        if(err != VmbErrorSuccess) { std::cout << "could not get image data from frame" << std::endl;}

        ImageVmb img;
        img.buffer.resize(nImageSize);
        img.buffer.assign(pImage, pImage+nImageSize);
        img.height = nHeight;
        img.width = nWidth;
        img.ts = time_stamp.count();
        img.n_bytes = nImageSize;

        std::async(std::launch::async, &FrameObserver::write_img, this, img);  
 
        i++;

     }
     else { std::cout << "did not acquire frame" << std::endl;}

    pCamera_->QueueFrame(pFrame);
}

void FrameObserver::write_img(ImageVmb img){
       cv::Mat im( img.height, img.width, CV_8UC3, img.buffer.data() );  //note this does not copy the bitmap.buffer data
        cv:cvtColor(im, im, CV_BGR2RGB);
        std::string img_name = "AcqCont_opencv_" + std::to_string(i) + ".jpg";
        cv::imwrite(img_name, im);   

}

}}//end namespaces
