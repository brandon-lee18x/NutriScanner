## Project Description
As college students, maintaining a healthy lifestyle is challenging since many of us are busy with classes, social activities, lack nutritional awareness, and the process of tracking daily caloric and nutritional intake is often too tedious to track. As a result, we find ourselves, like many college students making suboptimal decisions about meals leading to health issues and nutritional deficiency. 

To solve this problem, we developed the NutriScanner! Our solution is a user friendly nutrition scanner involving  a weight scale, barcode reader, and an LED display. Our solution instantly provides the user precise nutritional information for the exact amount of food they are consuming. By using API calls to gather nutritional information from the barcode scanner, we are one step closer to solving an important problem plaguing college students and empowering them to make healthier choices. 

## System Requirements

System
* Any OS system
* Arduino IDE
* python interpeter (we used VS code)


## Setup / Installation Steps
1. Install the [Arduino IDE](https://www.arduino.cc/en/software) version appropriate for your computer
2. Install the HX711_ADC library by uploading [this](https://github.com/olkal/HX711_ADC/tree/master) zip file to your arduino IDE
3. Install the ILI9341 library in your arduino IDE
4. Download the ReceiverArduinoDisplay.ino and NutriScanner.ino sketches from this repo

## Execution (how to run it)

### Hardware:
- Connect load cell/HX711 and barcode scanner to arduino uno wifi rev2 using ADC and UART pins respectively
- Connect LCD display to arduino uno rev3 via SPI
- To connect LCD display, pins have to be powered w/ 3.3. V, so a resistor for each wire is needed to act as a voltage divider
- Connect arduino uno wifi rev2 to arduino uno r3 via I2C
- Connect both arduinos to machine (ex. laptop) via USB

### Software:
- Run flask server in terminal by typing in
```
python nutriscanner_server.py
```
- Run ReceiverArduinoDisplay.ino sketch in arduino IDE
- In another arduino IDE tab, run NutriScanner.ino sketch

### Technologies
- HX711 Digital Load Cell Weight Sensor and HX711 ADC amplifier ([product link](https://www.amazon.com/gp/product/B09K7G3477/ref=ox_sc_act_title_1?smid=A27MCP768Z76HQ&th=1))
- Maikrt Embedded QR Code Scanning Module USB ([product link](https://www.amazon.com/gp/product/B07GVMKPQT/ref=ewc_pr_img_3?smid=AZACVJ0NR9HB&psc=1))
- Arduino Uno
- Arduino Uno WiFi Rev2
- ILI9341 TFT LCD Display

## Resources 
[Barcode Scanner](https://how2electronics.com/barcode-qr-code-reader-using-arduino-qr-scanner-module/)

[Load Cell](https://www.youtube.com/watch?v=sxzoAGf1kOo&t=2s&ab_channel=Indrek)

[3D Printed Parts](https://www.thingiverse.com/thing:4602226)
