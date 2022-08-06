/*---------------------------------------------------------------------------*\
The following code is used to work at constant pressure in a filtration 
system where the pump is regulated through a variable-frequency drive (VFD). 

The Arduino Mega 2560 board gains the information “PressureTarget” directly from the 
user through the keypad, and it displays it on the LCD screen together with the instant 
pressure value, which is taken from the data logger by converting the current value into 
an analog voltage input. 

Arduino measures the difference between the instant pressure and the “PressureTarget” 
to act on the variable frequency drive and increase or decrease the pump outlet.

/*---------------------------------------------------------------------------*\
 * 
Code corresponding to the article entitled:

"Kinetic and mechanistic analysis of membrane fouling in microplastics removal from water 
by dead-end microfiltration"

by:
A. Raffaella P. Pizzichetti, Cristina Pablos, Carmen Álvarez-Fernandez, and Javier Marugán
from
    Department of Chemical and Environmental Technology, ESCET, Universidad Rey Juan Carlos, 
    C/Tulipán s/n, 28933 Móstoles, Madrid, Spain 
and 
A. Raffaella P. Pizzichetti, Ken Reynolds and Simon Stanley
from 
  ProPhotonix IRL LTD, 3020 Euro Business Park, Little Island, Cork, T45 X211, Ireland 

Tel. +34 91 488 46 19; E-mail: cristina.pablos@urjc.es
    

License

  This file is is free software: you can redistribute it and/or modify it under the terms 
  of the GNU General Public License as published by the Free Software Foundation, either 
  version 3 of the License, or (at your option) any later version. You should have received 
  a copy of the GNU General Public License along with the code. 
  If not, see <http://www.gnu.org/licenses/>.

    
\*---------------------------------------------------------------------------*/



#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

const byte rowsCount = 4;
const byte columsCount = 4;
 
char keys[rowsCount][columsCount] = {
   { '1','2','3', 'A' },
   { '4','5','6', 'B' },
   { '7','8','9', 'C' },
   { '.','0','#', 'D' }
};  // keypad definition
 
const byte rowPins[rowsCount] = { 39, 41, 43, 45 }; // keypad pins
const byte columnPins[columsCount] = { 47, 49, 51, 53 }; // keypad pins
char pad; 
char padText[5]; 
char padPosition; 
int inputPin = A0; //pin connected to the pressure value of the Data Logger through a current to voltage converter
const int numReadings = 400;
unsigned int RawValue = 0;
float DigitalSignal = 0;
float Pressure;
float PressureTarget; 
const int PinVFD = 8; // pin connected to the Variable Frequency Drive
byte outputVFD = 0;  // value in input to the Variable Frequency Drive
float Sensor = 0; //value taken from the Data Logger 
 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rowsCount, columsCount); // keypad initialization
 
LiquidCrystal_I2C lcd(0x27, 16, 2);  // start the LCD screen with 16 characters and 2 lines at initial position 


void setup()
{
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps to comunicate with the Serial Monitor
  Serial.println("/** Water pressure sensor measurement **/");
  Serial.print("Pad \t PressureTarget \t DigitalSignal \t Pressure (bar)"); //to initialize the first line of the Serial Monitor
    Serial.println();
     lcd.begin();                      
   lcd.backlight();
   padPosition = -1;
}

void loop()
{
  char key = keypad.getKey();
 
  if (key) {
    pad = key; //save the value added in the keypad
    padPosition=padPosition+1;
  }
  if(pad=='D'){
    //pressing D to save the value inserted from the keypad as PressureTarget 
    if(padPosition>-1){
      PressureTarget = (float)strtod(padText,NULL); 
      if(PressureTarget > 2.5) //to establish a P_max
        PressureTarget=2.5;
      for(int i = 0; i < 5; i++)
        padText[i]=0;
      padPosition=-1;
      lcd.clear();
    }
  }
  else if(pad=='B'){
    //pressing B to delete the values inserted from the keypad
    for(int i = 0; i < 5; i++)
      padText[i]=0;
    padPosition=-1;
    lcd.clear();
  }
  else if(padPosition < 4){
    //to insert more than one value as input on the keypad
    padText[padPosition]=pad;
  }
 
  for(int i= 0; i < 4; i++)
    Serial.print(padText[i]); 
  
  Serial.print("\t");
  Serial.print(PressureTarget);
  Serial.print("\t");
  
  for (int x = 0; x < numReadings; x++) // analog readings for averaging the input pressure given from the Data Logger 
  {
    Sensor = analogRead(inputPin); //return an analog voltage
    RawValue = RawValue + map(Sensor,0,1023,0,255); // add each A/D reading to a total, map function to convert analog voltage to digital signal
    delay(1); // wait "x" milliseconds before the next loop
  }

  DigitalSignal = ((RawValue / numReadings) ); // returns the averaged digital signal
  Pressure= 0.0259*DigitalSignal-0.0764;  // calculate pressure from the digital signal value

  //act on the Variable Frequency Drive to achieve the Pressure Target value
  if (Pressure<PressureTarget-0.2){
  outputVFD=outputVFD+10;
  }
  else if (Pressure>PressureTarget+0.2){
  outputVFD=outputVFD-10;
  }
  else if (Pressure<PressureTarget){
  outputVFD=outputVFD+1;
  }
  else if (Pressure>PressureTarget){
  outputVFD=outputVFD-1;
  }
    
  analogWrite(PinVFD, outputVFD);

  Serial.print(DigitalSignal, 3); // shows the digital signal measured, the '3' after allows you to display 3 digits after decimal point
  Serial.print("\t");

  Serial.print(Pressure, 2);
  
  Serial.println(); //newline
  
  RawValue = 0; // reset value

  //write on the LCD screen
   lcd.setCursor(0, 0); //first line
   lcd.print(padText); //show the values while you are writing them on the keypad
   lcd.setCursor(6, 0);
   lcd.print("Pt");
   lcd.setCursor(9, 0);
   lcd.print(PressureTarget); //show the value after saving it as PressureTarget 
   lcd.setCursor(0, 1); //second line
   lcd.print("P(bar)");
   lcd.setCursor(9, 1);
   lcd.print(Pressure); //show the pressure from the Data Logger


}
