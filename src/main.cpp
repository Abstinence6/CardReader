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
#include "time.h"

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

bool century = false, h12Flag, pmFlag;

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
    Serial.print("\nSD Init :");

  File myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile)
  {
    Serial.println(" good");
    myFile.close();
  }
  else
    Serial.println(" bad");

  digitalWrite(SD_SS_PIN, OFF);
  digitalWrite(MFR_SS_PIN, ON);
  delay(100);

  Serial.println("MFRC Init");
  mfrc.PCD_Init();                // Init MFRC522
  mfrc.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
  mfrc.PCD_AntennaOn();

  digitalWrite(SD_SS_PIN, ON);
  digitalWrite(MFR_SS_PIN, OFF);
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("\nWiFi connect");
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

  unsigned long long now = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&now);

  myRTC.setSecond(ptm->tm_sec);
  myRTC.setMinute(ptm->tm_min);
  myRTC.setHour(ptm->tm_hour);
  //myRTC.setDoW(timeClient.getDay());
  myRTC.setDate(ptm->tm_mday);
  myRTC.setMonth(ptm->tm_mon);
  myRTC.setYear(ptm->tm_year-100);
  myRTC.setClockMode(0);

  Serial.print(ptm->tm_year + 1900, DEC);
  Serial.print("-");
  Serial.print(ptm->tm_mon, DEC);
  Serial.print("-");
  Serial.print(ptm->tm_mday, DEC);
  Serial.print(" ");
  Serial.print(ptm->tm_hour, DEC); // 24-hr
  Serial.print(":");
  Serial.print(ptm->tm_min, DEC);
  Serial.print(":");
  Serial.println(ptm->tm_sec, DEC);

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

void loop()
{
  ArduinoOTA.handle();

  if (mfrc.PICC_ReadCardSerial() || mfrc.PICC_IsNewCardPresent())
  {
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc.uid.uidByte, mfrc.uid.size);
    Serial.println();
  }


}