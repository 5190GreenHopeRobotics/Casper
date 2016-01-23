#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define NUMPIXELS 60
namespace light {
Adafruit_NeoPixel pixels;
enum color {
  RED = 1, GREEN = 2, ORANGE = 3, BLACK = 4
};

void burnLedSetup(const int pin) {
  pixels = Adafruit_NeoPixel(NUMPIXELS, pin, NEO_GRB + NEO_KHZ800);
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.show();

}
void burnLedColor(const int ledNum, const color colorWanted) {
  if (colorWanted == RED) {
    pixels.setPixelColor(ledNum, pixels.Color(255, 0, 0));
  }
  if (colorWanted == GREEN) {
    pixels.setPixelColor(ledNum, pixels.Color(0, 255, 0));
  }
  if (colorWanted == ORANGE) {
    pixels.setPixelColor(ledNum, pixels.Color(255, 165, 0));
  }
  if (colorWanted == BLACK) {
    pixels.setPixelColor(ledNum, pixels.Color(0, 0, 0));
  }
  pixels.show();
}

}

bool receivedFirst = false;
unsigned long lastReceived = 0;

// handler for receiving data
void recieveData(int num) {
  lastReceived = millis();
  // roborio should send 5 bytes at a time
    //Serial.println("Receive 5");

    /*
       if received the first byte, set all led to rgb 0,0,0, and set the first led to green
    */
    if (!receivedFirst) {
      for (int i = 1; i < NUMPIXELS; i++) {
        light::pixels.setPixelColor(i, 0, 0, 0);
      }
      light::pixels.show();
      light::pixels.setPixelColor(0, 0, 128, 0);
      light::pixels.show();
      receivedFirst = true;
    }

    /*
       we will ignore the register byte because it does not do anything
    */
    unsigned char registerByte = Wire.read();

    if (registerByte == 0) {
      /*
         read the id, r, g, and b
      */
      unsigned char id = Wire.read();
      unsigned char r = Wire.read();
      unsigned char g = Wire.read();
      unsigned char b = Wire.read();

      light::pixels.setPixelColor(id, r, g, b);
    } else if (registerByte == 1) {
      // Show a range of colors
      unsigned char idFrom = Wire.read();
      unsigned char idTo = Wire.read();
      unsigned char r = Wire.read();
      unsigned char g = Wire.read();
      unsigned char b = Wire.read();
    }
    light::pixels.show();
}


//set up light, i2c and init neo pixel values
void setup() {
  // setup neopixel
  light::burnLedSetup(13);

  //init i2c
  Wire.begin(84);
  Wire.onReceive(recieveData);

  //init default lights
  for (int i = 1; i < NUMPIXELS; i++) {
    light::pixels.setPixelColor(i, 25, 64, 25);
  }
  light::pixels.setPixelColor(0, 64, 0, 0, 0);
  light::pixels.show();
}

// event based so do nothing
void loop() {
  static unsigned long time1, time2;
  static unsigned int steps = 0;
  if (millis() - lastReceived > 10000) {
    for (int i = 1; i < NUMPIXELS; i++) {
      light::pixels.setPixelColor(i, 25, 64, 25);
    }
    light::pixels.setPixelColor(0, 64, 0, 0, 0);
    light::pixels.show();
  }
  if (!steps) {
    time1 = millis();
    steps++;
  } else {
    time2 = millis();
    if (time2 > time1) {
      if (time2 - time1 > 5000) {
        // Every five
        Wire.begin(84);
        Wire.onReceive(recieveData);
      }
    } else {
      time1 = millis();
    }
  }
}
