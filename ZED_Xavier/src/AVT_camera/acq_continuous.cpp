#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "VimbaCPP/Include/VimbaCPP.h"
#include "FrameObserver.h"

using namespace AVT::VmbAPI;
int main( int argc, char* argv[] )
{
   int dur_sec = atoi(argv[1]); //duration of image acquisition;
   float fps = atof(argv[2]); // frames per second
   
   VmbErrorType  err = VmbErrorSuccess;
    //start system
    AVT::VmbAPI::VimbaSystem&    sys = AVT::VmbAPI::VimbaSystem::GetInstance();  // Get a reference to the VimbaSystem singleton
    err = sys.Startup();               // Initialize the Vimba API
    if ( err != VmbErrorSuccess) { std::cout << "could not start system" << std::endl; return -1;}

    //identify cameras and open camera[0] - should only be one
    AVT::VmbAPI::CameraPtrVector cameras;                           // A vector of std::shared_ptr<AVT::VmbAPI::Camera> objects
    err = sys.GetCameras( cameras );            // Fetch all cameras known to Vimba
    if ( err != VmbErrorSuccess) { std::cout << "could not get camera list" << std::endl; return -1;}

    std::string strCameraID;
    AVT::VmbAPI::CameraPtr m_pCamera;
    if(cameras.empty()){ 
         std::cout << "could not cameras detected" << std::endl; return -1;
    }
    else{
        m_pCamera = cameras[0];
    }

    err = m_pCamera->Open(VmbAccessModeFull);
    if ( err != VmbErrorSuccess) { std::cout << "could not open camera[0]" << std::endl; return -1;}

   //set camera settings ("features")
    AVT::VmbAPI::FeaturePtr pCommandFeature;
    if ( VmbErrorSuccess == m_pCamera->GetFeatureByName( "GVSPAdjustPacketSize", pCommandFeature ))
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

        AVT::VmbAPI::FeaturePtr pFormatFeature;
        //set ExposureAuto
        err = m_pCamera->GetFeatureByName( "ExposureAuto", pFormatFeature );
          if ( VmbErrorSuccess == err )
         { 
              err = pFormatFeature->SetValue( "Continuous" );
              if ( VmbErrorSuccess != err ){
                 std::cout << "could not set ExposureAuto" << std::endl;
             }
         }
         else { std::cout << "could not access ExposureAuto" << std::endl;}

        //set ExposureAutoMax (500ms)
         err = m_pCamera->GetFeatureByName( "ExposureAutoMax", pFormatFeature );
          if ( VmbErrorSuccess == err )
         { 
              err = pFormatFeature->SetValue( 50000 );
              if ( VmbErrorSuccess != err ){
                 std::cout << "could not set ExposureAuto" << std::endl;
             }
         }
         else { std::cout << "could not access ExposureAuto" << std::endl;}

        //set auto-gain

         err = m_pCamera->GetFeatureByName( "BalanceWhiteAuto", pFormatFeature );
          if ( VmbErrorSuccess == err )
         { 
              err = pFormatFeature->SetValue( "Continuous" );
              if ( VmbErrorSuccess != err ){
                 std::cout << "could not set BalanceWhiteAuto" << std::endl;
             } 
         }
         else { std::cout << "could not access BalanceWhiteAuto" << std::endl;}


        //set auto-white balance
         err = m_pCamera->GetFeatureByName( "GainAuto", pFormatFeature );
          if ( VmbErrorSuccess == err )
         { 
              err = pFormatFeature->SetValue( "Continuous" );
              if ( VmbErrorSuccess != err ){
                 std::cout << "could not set GainAuto" << std::endl;
             } 
         }
         else { std::cout << "could not access GainAuto" << std::endl;}

     // Set pixel format. For the sake of simplicity we only support Mono and BGR in this example.
        err = m_pCamera->GetFeatureByName( "PixelFormat", pFormatFeature );
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

       //set trigger  mode - all this is so we can specify fps
       err = m_pCamera->GetFeatureByName( "TriggerMode", pFormatFeature );
       if ( VmbErrorSuccess == err )
        {
            // Try to set OFF
            err = pFormatFeature->SetValue( "Off" );
            if ( VmbErrorSuccess != err )
            {
                //
               std::cout << "could not set trigger mode: Off " << std::endl;
            }
        }

      //set trigger  selector
       err = m_pCamera->GetFeatureByName( "TriggerSelector", pFormatFeature );
       if ( VmbErrorSuccess == err )
        {
            // Try to set OFF
            err = pFormatFeature->SetValue( "FrameStart" );
            if ( VmbErrorSuccess != err )
            {
                //
               std::cout << "could not set trigger selector: FrameStart" << std::endl;
            }
        }

      //set trigger source
       err = m_pCamera->GetFeatureByName( "TriggerSource", pFormatFeature );
       if ( VmbErrorSuccess == err )
        {
            // Try to set OFF
            err = pFormatFeature->SetValue( "FixedRate" );
            if ( VmbErrorSuccess != err )
            {
                //
               std::cout << "could not set trigger source: FixedRate" << std::endl;
            }
        }

      //set frames per second
       err = m_pCamera->GetFeatureByName( "AcquisitionFrameRateAbs", pFormatFeature );
       if ( VmbErrorSuccess == err )
        {
            // Try to set OFF
            err = pFormatFeature->SetValue( fps );
            if ( VmbErrorSuccess != err )
            {
                //
               std::cout << "could not set AcquisitionFrameRateAbs" << std::endl;
            }
        }

       //do actual image acquisition
       IFrameObserverPtr pFrameObserver( new FrameObserver(m_pCamera));
       m_pCamera->StartContinuousImageAcquisition(6, pFrameObserver);
       std::this_thread::sleep_for(std::chrono::seconds(dur_sec));
       m_pCamera->StopContinuousImageAcquisition();
        

     m_pCamera->Close();
     sys.Shutdown();   // Release Vimba
}
