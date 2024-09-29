# ESPotify-Car-Thing
A version of the Spotify car thing made to run on ESP32 with a TFT display.

# Requirements
An ESP32  
A TFT display (with touch, the code is made for 240x320 with an ILI9341, other sizes will need some code change, untested with other drivers)  
Cables/Breadboard

# How to build
### Connect your display
Follow the diagram below

![image](https://github.com/user-attachments/assets/d2dfb49e-cb4e-4731-9803-5296487179ba)

(image taken from https://forum.arduino.cc/t/esp32-spi-tft-touch-display-touchscreen-not-providing-raw-values/1155801)

### Download the required libraries
TFT_eSPI  
TJpg_Decoder  
Arduino_JSON  

### Setup the TFT_eSPI library
1: Open the "User_Setup.h" file found in the library's folder  
2: Un-comment the line with the driver of your screen  
3: Define interface pins  

> [!NOTE]
> TFT_MISO 19  
> TFT_MOSI 23  
TFT_SCLK 18  
TFT_CS   15  
TFT_DC    2  
TFT_RST   4  
TOUCH_CS 21  
TOUCH_CLK 18  
TOUCH_DIN 23  
TOUCH_DO 19

### Add GitHub files
1: Paste the "SpotfiyCustom" folder into \Arduino\libraries  
2: Paste the "ESPotify" folder into \Arduino

### Upload UI images
> [!WARNING]
> Only works on an Arduino IDE below 2.0

1: Download the ESP32FS tool from here: https://github.com/me-no-dev/arduino-esp32fs-plugin  
2: Open the sketch with the IDE  
3: got to Tools > ESP32 Sketch Data Upload

# To do list
- [ ] Make sure the temporary token updates properly
- [ ] Add a way to connect to other networks (without having to re-upload the code to the ESP)
- [ ] Match playstate/shuffle/loop/volume when changing it from the web-player/app
- [ ] Find the cause of the crashes that can happen when searching/opening info page (probably a memory leak)
- [ ] Login with Username+Pass instead of cookie (avoids it resetting every once in a while) (?)
