#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <WiFi.h>
#include <time.h>

#define BTN_UP 1
#define BTN_DOWN 3
#define BTN_LEFT 0
#define BTN_RIGHT 4
#define BTN_ACCEPT 10
#define BTN_CANCEL 7

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

const char *MAIN_MENU_ITEMS[] = {
    "EXTRAS",
    "GAMES",
    "SETTINGS"};

const int8_t MAIN_MENU_ITEMS_COUNT = sizeof(MAIN_MENU_ITEMS) / sizeof(MAIN_MENU_ITEMS[0]);

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

void deepsleep(){
  u8g2.setPowerSave(1);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  esp_err_t err = esp_deep_sleep_enable_gpio_wakeup(1ULL << BTN_DOWN, ESP_GPIO_WAKEUP_GPIO_LOW);

  esp_deep_sleep_start();
}

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

bool deepsleepBind(){
  static uint32_t chordStartTime = 0;
  static bool chordActive = false;

  bool chordDown = digitalRead(BTN_ACCEPT) == LOW && digitalRead(BTN_CANCEL) == LOW;

  if(!chordDown){
    chordStartTime = 0;
    chordActive = false;
    return false;
  }

  if(chordStartTime == 0){
    chordStartTime = millis();
    return false;
  }

  if(!chordActive && (millis() - chordStartTime >= 2000)){
    chordActive = true;
    return true;
  }
  return false;
}

void input()
{
  if(deepsleepBind()){
    deepsleep();
    return;
  }

  int8_t max_items = 0;

  switch (APP_STATE)
  {
  case MainMenu:
    max_items = MAIN_MENU_ITEMS_COUNT - 1;
    break;
  case Extras:
    max_items = EXTRAS_ITEMS_COUNT - 1;
    break;
  case Games:
    max_items = GAMES_ITEMS_COUNT - 1;
    break;
  case Settings:
    max_items = SETTINGS_ITEMS_COUNT - 1;
    break;

  default:
    break;
  }

  static int8_t prev_btn_up = HIGH;
  static int8_t prev_btn_down = HIGH;
  static int8_t prev_btn_left = HIGH;
  static int8_t prev_btn_right = HIGH;
  static int8_t prev_btn_accept = HIGH;
  static int8_t prev_btn_cancel = HIGH;

  int8_t cur_btn_up = digitalRead(BTN_UP);
  int8_t cur_btn_down = digitalRead(BTN_DOWN);
  int8_t cur_btn_left = digitalRead(BTN_LEFT);
  int8_t cur_btn_right = digitalRead(BTN_RIGHT);
  int8_t cur_btn_accept = digitalRead(BTN_ACCEPT);
  int8_t cur_btn_cancel = digitalRead(BTN_CANCEL);

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
  else if (cur_btn_accept == LOW && prev_btn_accept == HIGH)
  {
    switch (APP_STATE)
    {
    case MainMenu:
      switch (current_item)
      {
      case 0:
        APP_STATE = Extras;
        current_item = 0;
        break;
      case 1:
        APP_STATE = Games;
        current_item = 0;
        break;
      case 2:
        APP_STATE = Settings;
        current_item = 0;
        break;
      }
      break;

    case Settings:
      switch (current_item)
      {
      case 4:
        deepsleep();
        break;
      }
      break;

    default:
      break;
    }
  }
  else if (cur_btn_cancel == LOW && prev_btn_cancel == HIGH)
  {
    switch (APP_STATE)
    {
    case Extras:
      APP_STATE = MainMenu;
      current_item = 0;
      break;
    case Games:
      APP_STATE = MainMenu;
      current_item = 0;
      break;
    case Settings:
      APP_STATE = MainMenu;
      current_item = 0;
      break;

    default:
      break;
    }
  }

  delay(20);

  prev_btn_up = cur_btn_up;
  prev_btn_down = cur_btn_down;
  prev_btn_left = cur_btn_left;
  prev_btn_right = cur_btn_right;
  prev_btn_accept = cur_btn_accept;
  prev_btn_cancel = cur_btn_cancel;
}

void menuDraw(const char **name, int8_t itemsDraw)
{
  u8g2.clearBuffer();

  statusBarDraw();

  const int offset_y = current_item * 128;

  for (int8_t i = 0; i < itemsDraw; i++)
  {
    int item_y = i * 128 - offset_y;

    u8g2.drawRFrame((128 - 54) / 2, item_y + 10, 54, 36, 5);

    u8g2.setFont(u8g2_font_tenthinguys_tr);

    int8_t textWidth = u8g2.getUTF8Width(name[i]);
    int8_t x = (128 - textWidth) / 2;

    u8g2.drawStr(x, item_y + 60, name[i]);
    item_y += 128;
  }

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

void drawAllMenus()
{
  switch (APP_STATE)
  {
  case MainMenu:
    menuDraw(MAIN_MENU_ITEMS, MAIN_MENU_ITEMS_COUNT);
    break;

  case Extras:
    menuDraw(EXTRAS_ITEMS, EXTRAS_ITEMS_COUNT);
    break;

  case Games:
    menuDraw(GAMES_ITEMS, GAMES_ITEMS_COUNT);
    break;

  case Settings:
    menuDraw(SETTINGS_ITEMS, SETTINGS_ITEMS_COUNT);
    break;

  default:
    break;
  }
}

void setup()
{
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_ACCEPT, INPUT_PULLUP);
  pinMode(BTN_CANCEL, INPUT_PULLUP);

  init();
}

void loop()
{
  drawAllMenus();
  updateClock();
  input();
}

// compile = arduino-cli compile -b esp32:esp32:esp32c3 -u -p (YOUR COM-PORT - arduino-cli board list)
