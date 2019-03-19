// Code for the BMP280. Temp, Pressure, Altidude. 
// Basic I2C reading in and intepereting register values. 

#include <Wire.h>
#define BMP280_Address 0x77 

uint16_t dig_T1; // unsigned short (from datasheet)
int16_t dig_T2; // signed short
int16_t dig_T3; 
uint16_t dig_P1; // unsigned short
int16_t dig_P2;
int16_t dig_P3;
int16_t dig_P4;
int16_t dig_P5;
int16_t dig_P6;
int16_t dig_P7;
int16_t dig_P8;
int16_t dig_P9;

int32_t  X1, X2, X3, B3, B5, B6, UP, UT, p;
uint32_t B4, B7;
//Below we choose the oversampling and conversion time. 
const uint8_t OverSampSett = 3;                       // We choose the oversampling rate 
const uint8_t ConvTimDel = 26;                       // We set the conversion time Delay~! (choose this in relation with the OSS).  
// the above two values have beeen chosen for ULTRA HIGH RESOLUTION MODE  
// Refer to the datasheeet if you want a different resolution

float T, P, A;                                             // Define our variables temp, press, altitude

void setup() 
{
Wire.begin();         // Begin communication with i2c device.
Serial.begin(9600);
}

void loop()
{
  // put your main code here, to run repeatedly:

 Serial.print("Environmental Temperature(°F) -  ");
 Serial.print(T,3);                 // The 3 denotes the amount of decimal places the measurement gives us
 Serial.print("|||"); 

 Serial.print("Environmental Pressure(HPa) -  ");  // Hectopascals 
 Serial.print(P, 2);                   // Pressure in 2 decimal places, 
 Serial.print("|||");

 A = 44330 * (1 - pow((P /968.244), 0.1903)); // Formula for altitude from page 16 
 Serial.print("Environmental Altitude (Meters) -  "); 
 Serial.print( A, 2);                   // Altitude, 2 decimal place accuracy 
 Serial.println("");
 Serial.println("");
 delay(20);

  groupAddresses(); // Initalize with this function and bring values to the loop with addresses , these are in global variables 
  ReadUncompTemp (); 
  ReadUncompPress (); 
  
}


//Create a function that reads in the values from the specific address for a byte


uint32_t readtoregistersFXN(uint8_t code, uint8_t numbytes)  // To properly adjust our resolution

{
  uint32_t value = 0;
  Wire.beginTransmission(BMP280_Address);         
  Wire.write(code);           // Write to the sensor the code that says I want this info                 
  Wire.endTransmission();      // We have to end because we need to tell it that we are done with our talk now it can speak to me                    
  Wire.requestFrom(BMP280_Address, numbytes); // We request a value of numbytes here/ either 2 or 3 so we can use this in relation with the temp and press fxns              
  if(Wire.available() >= numbytes) // return amount of bytes that have been stored to serial buffer 
  {
    for(uint8_t i = 0; i < numbytes; i++)
    {  
      value = (value << 8) | Wire.read();   // its shifts 8 over 
    }
  }                                              
  return value; 
}   
// now we will define where our register values are~~~

void groupAddresses ()
{   
  dig_T1 = readtoregistersFXN(0x88, 2); // we are reading 2 bytes
  dig_T2 = readtoregistersFXN(0x8A, 2);
  dig_T3 = readtoregistersFXN(0x8C, 2);
  dig_P1 = readtoregistersFXN(0x8E, 2);
  dig_P2 = readtoregistersFXN(0x90, 2);
  dig_P3 = readtoregistersFXN(0x92, 2);
  dig_P4 = readtoregistersFXN(0x94, 2);
  dig_P5 = readtoregistersFXN(0x96, 2);
  dig_P6 = readtoregistersFXN(0x98, 2);
  dig_P7 = readtoregistersFXN(0x9A, 2);
  dig_P8 = readtoregistersFXN(0x9C, 2);
  dig_P9 = readtoregistersFXN(0x9E, 2);
}

// we define a function to read uncompensated temperature value from the datasheet flow chart

void ReadUncompTemp ()
{
  Wire.beginTransmission(BMP280_Address); 
  Wire.write(0xF4);
  Wire.write(0x2E); 
  Wire.endTransmission(); 
  delay(4.5); 

UT = readtoregistersFXN(0xF7, 3);   // we read our UT using the readtoreg function from the address given in datasheet
 
// below we calculate True Temperature Value 
  X1 = (UT - (int32_t)AC6)*(int32_t)AC5/ pow(2,15);
  X2 = ((int32_t)MC * pow(2, 11)) / (X1 + (int32_t)MD); 
  B5 = (X1 + X2);
  T = (B5+8)/ pow(2,4); 
  T = T/10;
  T = T * 9/5 + 32;  // celcius to F conversion, this line can be deleted if you prefer celcius measurement
  return B5; 
  
}

void ReadUncompPress ()
{
  Wire.beginTransmission(BMP280_Address); 
  Wire.write(0xF4);
  Wire.write(0x34 + OverSampSett << 6); 
  Wire.endTransmission(); 
  delay(ConvTimDel); // End of first row of instructions in data sheet 
                    // Begin second row of instructions in "read uncompensated pressure value" 
 
  // Begin third row of instructions in "read uncompensated pressure value" 
 UP = (readtoregistersFXN(0xFA, 3)) >> (8-OverSampSett);         // Here we read in 3 bytes (MSB, LSB and XLSB) 
                                                                 
  // Calculate true pressure 
  
  B6 = B5 - 4000;
  X1 = (B2 *(B6 * B6 / 4092) / 2048) ; 
  X2 = AC2 * B6 / 2048; 
  X3 = X1 + X2; 
  B3 = (((AC1*4+X3) << OverSampSett) + 2) / 4 ; 
  X1 = AC3 * B6 / 8192; 
  X2 = (B1 * (B6 * B6 / 4096)) / 65536; 
  X3 = ((X1 + X2) + 2)/ 4; 
  B4 = AC4 *( (unsigned long) (X3 + 32768)) / 32768; 
  B7 = ((unsigned long) UP - B3) * (50000 >> OverSampSett); 
  if (B7 < 0x80000000)
  {
    P = (B7*2) / B4;
  } 
  else
  {
    P = (B7 / B4) * 2;
  }
  
  X1 = (P / 256) *(P / 256); 
  X1 = (X1 * 3038) / 65536; 
  X2 = (-7357 * P) / 65536; 
  P =  P + (X1 + X2 + 3791) / 16;
  P = P/100; // this gives us HPa 
  return P; 
 


}
