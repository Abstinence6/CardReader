#include <Arduino.h>
#include "ArduinoOTA.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>

#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>
#include <DS3231.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#define MFR_RST_PIN D3 // Configurable, see typical pin layout above 18
#define MFR_SS_PIN D0  // Configurable, see typical pin layout above  16
#define SD_SS_PIN D8
#define BUZZER A0 //D4
#define ON LOW
#define OFF HIGH

const char *ssid = "Keenetic-2077";
const char *password = "WruBAw4Z";

const long utc_in_sec = 7200;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"europe.pool.ntp.org",utc_in_sec);
MFRC522 mfrc(MFR_SS_PIN, MFR_RST_PIN); // Create MFRC522 instance
DS3231 myRTC;

void setup()
{
  Wire.begin();
  Serial.begin(9600);

  pinMode(MFR_RST_PIN, OUTPUT);
  pinMode(MFR_SS_PIN, OUTPUT);
  pinMode(SD_SS_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(MFR_SS_PIN, OFF);
  digitalWrite(SD_SS_PIN, ON);
  delay(100);

  SPI.begin();

  if (!SD.begin(0))
    tone(BUZZER, 300, 500); // Карта памяти не обнаружена или повреждена :(");
  else
    Serial.println("SD Init");

  File myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile)
  {
    Serial.print("Writing to test.txt...");
    myFile.close();
  }
  else
  {
    Serial.println("error opening test.txt");
  }

  digitalWrite(SD_SS_PIN, OFF);
  digitalWrite(MFR_SS_PIN, ON);
  delay(100);

  mfrc.PCD_Init();                // Init MFRC522
  mfrc.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
  mfrc.PCD_AntennaOn();
  Serial.println("MFRC Init");

  digitalWrite(SD_SS_PIN, ON);
  digitalWrite(MFR_SS_PIN, OFF);
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int ConnectTime = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    ConnectTime++;
    Serial.print(".");
    if (ConnectTime > 100)
      break;
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.begin();
  Serial.println("OTA init");

  timeClient.begin();
  timeClient.update();
  // myRTC.setEpoch(time_t epoch = 0, bool flag_localtime = false);
  myRTC.setSecond(timeClient.getSeconds());
  myRTC.setMinute(timeClient.getMinutes());
  myRTC.setHour(timeClient.getHours());
  // myRTC.setDoW(byte DoW);
  myRTC.setDate(timeClient.getDay());
  myRTC.setMonth(12);
  myRTC.setYear(23);
  myRTC.setClockMode(0);
  Serial.println("Time init");

  Serial.println("Init done");
}

void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

bool century = false, h12Flag, pmFlag;

void loop()
{
  ArduinoOTA.handle();

  if (mfrc.PICC_ReadCardSerial() || mfrc.PICC_IsNewCardPresent())
  {
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc.uid.uidByte, mfrc.uid.size);
    Serial.println();
  }

  Serial.print(myRTC.getYear(), DEC);
  Serial.print("-");
  Serial.print(myRTC.getMonth(century), DEC);
  Serial.print("-");
  Serial.print(myRTC.getDate(), DEC);
  Serial.print(" ");
  Serial.print(myRTC.getHour(h12Flag, pmFlag), DEC); // 24-hr
  Serial.print(":");
  Serial.print(myRTC.getMinute(), DEC);
  Serial.print(":");
  Serial.println(myRTC.getSecond(), DEC);

  delay(900);
}
/*
String str_buf;
String actual_date; 
String actual_time;

void check_date()   
{
  timeClient.update();
  str_buf = timeClient.getFormattedDate(); // возвращается в таком виде 2021-11-30T16:00:13Z
  int splitT = str_buf.indexOf("T");
  actual_date = str_buf.substring(0,splitT); //2021-11-30
  actual_date.replace("-",";");
  str_buf = "";
}
//=======================================заполняет актуальное время используя данные из интернета======================================
void check_time()        
{
   timeClient.update();
  str_buf = timeClient.getFormattedDate(); // возвращается в таком виде 2021-11-30T16:00:13Z
  int splitT = str_buf.indexOf("T");
  actual_time = str_buf.substring(splitT+1,str_buf.length()-4); //16:00:13
  str_buf = "";
}
*/
/*
void setup()
{
  Wire.begin();

  Serial.begin(9600);
  Serial.println("\nI2C Scanner");
}


void loop()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknow error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(5000);           // wait 5 seconds for next scan
}
*/