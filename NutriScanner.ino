/*
   Below is the main driver code for the NutriScanner. It handles the taring, weighing, scanning, and display.
   
   Portions of this code were refernced from the tutorial here: https://www.youtube.com/watch?v=sxzoAGf1kOo&ab_channel=Indrek
   Credit: Olav Kallhovd, Arduino library for HX711 24-Bit Analog-to-Digital Converter for Weight Scales
*/

#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#include <Wire.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

#define OLED_RESET -1
Adafruit_SH1106 display(OLED_RESET); /* Object of class Adafruit_SSD1306 */

// wifi credentials
char ssid[] = "iPhone (7)";
char password[] = "55oiqcmw49bk5";

// 172.20.10.4
IPAddress server(172, 20, 10, 4);
int port = 80; // Change this to your server port

WiFiClient wifi;
HttpClient client = HttpClient(wifi, server, port);

//pins:
const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5; //mcu > HX711 sck pin
const int TARE_BUTTON = 2; //set the button to input pin 2
const int DISPLAY_BUTTON = 3;

int prevTarState = 0;
int currTarState = 0;
int prevDisplayState = 0;
int currDisplayState = 0;

//nutrition database init
struct nutritionInfo {
  String barcode;
  float servingSizeInG;
  int calories;
  float fat;
  float carbohydrates;
  float protein;
};

const int FOOD_FACTS_SIZE = 1;
nutritionInfo foodFacts[FOOD_FACTS_SIZE];
void initializeNutritionDB();
void sendHeaderFlag();
void sendBodyFlag();
void sendData(String data);
void sendWeight(float data);
void connectToWiFi();
void makeHttpRequest(String barcode);

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;

//state machine states
enum states {SCAN, WEIGH, DISPLAYNUTRITION};
states state = SCAN;

void setup() {
  Serial.begin(57600); delay(10);
  Serial1.begin(9600);
  Serial.println();
  // sendHeaderFlag();
  // sendData("Starting...");
  Serial.println("Starting...");

  pinMode(TARE_BUTTON, INPUT);
  pinMode(DISPLAY_BUTTON, INPUT);
  
  initializeNutritionDB();

  display.begin(SH1106_SWITCHCAPVCC, 0x3C); /* Initialize display with address 0x3C */

  LoadCell.begin();
  LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  float calibrationValue; // calibration value (see example file "Calibration.ino")
  calibrationValue = 696.0; // uncomment this if you want to set the calibration value in the sketch
#if defined(ESP8266)|| defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266/ESP32 and want to fetch the calibration value from eeprom
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    // sendHeaderFlag();
    // sendData("Startup is complete");
    Serial.println("Startup is complete");
  }
  Wire.begin();
  connectToWiFi();
  // WiFi.config(server);
}

String barcode = "";
boolean hasDrawn = false;
double weight = 0.0;
boolean hasConfirmed = false;

void loop() {
  switch (state) {
    case SCAN:
      if (!hasDrawn) {
        sendData("Scan Item");
        drawInitState("Scan Item");
        Serial.println("Scan Item");
        hasDrawn = true;
      }
      if (Serial1.available()) { // Check if there is Incoming Data in the Serial1 Buffer (UART peripheral)
      while (Serial1.available()) // Keep reading Byte by Byte from the Buffer till the Buffer is empty
      {
        char input = Serial1.read(); // Read 1 Byte of data from Serial1 (UART peripheral)
        //Serial.print(input); // Print the Byte
        barcode += input;
        //makeHttpRequest();
        delay(5); // A small delay
      }
      if (barcode != "") state = WEIGH;
      // if (lookupNutritionInfo(barcode)) state = WEIGH;
      
      hasDrawn = false; //set next state to not yet drawn
      }
      break;
    case WEIGH:
      if (!hasDrawn) {
        sendData("Weigh Item");
        drawInitState("Weigh Item");
        Serial.println("Weigh Item");
        hasDrawn = true;
      }
      static boolean newDataReady = 0;
      const int serialPrintInterval = 1000; //increase value to slow down serial print activity

      // check for new data/start next conversion:
      if (LoadCell.update()) newDataReady = true;

      // get smoothed value from the dataset:
      if (newDataReady) {
        if (millis() > t + serialPrintInterval) {
          weight = LoadCell.getData();
          sendData(String(weight));
          // Print to serial monitor
          Serial.print("Load_cell output val: ");
          Serial.println(weight);
          // Update only the region where the weight is displayed on the OLED
          display.fillRect(16, 32, 128, 32, BLACK); // Clear the region
          display.setTextSize(2);
          display.setCursor(16, 32);
          display.print(weight); // Display the updated weight value
          display.display();
          newDataReady = 0;
          t = millis();
        }
      }

      prevTarState = currTarState;
      currTarState = digitalRead(TARE_BUTTON);
      if (prevTarState == LOW && currTarState == HIGH) {
        Serial.println("Tare button pressed");
        LoadCell.tareNoDelay();
        delay(100);
      }

      //prevDisplayState = currDisplayState;
      currDisplayState = digitalRead(DISPLAY_BUTTON);
      if (prevDisplayState == LOW && currDisplayState == HIGH) {
        hasConfirmed = true;
        sendData("Calculating Nutrition");
        Serial.println("calculating nutrition");
        calculateNutrition(weight);
        hasDrawn = false;
        delay(100);
      }

      // check if last tare operation is complete:
      if (LoadCell.getTareStatus() == true) {
        Serial.println("Tare complete");
      }

      if (hasConfirmed) state = DISPLAYNUTRITION;

      delay(50);
      break;
    case DISPLAYNUTRITION:
      Serial.println("hi");
      break;
    default:
      Serial.println("hi");
      break;
  }
}

void drawInitState(String header) {
  display.clearDisplay(); /* Clear display */
  display.setTextColor(WHITE);
  display.setTextSize(2); /* Select font size of text. Increases with size of argument. */
  display.setCursor(16, 6);
  display.println("Scan Item");
  display.display();
  delay(500);
}

void initializeNutritionDB() {
  nutritionInfo info;
  info.barcode = "00259668";
  info.servingSizeInG = 28;
  info.calories = 70;
  info.fat = 4;
  info.carbohydrates = 2;
  info.protein = 6;
  foodFacts[0] = info;
}

double servingSizeInG;
double calories;
double fat;
double carbohydrates;
double protein;

bool lookupNutritionInfo(String barcode) {
  for (int i = 0; i < FOOD_FACTS_SIZE; i++) {
    nutritionInfo foodItem = foodFacts[i];
    if (foodItem.barcode == barcode) {
      servingSizeInG = foodItem.servingSizeInG;
      calories = foodItem.calories;
      fat = foodItem.fat;
      carbohydrates = foodItem.carbohydrates;
      protein = foodItem.protein;
      return true;
    }
  }
  Serial.println("barcode not found. Try again.");
  return false;
}

void calculateNutrition(double weight) {
  // For debugging
  // double calcCals = ratio * calories;
  // double calcFat = ratio * fat;
  // double calcCarbs = ratio * carbohydrates;
  // double calcProtein = ratio * protein;
  // String calories = "Calories: " + String(calcCals);
  // String fat = "Fat: " + String(calcFat);
  // String carbs = "Carbs: " + String(calcCarbs);
  // String protein = "Protein: " + String(calcProtein);
  // sendData(calories);
  // sendData(fat);
  // sendData(carbs);
  // sendData(protein);
  // delay(3000);
  Serial.println("Making HTTP GET request...");

  int connectStatus = client.connect(server, port);
  
  Serial.println(connectStatus);

  if (!connectStatus) {
    Serial.println("Connection to api failed!");
    return;
  }

  client.get("http://172.20.10.4/api/nutrition-info?barcode=" + barcode);

  // Read and print the response
  int statusCode = client.responseStatusCode();
  while (client.headerAvailable()) {
    Serial.print("header: ");
    Serial.print(client.readHeaderName());
    Serial.print(": ");
    Serial.println(client.readHeaderValue());
  }

  String response = client.readString();
  
  Serial.print("HTTP Status Code: ");
  Serial.println(statusCode);
  Serial.print("Response Length: ");
  Serial.println(response.length());
  Serial.print("Response: ");
  Serial.println(response);
  barcode = "";

  // Parse the JSON response
  StaticJsonDocument<2000> doc;  // Adjust the size based on your JSON object size
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print(F("Error parsing JSON: "));
    Serial.println(error.c_str());
  } else {
    // Access JSON values like doc["key"]
    // Serial.println(doc["calories"].as<String>());
    // Serial.println(doc["fat"].as<String>());
    // Serial.println(doc["carbs"].as<String>());
    // Serial.println(doc["protein"].as<String>());
    // Serial.println(doc["serving_quantity"].as<String>());
    // double calcCals = doc["calories"];
    // double calcFat = doc["fat"];
    // double calcCarbs = doc["carbs"];
    // double calcProtein = doc["protein"];
    // String calories = "Calories: " + String(calcCals);
    // String fat = "Fat: " + String(calcFat);
    // String carbs = "Carbs: " + String(calcCarbs);
    // String protein = "Protein: " + String(calcProtein);
    // sendData(calories);
    // sendData(fat);
    // sendData(carbs);
    // sendData(protein);
    String calories, fat, carbs, protein;
    double ratio;
    for (int i = 0; i < doc.size(); i++) {
    JsonObject obj = doc[i];
    if (obj.containsKey("serving_quantity")) {
      double serving_quantity = obj["serving_quantity"];
      ratio = weight / serving_quantity;
    }
    if (obj.containsKey("calories")) {
      double calcCals = obj["calories"];
      calcCals *= ratio;
      calories = "Calories: " + String(calcCals);
      Serial.println(calories);
    } if (obj.containsKey("fat")) {
      double calcFat = obj["fat"];
      calcFat *= ratio;
      fat = "Fat: " + String(calcFat);
      Serial.println(fat);
    } if (obj.containsKey("carbs")) {
      double calcCarbs = obj["carbs"];
      calcCarbs *= ratio;
      carbs = "Carbs: " + String(calcCarbs);
      Serial.println(carbs);
    } if (obj.containsKey("protein")) {
      double calcProtein = obj["protein"];
      calcProtein *= ratio;
      protein = "Protein: " + String(calcProtein);
      Serial.println(protein);
    }
    double ratio = weight / servingSizeInG;
    sendData(calories);
    sendData(fat);
    sendData(carbs);
    sendData(protein);
  }
  }
}

//sends header flag to receiver arduino
void sendHeaderFlag() {
  Wire.beginTransmission(9);
  Wire.write("Header");
  Wire.endTransmission();
  delay(100);
}

//sends body flag to receiver arduino
void sendBodyFlag() {
  Wire.beginTransmission(9);
  Wire.write("Body");
  Wire.endTransmission();
  delay(100);
}

//sends content data to receiver arduino
void sendData(String data) {
  Wire.beginTransmission(9);
  for (char c : data) {
    Wire.write(c);
  }
  Wire.endTransmission();
  delay(10);
}

void sendWeight(float data) {
  Wire.beginTransmission(9);
  byte* byteArray = (byte*)(&data);
  for (int i = 0; i < sizeof(float); i++) {
    Wire.write(byteArray[i]);
  }
  Wire.endTransmission();
  delay(10);
}

void connectToWiFi() {
  // Connect to Wi-Fi network
  Serial.print("Connecting to WiFi");
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

