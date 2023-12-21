#include "ArduinoOTA.h"
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#include <Wire.h>

#include <DS3231.h>
#include <MFRC522.h>
#include <SD.h>
#include <SPI.h>

#include "time.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

//#define MUTE

#define MFR_RST_PIN D3 // Configurable, see typical pin layout above 18
#define MFR_SS_PIN D0  // Configurable, see typical pin layout above  16
#define SD_SS_PIN D8
#define IRQ_PIN A0
#ifndef MUTE
#define BUZZER D4
#endif
#define ON LOW
#define OFF HIGH

const char *ssid = "Mi Wi-Fi";     //"Keenetic-2077";
const char *password = "12345678"; //"WruBAw4Z";

const long utc_in_sec = 7200;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utc_in_sec);
MFRC522 mfrc(MFR_SS_PIN, MFR_RST_PIN); // Create MFRC522 instance
DS3231 myRTC;
MFRC522::MIFARE_Key key;

bool century = false, h12Flag, pmFlag;

void setup() {
  Wire.begin();
  Serial.begin(9600);

  pinMode(MFR_RST_PIN, OUTPUT);
  pinMode(SD_SS_PIN, OUTPUT);
#ifndef MUTE
  pinMode(BUZZER, OUTPUT);
#endif

  digitalWrite(SD_SS_PIN, ON);
  delay(100);

  SPI.begin();

  if (!SD.begin(0))
#ifndef MUTE
    tone(BUZZER, 300, 500); // Карта памяти не обнаружена или повреждена :(");
#else
    ;
#endif
  else
    Serial.print("\nSD Init :");

  File myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {
    Serial.println(" good");
    myFile.close();
  } else
    Serial.println(" bad");

  digitalWrite(SD_SS_PIN, OFF);

  Serial.println("MFRC Init");
  mfrc.PCD_Init();                // Init MFRC522
  mfrc.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card

  digitalWrite(SD_SS_PIN, ON);
  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("\nWiFi connect");
  int ConnectTime = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ConnectTime++;
    Serial.print(".");
    if (ConnectTime > 100) {
#ifndef MUTE
      tone(BUZZER, 300, 500);
#endif
      break;
    }
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.begin();
  Serial.println("OTA init");

  timeClient.begin();
  timeClient.update();

  unsigned long long now = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&now);

  myRTC.setSecond(ptm->tm_sec);
  myRTC.setMinute(ptm->tm_min);
  myRTC.setHour(ptm->tm_hour);
  myRTC.setDate(ptm->tm_mday);
  myRTC.setMonth(ptm->tm_mon);
  myRTC.setYear(ptm->tm_year - 100);
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

#ifndef MUTE
  tone(BUZZER, 500, 100);
  delay(100);
  tone(BUZZER, 500, 100);
#endif

  Serial.println("Init done");
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void loop() {
  ArduinoOTA.handle();

  if (mfrc.PICC_ReadCardSerial() || mfrc.PICC_IsNewCardPresent()) {
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc.uid.uidByte, mfrc.uid.size);
    Serial.println();
#ifndef MUTE
    tone(BUZZER, 500, 1000);
#endif
  }

  delay(50);
}