/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x64 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  analogReadResolution(10);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay(); // Clear display buffer
  // display.startscrollleft(0x00, 0x01);
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  display.setTextSize(1);

}

short i = 0, ah = 0, al = 1024, px = 0, py = 0, cy = 0, av = 0;
unsigned c = 0;
void loop() {  
  av = analogRead(2);
  if (ah < av) {ah = av;} else if (al > av) {al = av;}
  cy = (av-al)*64/(ah-al+1);
  // Serial.println(av);
  display.drawLine(px, py, i, cy, SSD1306_WHITE);
  display.setCursor(0,0);
  display.print(ah);
  display.setCursor(0, 55);
  display.print(al);
  display.setCursor(100,0);
  display.print(av);
  display.display();
  px = i, py = cy;
  if (i<128) { 
    i++;
  } else {
    i = 0;
    display.clearDisplay(); // Clear display buffer
    px = i;
    Serial.println((ESP.getCycleCount()-c)*25/1000000);
    c = ESP.getCycleCount();
  }
  // delay(1);  // delay in between reads for clear read from serial
}
