#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

RTC_DS1307 rtc;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4);
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednessday", "Thursday", "Friday", "Saturday"};



int interrupt_1 = 2;
int interrupt_2 = 3;
int interrupt_3 = 18;
volatile int extEvent = 0;
String phoneToSMS = "07064952721";
//String phoneToSMS = "08074716528";


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial3.begin(19200);

  lcd.init();
  lcd.backlight();

  pinMode(interrupt_1, INPUT);
  pinMode(interrupt_2, INPUT);
  pinMode(interrupt_3, INPUT);
  
if (! rtc.begin()) {
    Serial.println ("Could not access Real Time Clock");
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("Could not access");
    lcd.setCursor(2, 2);
    lcd.print("Real Time Clock");
    while (1);
  }
  
  if (! rtc.isrunning()) {Serial.println("RTC is NOT running!");
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("RTC is NOT running!");

  }
  // we can manual set the RTC to the date and time or automatically set it with computer time with the sketch below
  //     rtc.adjust(DateTime(2020, 9, 14, 2, 44, 0));
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 


  attachInterrupt(digitalPinToInterrupt(interrupt_1), red, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interrupt_2), green, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interrupt_3), blue, CHANGE);

 while (!checkSIMModule()) {
    Serial.println("SIM Module is not responding");
    Serial.println();
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("SIM Module is not responding");
    delay(2000);
//    resetSim800();
  }
//  waitToNetCom();

  intialize();

  Serial.println("Initialization Completed");
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Distribution");
  lcd.setCursor(5, 1);
  lcd.print("Transformer");
  lcd.setCursor(5, 2);
  lcd.print("Monitoring");
  lcd.setCursor(7, 3);
  lcd.print("System");
  delay(2000);
  lcd.clear();


}

void loop() {
  // put your main code here, to run repeatedly:

  if (extEvent == 1) {
    delay(250);

    int interrupt1State = digitalRead(interrupt_1);
    int interrupt2State = digitalRead(interrupt_2);
    int interrupt3State = digitalRead(interrupt_3);
    //    String stateOfRed = (interrupt1State == 1) ? ("OPEN") : ("CLOSE");
    //    String stateOfGreen = (interrupt2State == 1) ? ("OPEN") : ("CLOSE");
    //    String stateOfBlue = (interrupt3State == 1) ? ("OPEN") : ("CLOSE");

    String stateOfRed = (interrupt1State == 1) ? ("CLOSE") : ("OPEN");
    String stateOfGreen = (interrupt2State == 1) ? ("CLOSE") : ("OPEN");
    String stateOfBlue = (interrupt3State == 1) ? ("CLOSE") : ("OPEN");

    String eventTimeStamp = currentTimeStamp();

    //Print out to Serial Monitor
    Serial.println("CURRENT PHASE STATUS. ");
    Serial.println(eventTimeStamp);
    Serial.print("Line 1: ");
    Serial.print(stateOfRed);
    Serial.print(" | ");
    Serial.print("Line 2: ");
    Serial.print(stateOfGreen);
    Serial.print(" | ");
    Serial.print("Line 3: ");
    Serial.print(stateOfBlue);
    Serial.println();

    // Display on 20 x 4 LCD
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("PHASE STATUS");
    lcd.setCursor(2, 1);
    lcd.print("Line 1:");
    lcd.setCursor(10, 1);
    lcd.print(stateOfRed);
    lcd.setCursor(2, 2);
    lcd.print("Line 2:");
    lcd.setCursor(10, 2);
    lcd.print(stateOfGreen);
    lcd.setCursor(2, 3);
    lcd.print("Line 3:");
    lcd.setCursor(10, 3);
    lcd.print(stateOfBlue);
    delay(50);


    //Build up the message to send via SMS
    String messageToServer = "CURRENT PHASE STATUS.\r ";
    messageToServer += eventTimeStamp;
    messageToServer += "\r";
    messageToServer += "Line 1: ";
    messageToServer += stateOfRed;
    messageToServer += "\r";
    messageToServer += "Line 2: ";
    messageToServer += stateOfGreen;
    messageToServer += "\r";
    messageToServer += "Line 3: ";
    messageToServer += stateOfBlue;

    sendSMS(messageToServer, phoneToSMS);

    extEvent = 0;
  }



}

void red() {
  extEvent = 1;
}


void green() {
  extEvent = 1;
}

void blue() {
  extEvent = 1;
}

void intialize() {
  extEvent = 1;
}

void sendSMS(String message, String number)
{
  String numb = "AT+CMGS=";
  numb += "\"" + number + "\"";
  numb += "\r";
  while (!checkSIMModule()) {
    Serial.println("SIM MODULE is not responding");
    Serial.println();
    delay(2000);
    //resetSim800();
  }
  Serial3.println("AT\r");
  delay(1000);
  Serial3.println("AT+CMGF=1\r");
  delay(1000);
  Serial3.println(numb);
  delay(1000);
  Serial3.println(message);
  delay(1000);
  Serial3.println((char)26);
  delay(250);
  serialEvent();
}

void serialEvent() {
  while (Serial.available())
  {
    Serial3.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (Serial3.available())
  {
    Serial.write(Serial3.read());//Forward what Software Serial received to Serial Port
  }
}

String currentTimeStamp() {
  DateTime now = rtc.now();
  String day = String(now.day());
  String month = String(now.month());
  String year = String(now.year());
  String hour = String(now.hour());
  String minute = String(now.minute());
  String second = String(now.second());

  String timeStamp = day;
  timeStamp += ("/");
  timeStamp += month;
  timeStamp += ("/");
  timeStamp += year;
  timeStamp += (" ");
  timeStamp += hour;
  timeStamp += (":");
  timeStamp += minute;
  timeStamp += (":");
  timeStamp += second;

  return timeStamp;
}

bool checkSIMModule() {
  //Serial3.begin(9600);
  while (Serial3.available() > 0)
    Serial3.read();
  Serial3.print("AT\r");
  delay(100);
  if (Serial3.available() > 0) {
    Serial3.read();
    return true;
    serialEvent();
  }
  else
    return false;
}

void waitToNetCom() {
  Serial.println("Waiting to Connect SIM to Network");
  Serial.println();
  do {
    Serial3.print("AT+COPS?\r");
  } while (Serial3.readString().indexOf("+COPS: 0,0,\"") == -1);
}
