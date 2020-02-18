#include "adaimu_test.h"
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

int main(void){
  char buf[32];
  int I2CBus  = 1;
  int I2CAddr = 0x28; 
  int m_I2C;
  sprintf(buf, "/dev/i2c-%d", I2CBus);
  m_I2C = open("/dev/i2c-1", O_RDWR);
  if (m_I2C < 0) {
    fprintf(stdout, "Failed to open I2C bus %d\n", I2CBus);
     m_I2C = -1;
     return -1;
  }
  if(ioctl(m_I2C, I2C_SLAVE, I2CAddr) <0){
    fprintf(stdout, "Failed to connect to the sensor\n");
    return -1;
  }
  
  //simple TEMP read
  int addr = 0x34;
  int* regAddr = &addr;
  unsigned char* data[1];
  int result;
  write(m_I2C,regAddr,1);
  result = read(m_I2C, data,1);
  if(result < 0){
    fprintf(stdout, "error reading from register\n");
  }
  
  int8_t* temp = (int8_t*)data;
  fprintf(stdout, "temp: %d\n", *temp);

  //calibration status
  addr = 0x35;
  write(m_I2C, regAddr,1);
  read(m_I2C, data,1);
  uint8_t* calData =(uint8_t*)data;
  uint8_t  sys;
  uint8_t  gyro;
  uint8_t  accel;
  uint8_t  mag;
 
  sys = (*calData >> 6) & 0x03;
  gyro = (*calData >> 4) & 0x03;
  accel = (*calData >> 2) & 0x03;
  mag = *calData & 0x03;

  fprintf(stdout, "sys cal: %d\n", sys);
  fprintf(stdout, "gyro cal: %d\n", gyro);
  fprintf(stdout, "accel cal: %d\n", accel);
  fprintf(stdout, "mag cal: %d\n", mag);

  //Euler angles
  uint8_t buffer[6];
  memset(buffer, 0, 6);
  int16_t x, y, z;
  x = y = z = 0;

  /* Read vector data (6 bytes) */
  //readLen((adafruit_bno055_reg_t)vector_type, buffer, 6);
    addr = 0x1A;
    write(m_I2C, regAddr,1);
    int length = 6;
    int total = 0;
    int tries = 0;

        while ((total < length) && (tries < 5)) {
            result = read(m_I2C, buffer + total, length - total);

            if (result < 0) {
                    fprintf(stdout,"I2C read error from %d, %d\n", I2CAddr, *regAddr);
                return -1;
            }

            total += result;

            if (total == length)
                break;

            sleep(10000);
            tries++;
        }

        if (total < length) {
                fprintf(stdout,"I2C read from %d, %d failed\n", I2CAddr, *regAddr);
            return -1;
        }


  x = ((int16_t)buffer[0]) | (((int16_t)buffer[1]) << 8);
  y = ((int16_t)buffer[2]) | (((int16_t)buffer[3]) << 8);
  z = ((int16_t)buffer[4]) | (((int16_t)buffer[5]) << 8);

  double xd, yd, zd;
  xd = ((double)x)/16.0;
  yd = ((double)y)/16.0;
  zd = ((double)z)/16.0;

  fprintf(stdout,"x: %f y: %f z: %f \n", xd, yd, zd);


  close(m_I2C);

  return 0;

}

