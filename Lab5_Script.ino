#include<SoftwareSerial.h>
SoftwareSerial transmit(6, 7); // digital pin 6 is receiving and 7 is transmitting
  
void setup() {
  // put your setup code here, to run once:

  transmit.begin(9600); // begin data transfer rate at 9600
  transmit.println("PowerSupply,LM1117,LM7805,Time"); // sets the top line of the SD card
}

// Initialize variables
unsigned long times; // sets the time variable
double LM7805_voltage = 0;
double LM1117_voltage = 0;
double PowerVolts = 0;
double conversion = (4.71/1024); // conversion factor from analog to voltage

void loop() {
LM1117_voltage = analogRead(5);
LM7805_voltage = analogRead(3);
PowerVolts = analogRead(1);

LM1117_voltage = LM1117_voltage * conversion;
LM7805_voltage = LM7805_voltage * conversion;
PowerVolts = PowerVolts * conversion;

times = millis(); // sets the time

// Print voltages and times on each line
transmit.print(PowerVolts); 
transmit.print(",");
transmit.print(LM1117_voltage); 
transmit.print(",");
transmit.print(LM7805_voltage);
transmit.print(",");
transmit.println(times);
delay(1000);
}
