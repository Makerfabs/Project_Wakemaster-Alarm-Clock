#include "I2S.h"
#include <FS.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <WiFi.h>
#include "time.h"
#include "esp32_config.h"

const int offset = 0x2C;
char data[800];
long i = 0;

const char *ssid = "Makerfabs";
const char *password = "20160704";

//const char* ntpServer = "pool.ntp.org";
const char *ntpServer = "120.25.108.11";
const long gmtOffset_sec = 8 * 60 * 60; //中国+8
const int daylightOffset_sec = 0;

String clock_time = "01:04";
String clock_time2 = "09:58";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
struct tm timeinfo;

void setup()
{
  pinMode(Pin_pause, INPUT_PULLUP);

  Serial.begin(115200);

  Wire.begin(MAKEPYTHON_ESP32_SDA, MAKEPYTHON_ESP32_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  lcd_text("Opening");

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");
  lcd_text("Wifi OK");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println(F("Alread get npt time."));
  printLocalTime();
  lcd_text("NPT FLESH");

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  SPI.begin(18, 19, 23, 5);
  if (!SD.begin(22, SPI))
  {
    Serial.println("Card Mount Failed");
    lcd_text("SD ERR");
    while (1)
      ;
  }
  else
  {
    lcd_text("SD OK");
  }

  I2S_Init();
  lcd_text("I2S OK");
}

void loop()
{
  printLocalTime();
  if (showtime() == 1)
  {
    while (1)
    {
      File file = SD.open("/clock.wav");
      file.seek(offset);
      if (play_music(file) == 1)
        break;
      printLocalTime();
      showtime();
    }
  }
  if (showtime() == 2)
  {
    while (1)
    {
      File file = SD.open("/clock.wav");
      file.seek(offset);
      if (play_music(file) == 1)
        break;
      printLocalTime();
      showtime();
    }
  }
  delay(1000);
}

void lcd_text(String text)
{
  display.clearDisplay();

  display.setTextSize(2);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner
  display.println(text);
  display.display();
  delay(500);
}

void printLocalTime()
{

  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

int showtime()
{
  int hour = timeinfo.tm_hour;
  int min = timeinfo.tm_min;
  int sec = timeinfo.tm_sec;
  char time_str[10];
  sprintf(time_str, "%02d:%02d:%02d", hour, min, sec);
  display.clearDisplay();

  display.setTextSize(2);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner
  display.println(time_str);
  display.display();
  String temp = (String)time_str;
  if (temp.startsWith(clock_time))
  {
    return 1;
  }
  if (temp.startsWith(clock_time2))
  {
    return 2;
  }

  return 0;
}

int play_music(File &file)
{
  while (file.readBytes(data, sizeof(data)))
  {
    if (digitalRead(Pin_pause) == 0)
    {
      Serial.println("Pin_pause");
      for (int i = 0; i < sizeof(data); ++i)
        data[i] = 0; // to prevent buzzing
      for (int i = 0; i < 5; ++i)
        I2S_Write(data, sizeof(data));

      file.close();
      delay(1000);
      return 1;
    }
    I2S_Write(data, sizeof(data));
  }
  file.close();
  for (int i = 0; i < sizeof(data); ++i)
    data[i] = 0; // to prevent buzzing
  for (int i = 0; i < 5; ++i)
    I2S_Write(data, sizeof(data));

  return 0;
}