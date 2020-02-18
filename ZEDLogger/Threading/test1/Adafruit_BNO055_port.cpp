#include "Adafruit_BNO055_port.h"
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#define MAX_WRITE_LEN 50

/***************************************************************************
 CONSTRUCTOR
 ***************************************************************************/
BNO055::BNO055(uint8_t address)
{
  _address = address;
}

/***************************************************************************
 DECONSTRUCTOR
 ***************************************************************************/
BNO055::~BNO055()
{
   close(_file);
}


// begin connects to BNO055 over I2C
/**************************************************************************/
/*!
    @brief  Sets up the HW
*/
/**************************************************************************/
bool BNO055::begin(int I2CBusID)
{
 char buf[32];
 sprintf(buf, "/dev/i2c-%d", I2CBusID);
 _file = open(buf, O_RDWR);
 
 if(_file <0){
    fprintf(stdout, "Failed to open I2C bus %d\n", I2CBusID);
    return false;
 }
 
 if(ioctl(_file, I2C_SLAVE, _address) <0){
    fprintf(stdout, "Failed to connect to the sensor\n");
    return -1;
  }
  delayMs(50);

  /* Reset */
  if(!write8(BNO055_SYS_TRIGGER_ADDR, 0x20))
    {fprintf(stdout, "Failed to reset BNO055\n");}
  while (read8(BNO055_CHIP_ID_ADDR) != BNO055_ID)
  {
    delayMs(10);
  }
  delayMs(50);



  /* Make sure we have the right device */
  uint8_t id = read8(BNO055_CHIP_ID_ADDR);
  if(id != BNO055_ID)
  {
    sleep(1); // hold on for boot
    id = read8(BNO055_CHIP_ID_ADDR);
    if(id != BNO055_ID) {
      return false;  // still not? ok bail
    } else {fprintf(stdout,"BNO055 confirmed its identity\n");}
  } else {fprintf(stdout,"BNO055 confirmed its identity\n");}

  setMode(OPERATION_MODE_CONFIG);
  delayMs(50);

  /* Reset */
  if(!write8(BNO055_SYS_TRIGGER_ADDR, 0x20))
    {fprintf(stdout, "Failed to reset BNO055\n");}
  while (read8(BNO055_CHIP_ID_ADDR) != BNO055_ID)
  {
    delayMs(10);
  }
  delayMs(50);

  /* Set to normal power mode */
  setMode(OPERATION_MODE_CONFIG);
  delayMs(20);

  //setExtCrystalUse(true);//use external clock

  write8(BNO055_PAGE_ID_ADDR, 0);


  if(!write8(BNO055_SYS_TRIGGER_ADDR, 0x00))
    {fprintf(stdout, "Failed to start BNO055\n");}
  delayMs(50);

  setMode(OPERATION_MODE_NDOF);  // set operating mode - 9 DOF Fusion mode
  delayMs(20);
 
} // end begin function

/***************************************************************************
UTILITY FUNCTIONS
 ***************************************************************************/

void BNO055::setMode(adafruit_bno055_opmode_t mode)
{
  _mode = mode;
  if(!write8(BNO055_OPR_MODE_ADDR, _mode))
  { fprintf(stdout,"could not write new mode\n");}

  delayMs(30);
}

/**************************************************************************/
/*!
    @brief  Use the external 32.768KHz crystal
*/
/**************************************************************************/
void BNO055::setExtCrystalUse(bool usextal) //TRIED THIS BUT IT CAUSED PROBLEMS - after calling no data came out
{
  adafruit_bno055_opmode_t modeback = _mode;

  /* Switch to config mode */
  setMode(OPERATION_MODE_CONFIG);
  delayMs(25);
  write8(BNO055_PAGE_ID_ADDR, 0);
  if (usextal) {
    write8(BNO055_SYS_TRIGGER_ADDR, 0x80);
  } else {
    write8(BNO055_SYS_TRIGGER_ADDR, 0x00);
  }
  delayMs(10);
  /* Set the requested operating mode (see section 3.3) */
  setMode(modeback);
  delayMs(20);
}


/***************************************************************************
     DATA ACQUISITION FUNCTIONS
 ***************************************************************************/
/**************************************************************************/
/*!
    @brief  Gets the temperature in degrees celsius
*/
/**************************************************************************/
int8_t BNO055::getTemp(void)
{
  int8_t temp = (int8_t)(read8(BNO055_TEMP_ADDR));
  return temp;
}

/**************************************************************************/
/*!
    @brief  Gets a quaternion reading from the specified source
*/
/**************************************************************************/
imu::Quaternion BNO055::getQuat(void)
{ 
  unsigned char* buffer;
  int16_t x, y, z, w;
  x = y = z = w = 0;

  /* Read quat data (8 bytes) */
  buffer = readLen(BNO055_QUATERNION_DATA_W_LSB_ADDR, 8);
  w = (((uint16_t)buffer[1]) << 8) | ((uint16_t)buffer[0]);
  x = (((uint16_t)buffer[3]) << 8) | ((uint16_t)buffer[2]);
  y = (((uint16_t)buffer[5]) << 8) | ((uint16_t)buffer[4]);
  z = (((uint16_t)buffer[7]) << 8) | ((uint16_t)buffer[6]);

  /* Assign to Quaternion */
  /* See http://ae-bst.resource.bosch.com/media/products/dokumente/bno055/BST_BNO055_DS000_12~1.pdf
     3.6.5.5 Orientation (Quaternion)  */
  const double scale = (1.0 / (1<<14));
  imu::Quaternion quat(scale * w, scale * x, scale * y, scale * z);

  delete [] buffer;
  return quat;
}

//EULER ANGLES
imu::Vector<3> BNO055::getEuler(void)
{
  imu::Vector<3> xyz;
  unsigned char* buffer;
  int16_t x, y, z;
  x = y = z = 0;
   
  buffer = readLen(BNO055_EULER_H_LSB_ADDR, 6);

  x = ((int16_t)buffer[0]) | (((int16_t)buffer[1]) << 8);
  y = ((int16_t)buffer[2]) | (((int16_t)buffer[3]) << 8);
  z = ((int16_t)buffer[4]) | (((int16_t)buffer[5]) << 8);

  /* 1 degree = 16 LSB */
  xyz[0] = ((double)x)/16.0;
  xyz[1] = ((double)y)/16.0;
  xyz[2] = ((double)z)/16.0;
  
  delete [] buffer;
  return xyz;
  
}



// CALIBRATION
/**************************************************************************/
/*!
    @brief  Gets current calibration state.  Each value should be a uint8_t
            pointer and it will be set to 0 if not calibrated and 3 if
            fully calibrated.
*/
/**************************************************************************/
void BNO055::getCalibration(uint8_t* sys, uint8_t* gyro, uint8_t* accel, uint8_t* mag) {
  uint8_t calData = read8(BNO055_CALIB_STAT_ADDR);
  if (sys != NULL) {
    *sys = (calData >> 6) & 0x03;
  }
  if (gyro != NULL) {
    *gyro = (calData >> 4) & 0x03;
  }
  if (accel != NULL) {
    *accel = (calData >> 2) & 0x03;
  }
  if (mag != NULL) {
    *mag = calData & 0x03;
  }
}

// BASIC READ/WRITE FUNCTIONS
byte BNO055::read8(adafruit_bno055_reg_t reg)
{
     byte readVal = 0;
     byte* preadVal = &readVal;
     bool result;
     
     result = write8(reg, 1);
     if(result = false) {
       fprintf(stdout,"error could not read from register %d\n", (uint8_t)reg);
    
     } else {
        result = read(_file,preadVal, 1);
     }
     
    return readVal;
  
} //end read8 


bool  BNO055::write8(adafruit_bno055_reg_t reg, byte value ) 
{
    int result;
    int length = 1; //length of data to write is 1 byte for write8;
    unsigned char txBuff[MAX_WRITE_LEN + 1];
    uint8_t  regAd  = (uint8_t)reg;
    txBuff[0] = regAd;

    unsigned char data[1];
    data[0] = (unsigned char)value;

    memcpy(txBuff + 1, data, length);
    result = write(_file,txBuff, length+1);
    
    if(result < 0) {
         return false;
    } else {
         return true;
    }

} //end write8

unsigned char * BNO055::readLen(adafruit_bno055_reg_t reg, int len)
{ 
     unsigned char* result  = new unsigned char[len];
     unsigned char  thisReg[1];
     thisReg[0] = (unsigned char) reg;

     if(write(_file,thisReg,1)!=1){
        fprintf(stdout, "failed to write to device\n");
     }
     
     if(read(_file,result, len)!=len){
       fprintf(stdout, "failed to read the full buffer\n");
      }

     return result;

}//end readLen



bool  BNO055::writeLen(adafruit_bno055_reg_t reg, unsigned char* data, int length ) /// UNTESTED!!!!!
{
    int result;
    unsigned char txBuff[MAX_WRITE_LEN + 1];
    uint8_t  regAd  = (uint8_t)reg;
    txBuff[0] = regAd;

    memcpy(txBuff + 1, data, length);
    result = write(_file,txBuff, length+1);
    
    if(result < 0) {
         return false;
    } else {
         return true;
    }

} //end write8

void BNO055::delayMs(int milliSeconds)
{
   usleep(1000 * milliSeconds);
 
}

