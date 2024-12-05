#include <SPI.h>
#include <TFT_eSPI.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "font/simfang16.h"

////////////////////////////////
const char* SSID = "CMCC-boyu1783";
const char* PASSWORD = "boyu1783";
const char* KEY = "xxx";
const char* CITY = "510100";
const char* URL = "https://restapi.amap.com/v3/weather/weatherInfo";

const char* test_json = " \
  { \
    \"status\":\"1\", \
    \"count\":\"1\", \
    \"info\":\"OK\", \
    \"infocode\":\"10000\", \
    \"lives\":[ \
      {\"province\":\"四川\", \
      \"city\":\"成都市\", \
      \"adcode\":\"510100\", \
      \"weather\":\"晴\", \
      \"temperature\":\"19\", \
      \"winddirection\":\"西南\", \
      \"windpower\":\"≤3\", \
      \"humidity\":\"72\", \
      \"reporttime\":\"2024-10-27 11:02:23\", \
      \"temperature_float\":\"19.0\", \
      \"humidity_float\":\"72.0\"} \
    ] \
  } \
";

TFT_eSPI tft = TFT_eSPI(128, 128);

struct WhetherMeta {
  String province;
  String city;
  String weather;
  String temperature;
  String windpower;
  String date;
  String time;
  String weekday;
};

struct WhetherMeta meta;

void _grid_line() {
  tft.setCursor(tft.getCursorX() + 4, tft.getCursorY() + 4);
}

void _parse_datetime(const char* datetime, struct WhetherMeta* meta) {
  char temp[20];
  strncpy(temp, datetime, sizeof(temp));
  temp[sizeof(temp) - 1] = '\0';  // Null-terminate to be safe

  char* token = strtok(temp, " ");

  if (token == NULL)
    return;

  meta->time = strtok(NULL, " ");

  char _date[20] = "";
  char* _token = strtok(token, "-");
  if (_token != NULL) {
    strcat(_date, strtok(NULL, "-"));
    strcat(_date, "月");
    strcat(_date, strtok(NULL, "-"));
    strcat(_date, "日");
    meta->date = String(_date);
  }

  meta->weekday = "日";
}


void _parse_json(const char* whether, struct WhetherMeta* meta) {
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, whether);

  if (error) {
    tft.setTextColor(TFT_RED);
    tft.println("parse error");
    return;
  }

  JsonObject _lives = doc["lives"][0];

  meta->province = String(_lives["province"]);
  meta->city = String(_lives["city"]);
  meta->weather = String(_lives["weather"]);
  meta->temperature = String(_lives["temperature"]) + "°";
  meta->windpower = String(_lives["winddirection"]) + " " + String(_lives["windpower"]);

  _parse_datetime(_lives["reporttime"], meta);
}

void setup(void) {
  Serial.begin(9600);

  tft.init();
  Serial.println("init successful " + String(tft.width()) + "-" + String(tft.height()));

  // wifi connection
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("connecting");
    delay(500);
  }

  Serial.println("wifi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());

  delay(1000);

  _parse_json(test_json, &meta);
}

void display(struct WhetherMeta* meta) {
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);

  tft.drawFastHLine(0, 28, tft.width(), TFT_WHITE);
  tft.drawFastVLine(88, 0, 28, TFT_WHITE);

  tft.drawFastHLine(0, 96, tft.width(), TFT_WHITE);
  tft.drawFastHLine(0, 124, tft.width(), TFT_WHITE);
  tft.drawFastVLine(24, 96, 28, TFT_WHITE);
  tft.drawFastVLine(tft.width() - 24, 96, 28, TFT_WHITE);

  tft.fillRect(0, 96, 24, 28, TFT_ORANGE);
  tft.fillRect(104, 96, 24, 28, TFT_ORANGE);

  tft.setCursor(32, 8);
  tft.setTextColor(TFT_WHITE);
  tft.loadFont(simfang16);
  tft.drawString(meta->city, 44, 14);
  tft.drawString(meta->weather, 108, 14);

  tft.drawString(meta->weekday, 12, 110);
  tft.drawString(meta->date, tft.width() / 2, 110);
  tft.drawString(meta->temperature, tft.width() - 12, 110);

  tft.unloadFont();

  tft.setTextColor(TFT_GREENYELLOW);
  tft.drawString(meta->time, tft.width() / 2, tft.height() / 2, 4);
}

void loop() {
  // Binary inversion of colours
  tft.invertDisplay(false);

  tft.fillScreen(TFT_BLACK);

  tft.setCursor(0, 0);

  display(&meta);

  delay(5000);
}
