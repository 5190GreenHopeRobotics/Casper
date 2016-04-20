#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel product5 w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(184, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code


  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Wire.begin(84);
  Wire.onReceive(recieveData);
}


bool receivedFirst = false;

// handler for receiving data
void recieveData(int num) {
  // roborio should send 5 bytes at a time
  if (num == 5) {
    //Serial.println("Receive 5");

    /*
       if received the first byte, set all led to rgb 0,0,0, and set the first led to green
    */
    if (!receivedFirst) {
      for (int i = 1; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, 0, 0, 0);
      }
      strip.show();
      strip.setPixelColor(0, 0, 128, 0);
      strip.show();
      receivedFirst = true;
    }

    /*
       we will ignore the register byte because it does not do anything
    */
    unsigned char registerByte = Wire.read();
    /*
       read the id, r, g, and b
    */
    unsigned char id = Wire.read();
    unsigned char r = Wire.read();
    unsigned char g = Wire.read();
    unsigned char b = Wire.read();

    strip.setPixelColor(id, r, g, b);
    strip.show();
  }
}

static int effect = 0;

void loop() {
  // Some example procedures showing how to display to the pixels:
  //colorWipe(strip.Color(255, 0, 0), 50); // Red
  //colorWipe(strip.Color(0, 255, 0), 50); // Green
  //colorWipe(strip.Color(0, 0, 255), 50); // Blue
  //colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
  // Send a theater pixel chase in...
  //theaterChase(strip.Color(127, 127, 127), 50); // White
  //theaterChase(strip.Color(127, 0, 0), 50); // Red
  //theaterChase(strip.Color(0, 0, 127), 50); // Blue

  //rainbow(20);
  if (effect == 0) {
    theaterChaseRainbow();
    delay(50);
  } else if (effect == 1) {
    colorWipe(strip.Color(0, 255, 0));
    delay(50);
  } else  if (effect == 2) {
    colorWipe(strip.Color(0, 0, 255));
    delay(50);
  } else  if (effect == 3) {
    theaterChase(strip.Color(127, 127, 127));
    delay(50);
  } else   if (effect == 4) {
    theaterChase(strip.Color(127, 0, 0));
    delay(50);
  } else   if (effect == 5) {
    theaterChase(strip.Color(0, 0, 127));
    delay(50);
  } else if (effect == 6) {
    rainbowCycle();
    delay(20);
  } else if (effect == 7) {
    rainbow();
    delay(20);
  } else if (effect == 8) {
    colorWipe(strip.Color(255, 0, 0));
    delay(50);
  } else if (effect == 9) {
    colorWipe(strip.Color(0, 0, 0));
    delay(50);
  } else {
    effect = 0;
  }
  test();

  static unsigned long time1, time2;
  static unsigned int steps = 0;

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


static int it = 0;
static int lit = 0;
static int tc = 0;

void test() {
  strip.setPixelColor(it, Wheel(tc));
  strip.setPixelColor(lit, 0, 0, 0);
  strip.show();
  lit = it;
  it++;
  if (it > 15) {
    it = 0;
  }
  tc++;
  if (tc > 254) {
    tc = 0;
  }
}

static int icw = 16;

// Fill the dots one after the other with a color
void colorWipe(uint32_t c) {
  strip.setPixelColor(icw, c);
  strip.show();

  icw++;
  if (icw > strip.numPixels()) {
    icw = 16;
    effect++;
  }
}

static int jr = 0;

void rainbow() {
  uint16_t i, j;

  for (i = 16; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((i + jr) & 255));
  }
  strip.show();
  jr++;
  if (jr > 256) {
    jr = 0;
    effect++;
  }
}

static int jrc = 0;

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
  uint16_t i, j;
  // 5 cycles of all colors on wheel
  for (i = 16; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + jrc) & 255));
  }
  strip.show();
  jrc++;
  if (jrc > 256 * 5) {
    jrc = 0;
    effect++;
  }
}

static int jtc = 0;
static int qtc = -184;

//Theatre-style crawling lights.
void theaterChase(uint32_t c) {

  for (uint16_t i = 16; i < strip.numPixels(); i = i + 3) {
    int p = qtc + i;
    if(p > 15){
      strip.setPixelColor(p, 0);      //turn every third pixel off
    }
  }
  qtc++;
  for (uint16_t i = 16; i < strip.numPixels(); i = i + 3) {
    int p = qtc + i;
    if(p > 15){
      strip.setPixelColor(p, c); //turn every third pixel on
    }
  }

  strip.show();

  if (qtc > 184) {
    jtc++;
    qtc = -184;
  }
  if (jtc > 10) {
    jtc = 0;
    effect++;
  }
}

static int jcr = 0;
static int qcr = 0;
static int ftc = 0;

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow() {
  if (ftc == 0) {
    ftc = 1;
    qcr = -strip.numPixels();
  }
  // cycle all 256 colors in the wheel
  for (uint16_t i = 16; i < strip.numPixels(); i = i + 3) {
    strip.setPixelColor(i + qcr, 0);      //turn every third pixel off
  }
  qcr++;
  for (uint16_t i = 16; i < strip.numPixels(); i = i + 3) {
    strip.setPixelColor(i + qcr, Wheel( (i + jcr) % 255)); //turn every third pixel on
  }
  strip.show();
  if (qcr > strip.numPixels()) {
    jcr++;
    ftc = 0;
    qcr = 0;
  }
  if (jcr > 2) {
    jcr = 0;
    effect++;
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
