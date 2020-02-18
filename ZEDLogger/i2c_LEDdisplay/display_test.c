#include <linux/i2c-dev.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int WriteChar( const int file , char buf);
int ClearDisplay( const int file);

int main(){

  unsigned char I2Cbus = 1;
  int addr = 0x71;

  int file;
  char filename[40];
  sprintf(filename,"/dev/i2c-1");

  if((file = open(filename, O_RDWR)) <0){  //open connection to i2c device with read/write capabilities
     perror("Failed to open the i2c bus");
     exit(1);
  }

  if(ioctl(file,I2C_SLAVE,addr) < 0) {
     printf("Failed to acquire bus access and/or talk to slave.\n");
     exit(1);
  }
  ClearDisplay(file);
  sleep(1);
  char run[3] = {0x52, 0x55, 0x4E};  //ascii hex values of characters to write, in this case "RUN". 7 seg display does its best to display these characters
  WriteChar(file,run[0]);
  WriteChar(file,run[1]);
  WriteChar(file,run[2]);
  
  char prog[1] = {0x01};
  WriteChar(file, prog[0]);

}

int WriteChar( const int file, char buf){
   char *pbuf = &buf;  //I prefered to pass characters but 'write' wants a pointer to character array
   int res = write(file,pbuf,1);
   if(res != 1){
   printf("Failed to write to the i2c bus.\n");
   }
   return res; 
   
}

int ClearDisplay(const int file){
  char clrcmd[1] = {0x76};   //see sparkfun datasheet - this is the command to clear the display (ascii 'u')
  int res = write(file, clrcmd,1);
  if( res != 1){
   printf("Failed to clear display\n");
  } 
  return res;

}
