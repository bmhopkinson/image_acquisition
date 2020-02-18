#include "Adafruit_BNO055_port.h"
#include <iostream>
#include <unistd.h>
#include <fstream>

using namespace std;

int main(void){
  BNO055 imu = BNO055( BNO055_ADDRESS);
  int I2CBus = 1;
  imu.begin(I2CBus);
  
  uint8_t sys, gyro, accel, mag;
  uint8_t* psys    = &sys;
  uint8_t* pgyro   = &gyro;
  uint8_t* paccel  = &accel; 
  uint8_t* pmag    = &mag;

  
  ofstream imuFile;
  imuFile.open("/media/ubuntu/Orbitty_New_ext4/imudata2.txt");

 for(int j = 0; j<5; j++){

  imu.getCalibration( psys, pgyro, paccel, pmag);
  fprintf(stdout, "sys cal: %d\n", *psys);
  fprintf(stdout, "gyro cal: %d\n", *pgyro);
  fprintf(stdout, "accel cal: %d\n", *paccel);
  fprintf(stdout, "mag cal: %d\n", *pmag);

  int8_t T;
  T = imu.getTemp();
  fprintf(stdout, "Temp (C): %d\n", T);

   for(int i = 0; i < 100 ; i++){
    imu::Quaternion Q;
    Q = imu.getQuat();
    double w, x, y, z;
    w = Q.w(); x = Q.x(), y = Q.y(); z = Q.z();
    cout << "Quaternion: w: " << w << " x: "<< x << " y: " << y << " z: " << z << endl;
    imu::Vector<3> xyz;
    xyz = imu.getEuler();     
    cout << "Euler: x: " << xyz[0] << " y: "<< xyz[1] <<" z: " << xyz[2] << endl;
    imuFile << xyz[0] << " "<< xyz[1] << " " << xyz[2] << endl;
    usleep(100*1000);

   } //end inner for loop
 } //end outer for loop
   
  imuFile.close();

  return 0;     
  
  
}
