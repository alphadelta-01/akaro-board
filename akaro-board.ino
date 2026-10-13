#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <WiFi.h>
#include <time.h>

#define BTN_UP 0
#define BTN_DOWN 1
#define BTN_LEFT 3
#define BTN_RIGHT 10  

#define I2C_SDA 5
#define I2C_SCL 6

#define WIFI_SSID "zte10"
#define WIFI_PASSWORD "3963939639"

int8_t current_item = 0;

RTC_DATA_ATTR int8_t hours = 0;
RTC_DATA_ATTR int8_t minutes = 0;
RTC_DATA_ATTR int8_t seconds = 0;

RTC_DATA_ATTR int8_t utc = 3;

const char *EXTRAS_ITEMS[] = {
    "CALCULATOR",
    "FLASHLIGHT",
    "CALENDAR",
    "CLOCK",
    "WEATHER"};

const int8_t EXTRAS_ITEMS_COUNT = sizeof(EXTRAS_ITEMS) / sizeof(EXTRAS_ITEMS[0]);

const char *GAMES_ITEMS[] = {
    "TETRIS",
    "DYNO",
    "PONG",
    "COUNTER"};

const int8_t GAMES_ITEMS_COUNT = sizeof(GAMES_ITEMS) / sizeof(GAMES_ITEMS[0]);

const char *SETTINGS_ITEMS[] = {
    "TIME",
    "WIFI",
    "BRIGHTNESS",
    "LANGUAGE",
    "DEEP-SLEEP",
    "ABOUT"};

const int8_t SETTINGS_ITEMS_COUNT = sizeof(SETTINGS_ITEMS) / sizeof(SETTINGS_ITEMS[0]);

enum AppState
{
  MainMenu,
  Extras,
  Games,
  Settings
};

AppState APP_STATE = MainMenu;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, I2C_SCL, I2C_SDA);

struct tm timeInfo;

void syncTime()
{

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (int8_t i = 0; i <= 25; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      break;
    }
    delay(500);
  }

  configTime(utc * 3600, 0, "pool.ntp.org", "time.google.com");

  while (!getLocalTime(&timeInfo))
  {
    delay(75);
  }
}

void updateClock()
{
  if (getLocalTime(&timeInfo))
  {
    hours = timeInfo.tm_hour;
    minutes = timeInfo.tm_min;
    seconds = timeInfo.tm_sec;
  }
}

void init()
{
  Wire.begin(I2C_SDA, I2C_SCL);

  u8g2.begin();
  u8g2.setPowerSave(0);

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_tenthinguys_tr);
  u8g2.drawStr((128 - u8g2.getUTF8Width("akaro-board")) / 2, 37, "akaro-board");

  u8g2.setFont(u8g2_font_tiny5_tf);
  u8g2.drawStr((128 - u8g2.getUTF8Width("loading...")) / 2, 47, "loading...");

  u8g2.sendBuffer();

  syncTime();
}

void statusBarDraw()
{
  char clockText[9];

  snprintf(clockText, sizeof(clockText), "%02d:%02d:%02d", hours, minutes, seconds);

  u8g2.setFont(u8g2_font_micro_tn);

  u8g2.drawStr(3, 7, clockText);
}

void input()
{
  int8_t max_items = 0;

  switch (APP_STATE)
  {
  case MainMenu:
    max_items = 2;
    break;

  default:
    break;
  }

  static int8_t prev_btn_up = HIGH;
  static int8_t prev_btn_down = HIGH;
  static int8_t prev_btn_left = HIGH;
  static int8_t prev_btn_right = HIGH;

  int8_t cur_btn_up = digitalRead(BTN_UP);
  int8_t cur_btn_down = digitalRead(BTN_DOWN);
  int8_t cur_btn_left = digitalRead(BTN_LEFT);
  int8_t cur_btn_right = digitalRead(BTN_RIGHT);

  if (cur_btn_down == LOW && prev_btn_down == HIGH)
  {
    if (current_item < max_items)
    {
      current_item++;
    }
  }
  else if (cur_btn_up == LOW && prev_btn_up == HIGH)
  {
    if (current_item > 0)
    {
      current_item--;
    }
  }

  else if((cur_btn_left == LOW && prev_btn_left == HIGH) && (cur_btn_right == LOW && prev_btn_right == HIGH)){
    u8g2.drawStr(10, 10, "YES!");
    u8g2.sendBuffer();
  }

  delay(20);

  prev_btn_up = cur_btn_up;
  prev_btn_down = cur_btn_down;
  prev_btn_left = cur_btn_left;
  prev_btn_right = cur_btn_right;
}

void menuDraw(const char *name, int8_t itemsDraw)
{
  u8g2.clearBuffer();

  statusBarDraw();

  u8g2.drawRFrame((128 - 54) / 2, 10, 54, 36, 5);

  u8g2.setFont(u8g2_font_tenthinguys_tr);

  int8_t textWidth = u8g2.getUTF8Width(name);
  int8_t x = (128 - textWidth) / 2;

  u8g2.drawStr(x, 60, name);

  for (int8_t i = 0; i < itemsDraw; i++)
  {
    int8_t y = 2 + i * (6 + 3);
    u8g2.drawFrame(128 - 10, y, 8, 8);

    if (i == current_item)
    {
      u8g2.drawBox(128 - 10, y, 8, 8);
    }
  }

  u8g2.sendBuffer();
  delay(16);
}

void drawAllMenu()
{
  switch (APP_STATE)
  {
  case MainMenu:
    switch (current_item)
    {
    case 0:
      menuDraw("Extras", 3);
      break;
    case 1:
      menuDraw("Games", 3);
      break;
    case 2:
      menuDraw("Settings", 3);
      break;
    }
  }
}

void setup()
{
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  init();
}

void loop()
{
  updateClock();
  input();
  drawAllMenu();
}

// compile = arduino-cli compile -b esp32:esp32:esp32c3 -u -p (YOUR COM-PORT - arduino-cli board list)
