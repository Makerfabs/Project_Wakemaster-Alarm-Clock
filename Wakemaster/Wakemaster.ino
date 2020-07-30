#include "Arduino.h"
#include <Wire.h>
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"

//SD Card
#define SD_CS 22
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18

//Digital I/O used  //Makerfabs Audio V2.0
#define I2S_DOUT 27
#define I2S_BCLK 26
#define I2S_LRC 25

//SSD1306
#define MAKEPYTHON_ESP32_SDA 4
#define MAKEPYTHON_ESP32_SCL 5
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Button
const int Pin_vol_up = 39;
const int Pin_vol_down = 36;
const int Pin_mute = 35;

const int Pin_previous = 15;
const int Pin_pause = 33;
const int Pin_next = 2;

Audio audio;

//WIFI
const char *ssid = "Makerfabs";
const char *password = "20160704";

//NTP time
//const char* ntpServer = "pool.ntp.org";
const char *ntpServer = "120.25.108.11";
const long gmtOffset_sec = 8 * 60 * 60; //China+8
const int daylightOffset_sec = 0;
String clock_time = "18:01:00";
String clock_time2 = "18:03:00";

struct Music_info
{
    String name;
    int length;
    int runtime;
    int volume;
    int status;
    int mute_volume;
} music_info = {"", 0, 0, 0, 0, 0};

struct tm timeinfo;

void setup()
{
    //IO mode init
    pinMode(Pin_vol_up, INPUT_PULLUP);
    pinMode(Pin_vol_down, INPUT_PULLUP);
    pinMode(Pin_mute, INPUT_PULLUP);
    pinMode(Pin_previous, INPUT_PULLUP);
    pinMode(Pin_pause, INPUT_PULLUP);
    pinMode(Pin_next, INPUT_PULLUP);

    //Serial
    Serial.begin(115200);

    //LCD
    Wire.begin(MAKEPYTHON_ESP32_SDA, MAKEPYTHON_ESP32_SCL);
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3C for 128x32
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    display.clearDisplay();
    logoshow();

    //SD(SPI)
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    SPI.setFrequency(1000000);
    if (!SD.begin(SD_CS, SPI))
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

    //Audio(I2S)
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // 0...21
}

uint button_time = 0;
uint alarm_flag = 0;

void loop()
{
    printLocalTime();
    audio.loop();
    if (millis() - button_time > 600)
    {
        if (alarm_flag == 0)
        {
            if (showtime() != 0)
            {
                open_new_song("clock.wav");
                alarm_flag = 1;
                display.setCursor(0, 24); // Start at top-left corner
                display.println("ALARM!!!!!");
                display.display();
                delay(1000);
                button_time = millis();
            }
        }

        //Button logic
        if (digitalRead(Pin_mute) == 0)
        {
            Serial.println("Pin_mute");
            audio.stopSong();
            alarm_flag = 0;
            button_time = millis();
        }
    }
}

void open_new_song(String filename)
{
    //去掉文件名的根目录"/"和文件后缀".mp3",".wav"
    music_info.name = filename.substring(1, filename.indexOf("."));
    audio.connecttoFS(SD, filename);
    music_info.runtime = audio.getAudioCurrentTime();
    music_info.length = audio.getAudioFileDuration();
    music_info.volume = audio.getVolume();
    music_info.status = 1;
    Serial.println("**********start a new sound************");
}

void logoshow(void)
{
    display.clearDisplay();

    display.setTextSize(2);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.println(F("MakePython"));
    display.setCursor(0, 20); // Start at top-left corner
    display.println(F("WAKEMASTER"));
    display.display();
    delay(2000);
}

void lcd_text(String text)
{
    display.clearDisplay();

    display.setCursor(0, 0); // Start at top-left corner
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
    //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

int showtime()
{
    int hour = timeinfo.tm_hour;
    int min = timeinfo.tm_min;
    int sec = timeinfo.tm_sec;
    char time_str[10];
    sprintf(time_str, "%02d:%02d:%02d", hour, min, sec);
    display.clearDisplay();
    display.setCursor(0, 0); // Start at top-left corner
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

//**********************************************
// optional
void audio_info(const char *info)
{
    Serial.print("info        ");
    Serial.println(info);
}
void audio_id3data(const char *info)
{ //id3 metadata
    Serial.print("id3data     ");
    Serial.println(info);
}

//歌曲结束逻辑
void audio_eof_mp3(const char *info)
{ //end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
    open_new_song("clock.wav");
}
void audio_showstation(const char *info)
{
    Serial.print("station     ");
    Serial.println(info);
}
void audio_showstreaminfo(const char *info)
{
    Serial.print("streaminfo  ");
    Serial.println(info);
}
void audio_showstreamtitle(const char *info)
{
    Serial.print("streamtitle ");
    Serial.println(info);
}
void audio_bitrate(const char *info)
{
    Serial.print("bitrate     ");
    Serial.println(info);
}
void audio_commercial(const char *info)
{ //duration in sec
    Serial.print("commercial  ");
    Serial.println(info);
}
void audio_icyurl(const char *info)
{ //homepage
    Serial.print("icyurl      ");
    Serial.println(info);
}
void audio_lasthost(const char *info)
{ //stream URL played
    Serial.print("lasthost    ");
    Serial.println(info);
}
void audio_eof_speech(const char *info)
{
    Serial.print("eof_speech  ");
    Serial.println(info);
}