#include "AVTCamera.h"
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "VimbaCPP/Include/VimbaCPP.h"
#include "FrameObserver.h"


namespace AVT{
namespace VmbAPI{
AVTCamera::AVTCamera() : sys(AVT::VmbAPI::VimbaSystem::GetInstance())
{}

AVTCamera::~AVTCamera()
{
     pCamera->Close();
     sys.Shutdown();   // Release Vimba
}

int AVTCamera::initialize_camera()
{   
   VmbErrorType  err = VmbErrorSuccess;
    //start system
    err = sys.Startup();               // Initialize the Vimba API
    if ( err != VmbErrorSuccess) { std::cout << "could not start system" << std::endl; return -1;}

    //identify cameras and open camera[0] - should only be one
    AVT::VmbAPI::CameraPtrVector cameras;                           // A vector of std::shared_ptr<AVT::VmbAPI::Camera> objects
    err = sys.GetCameras( cameras );            // Fetch all cameras known to Vimba
    if ( err != VmbErrorSuccess) { std::cout << "could not get camera list" << std::endl; return -1;}

    std::string strCameraID;
    if(cameras.empty()){ 
         std::cout << "could not cameras detected" << std::endl; return -1;
    }
    else{
        pCamera = cameras[0];  //should only be one camera right now
    }

    err = pCamera->Open(VmbAccessModeFull);
    if ( err != VmbErrorSuccess) { std::cout << "could not open camera[0]" << std::endl; return -1;}
  
   return 0;
}

int AVTCamera::set_camera_features(YAML::Node settings){
   //set camera settings ("features")
    VmbErrorType  err;
    AVT::VmbAPI::FeaturePtr pFormatFeature;

    if(settings["Acquisition"]){
        set_acquisition(settings["Acquisition"]);
    }
    if(settings["Balance"]){
        set_balance(settings["Balance"]);
    }
    if(settings["Exposure"]){
        set_exposure(settings["Exposure"]);
    }
    if(settings["Gain"]){
        set_gain(settings["Gain"]);
    }
    if(settings["Trigger"]){
        set_trigger(settings["Trigger"]);
    }

    AVT::VmbAPI::FeaturePtr pCommandFeature;
    if ( VmbErrorSuccess == pCamera->GetFeatureByName( "GVSPAdjustPacketSize", pCommandFeature ))
    {
            if ( VmbErrorSuccess == pCommandFeature->RunCommand() )
            {
                bool bIsCommandDone = false;
                do
                {
                    if ( VmbErrorSuccess != pCommandFeature->IsCommandDone( bIsCommandDone ))
                    {
                        break;
                    }
                } while ( false == bIsCommandDone );
            }
        }

     // Set pixel format. For the sake of simplicity we only support Mono and BGR in this example.
        err = pCamera->GetFeatureByName( "PixelFormat", pFormatFeature );
        if ( VmbErrorSuccess == err )
        {
            // Try to set BGR
            err = pFormatFeature->SetValue( VmbPixelFormatRgb8 );
            if ( VmbErrorSuccess != err )
            {
                //
               std::cout << "could not set RGB8 mode" << std::endl;
            }
        }

  
     return 0;
}

int AVTCamera::start_acquisition(){
   IFrameObserverPtr pFrameObserver( new FrameObserver(pCamera));
   pCamera->StartContinuousImageAcquisition(6, pFrameObserver);
   return 0;
}

int AVTCamera::stop_acquisition(){
   pCamera->StopContinuousImageAcquisition();
   return 0;
}

//set feature functions
void AVTCamera::set_acquisition(YAML::Node settings){
    AVT::VmbAPI::FeaturePtr pFormatFeature;
    VmbErrorType  err;
    if(settings["AcquisitionFrameRateAbs"]){
      //set frames per second
       err = pCamera->GetFeatureByName( "AcquisitionFrameRateAbs", pFormatFeature );
       if ( VmbErrorSuccess == err )
        {
            // Try to set 
            err = pFormatFeature->SetValue( settings["AcquisitionFrameRateAbs"].as<float>() );
            if ( VmbErrorSuccess != err )
            {
                //
               std::cout << "could not set AcquisitionFrameRateAbs" << std::endl;
            }
        }
    }

}

void AVTCamera::set_balance(YAML::Node settings){
   AVT::VmbAPI::FeaturePtr pFormatFeature;
   VmbErrorType  err;

   if(settings[ "BalanceWhiteAuto"]){
       err = pCamera->GetFeatureByName( "BalanceWhiteAuto", pFormatFeature );
          if ( VmbErrorSuccess == err )
         { 
              err = pFormatFeature->SetValue( settings["BalanceWhiteAuto"].as<std::string>().c_str()  );
              if ( VmbErrorSuccess != err ){
                 std::cout << "could not set BalanceWhiteAuto" << std::endl;
             } 
         }
         else { std::cout << "could not access BalanceWhiteAuto" << std::endl;}
    }

   if(settings[ "BalanceWhiteAutoRate"]){
       err = pCamera->GetFeatureByName( "BalanceWhiteAutoRate", pFormatFeature );
          if ( VmbErrorSuccess == err )
         { 
              err = pFormatFeature->SetValue( settings["BalanceWhiteAutoRate"].as<int>()  );
              if ( VmbErrorSuccess != err ){
                 std::cout << "could not set BalanceWhiteAutoRate" << std::endl;
             } 
         }
         else { std::cout << "could not access BalanceWhiteAutoRate" << std::endl;}
    }



}

void AVTCamera::set_exposure(YAML::Node settings){
   AVT::VmbAPI::FeaturePtr pFormatFeature;
   VmbErrorType  err;

   if(settings["ExposureAuto"]){
        //set ExposureAuto
        err = pCamera->GetFeatureByName( "ExposureAuto", pFormatFeature );
          if ( VmbErrorSuccess == err )
         { 
              err = pFormatFeature->SetValue( settings["ExposureAuto"].as<std::string>().c_str() );
              if ( VmbErrorSuccess != err ){
                 std::cout << "could not set ExposureAuto" << std::endl;
             }
         }
         else { std::cout << "could not access ExposureAuto" << std::endl;}
     }

  if(settings["ExposureAutoAlg"]){
         err = pCamera->GetFeatureByName( "ExposureAutoAlg", pFormatFeature );
          if ( VmbErrorSuccess == err )
         { 
              err = pFormatFeature->SetValue( settings["ExposureAutoAlg"].as<std::string>().c_str()  );
              if ( VmbErrorSuccess != err ){
                 std::cout << "could not set ExposureAutoAlg" << std::endl;
             }
         }
         else { std::cout << "could not access ExposureAutoAlg" << std::endl;}
    }


  if(settings["ExposureAutoMax"]){
         err = pCamera->GetFeatureByName( "ExposureAutoMax", pFormatFeature );
          if ( VmbErrorSuccess == err )
         { 
              err = pFormatFeature->SetValue( settings["ExposureAutoMax"].as<int>() );
              if ( VmbErrorSuccess != err ){
                 std::cout << "could not set ExposureAuto" << std::endl;
             }
         }
         else { std::cout << "could not access ExposureAuto" << std::endl;}
    }

  if(settings["ExposureAutoRate"]){
        //set ExposureAutoMax (10ms)
         err = pCamera->GetFeatureByName( "ExposureAutoRate", pFormatFeature );
          if ( VmbErrorSuccess == err )
         { 
              err = pFormatFeature->SetValue( settings["ExposureAutoRate"].as<int>() );
              if ( VmbErrorSuccess != err ){
                 std::cout << "could not set ExposureAutoRate" << std::endl;
             }
         }
         else { std::cout << "could not access ExposureAutoRate" << std::endl;}
    }

  if(settings["ExposureAutoTarget"]){
        //set ExposureAutoMax (10ms)
         err = pCamera->GetFeatureByName( "ExposureAutoTarget", pFormatFeature );
          if ( VmbErrorSuccess == err )
         { 
              err = pFormatFeature->SetValue( settings["ExposureAutoTarget"].as<int>() );
              if ( VmbErrorSuccess != err ){
                 std::cout << "could not set ExposureAutoTarget" << std::endl;
             }
         }
         else { std::cout << "could not access ExposureAutoTarget" << std::endl;}
    }
}

void AVTCamera::set_gain(YAML::Node settings){
    AVT::VmbAPI::FeaturePtr pFormatFeature;
    VmbErrorType  err;
    if(settings["GainAuto"]){
        err = pCamera->GetFeatureByName( "GainAuto", pFormatFeature );
        if ( VmbErrorSuccess == err )
        { 
            err = pFormatFeature->SetValue(  settings["GainAuto"].as<std::string>().c_str() );
            if ( VmbErrorSuccess != err ){
                std::cout << "could not set GainAuto" << std::endl;
            } 
        }
        else { std::cout << "could not access GainAuto" << std::endl;}
    }

    if(settings["GainAutoRate"]){
        err = pCamera->GetFeatureByName( "GainAutoRate", pFormatFeature );
        if ( VmbErrorSuccess == err )
        { 
            err = pFormatFeature->SetValue(  settings["GainAutoRate"].as<int>() );
            if ( VmbErrorSuccess != err ){
                std::cout << "could not set GainAutoRate" << std::endl;
            } 
        }
        else { std::cout << "could not access GainAutoRate" << std::endl;}
    }

    if(settings["GainAutoTarget"]){
        err = pCamera->GetFeatureByName( "GainAutoTarget", pFormatFeature );
        if ( VmbErrorSuccess == err )
        { 
            err = pFormatFeature->SetValue(  settings["GainAutoTarget"].as<int>() );
            if ( VmbErrorSuccess != err ){
                std::cout << "could not set GainAutoTarget" << std::endl;
            } 
        }
        else { std::cout << "could not access GainAutoTarget" << std::endl;}
    }



}

void AVTCamera::set_trigger(YAML::Node settings){
    AVT::VmbAPI::FeaturePtr pFormatFeature;
    VmbErrorType  err;
    //set trigger  mode - all this is so we can specify fps
    if(settings["TriggerMode"]){
        err = pCamera->GetFeatureByName( "TriggerMode", pFormatFeature );
        if ( VmbErrorSuccess == err )
        {
            // Try to set OFF
            err = pFormatFeature->SetValue( settings["TriggerMode"].as<std::string>().c_str() );
            if ( VmbErrorSuccess != err )
           {
                std::cout << "could not set trigger mode " << std::endl;
            }
        }
    }

    //set trigger  selector
    if(settings["TriggerSelector"]){
        err = pCamera->GetFeatureByName( "TriggerSelector", pFormatFeature );
        if ( VmbErrorSuccess == err )
        {
            err = pFormatFeature->SetValue( settings["TriggerSelector"].as<std::string>().c_str() );
            if ( VmbErrorSuccess != err )
            {
                std::cout << "could not set trigger selector" << std::endl;
            }
        }
    }

    //set trigger source
    if(settings["TriggerSource"]){
        err = pCamera->GetFeatureByName( "TriggerSource", pFormatFeature );
        if ( VmbErrorSuccess == err )
        {
            err = pFormatFeature->SetValue( settings["TriggerSource"].as<std::string>().c_str() );
            if ( VmbErrorSuccess != err )
            {
                std::cout << "could not set trigger source" << std::endl;
            }
          }
    }
}

}}
