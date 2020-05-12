
#include <Wire.h> // Include the Arduino SPI library

// Here we'll define the I2C address of our S7S. By default it
//  should be 0x71. This can be changed, though.
const byte s7sAddress = 0x71;
String readString;
String option; 
String value;
bool accum; 
int ind1; //delimited index

void setup() {
  Serial.begin(115200);
  Wire.begin();  // Initialize hardware I2C pins
  accum = false;
}

void loop() {
  if(Serial.available())
  {
    char ch = Serial.read();
    if(ch == 35) { //start character: # (ascii 35)
      //clear values
      readString = "";
      option = "";
      value = "";
      accum = true;
    }
      
    if(ch == 10 && accum) {  //end indicator: new line character - do stuff
     //   Serial.println(readString);
        ind1   = readString.indexOf(',');
        option = readString.substring(1,ind1);  //start at 1 to dump start character
        value  = readString.substring(ind1+1);
    //    Serial.println(value);
  
        //switch on option
        if(option.equals("7seg")){
            clearDisplayI2C();
            s7sSendStringI2C(value);
        }
        else {
      //      Serial.println("option not recognized");
        }

        accum = false;
  
    } // end if on end character
    
    if(accum){
        readString += ch;
    }

 }//end if on Serial.available()

}
//  You can send it an array of chars (string) and it'll print
//  the first 4 characters in the array.
void s7sSendStringI2C(String toSend)
{
  Wire.beginTransmission(s7sAddress);
  for(int i=0; i<min(4,toSend.length()); i++)
  {
    Wire.write(toSend[i]);
  }
  Wire.endTransmission();
}
void clearDisplayI2C()
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x76);  // Clear display command
  Wire.endTransmission();
}
