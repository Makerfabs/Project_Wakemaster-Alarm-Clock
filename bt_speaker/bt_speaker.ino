#include <btAudio.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//SSD1306
#define MAKEPYTHON_ESP32_SDA 4
#define MAKEPYTHON_ESP32_SCL 5
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Sets the name of the audio device
btAudio audio = btAudio("ESP_Speaker");

void setup()
{

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
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE); 
    display.setCursor(0, 0); // Start at top-left corner
    display.println("ESP BT");
    display.println("SPEAKER");
    display.display();

    // streams audio data to the ESP32
    audio.begin();

    //Already change to Makepython Audio
    int bck = 26;
    int ws = 25;
    int dout = 27;
    audio.I2S(bck, dout, ws);
}

void loop()
{
}
