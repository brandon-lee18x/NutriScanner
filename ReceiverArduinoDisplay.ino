// Receiver Arduino
#include <Wire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include  "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void receiveEvent();

void setup() {
  Wire.begin(9);  // Set the Arduino's I2C address
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_WHITE);
}

void loop() {

}

//0 = drawHeader, 1 = drawBody
int state = 0;
int centerWx;
int centerWy;
void receiveEvent() {
  String result = "";
  while (Wire.available()) {
    char data = Wire.read();
    result += data;
    delay(5);
  }
  //if result includes these items
  if (result.indexOf("Scan Item") != -1) {
    tft.fillScreen(ILI9341_WHITE);
    int centerX = calcCenterX(result, 3);
    tft.setCursor(40, 10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(3);
    tft.print(result);
    result = "";
  } else if (result.indexOf("Weigh Item") != -1) {
    tft.fillRect(40, 10, 320, 100, ILI9341_WHITE);
    int centerX = calcCenterX(result, 3);
    tft.setCursor(centerX, 10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(3);
    tft.println(result);
  } else if (result.indexOf("Calculating Nutrition") != -1) {
    String calculating = result.substring(0, result.indexOf(" "));
    String nutrition = result.substring(result.indexOf(" ") + 1);
    Serial.println(calculating);
    Serial.println(nutrition);
    tft.fillRect(0, 10, 220, 100, ILI9341_WHITE);
    tft.fillRect(0, centerWy, 220, 100, ILI9341_WHITE);
    int centerCalcX = calcCenterX(calculating, 3);
    int centerNutX = calcCenterX(nutrition, 3);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(centerCalcX, 10);
    tft.setTextSize(3);
    tft.println(calculating);
    tft.setCursor(centerNutX, 40);
    tft.println(nutrition);
  }
  else if (result.indexOf("Calories:") != -1) {
    tft.fillRect(0, 0, 220, 100, ILI9341_WHITE);
    tft.setCursor(10, 100);
    tft.setTextSize(2);
    tft.println(result);
  } else if (result.indexOf("Fat:") != -1) {
    result += "g";
    tft.setCursor(10, 140);
    tft.setTextSize(2);
    tft.println(result);
  } else if (result.indexOf("Carbs:") != -1) {
    result += "g";
    tft.setCursor(10, 180);
    tft.setTextSize(2);
    tft.println(result);
  } else if (result.indexOf("Protein:") != -1) {
    result += "g";
    tft.setCursor(10, 220);
    tft.setTextSize(2);
    tft.println(result);
  }
  else { //weight measurement
    centerWx = calcCenterX(result + " g", 3);
    centerWy = calcCenterY(result + " g", 3);
    tft.fillRect(0, centerWy, 220, 100, ILI9341_WHITE);
    tft.setCursor(centerWx, centerWy);
    tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
    tft.println(result + " g");
    result = result.substring(0, result.indexOf("m"));
  }
  // if (result == "Header") {
  //   state = 0;
  // }
  // else if (result == "Body") {
  //   state = 1;
  // }
  // else {
  //   tft.fillScreen(ILI9341_WHITE);
  //   switch (state) {
  //   case 0:
  //     tft.setCursor(10, 10);
  //     tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  //     tft.print(result);
  //     Serial.println("Received: " + result);
  //     break;
  //   case 1:
  //     tft.setCursor(10, 120);
  //     tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  //     tft.print(result);
  //     Serial.println("Received: " + result);
  //     break;
  //   default:
  //     break;
  //   }
  // }
   delay(10);
  
}

int calcCenterX(String result, int textSize) {
  int16_t x, y;
  uint16_t textWidth, textHeight;
  tft.getTextBounds(result, 0, 0, &x, &y, &textWidth, &textHeight);
  int16_t xPos = (tft.width() - textWidth) / 2;
  return xPos;
}

int calcCenterY(String result, int textSize) {
  int16_t x, y;
  uint16_t textWidth, textHeight;
  tft.getTextBounds(result, 0, 0, &x, &y, &textWidth, &textHeight);
  int16_t yPos = (tft.height() - textHeight) / 2;
  return yPos;
}
