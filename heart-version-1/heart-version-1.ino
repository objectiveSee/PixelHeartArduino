#include <Adafruit_CircuitPlayground.h>

static const float kBrightnessPercent = 1.0f;

// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define CLICKTHRESHHOLD 120

int green = 0;
int blue = 0;
#define COLOR_FADE 2


struct Color {
   uint8_t red;
   uint8_t green;
   uint8_t blue;
};

static struct Color ringColor;
static struct Color previousRingColor;
bool isCircling = true;
unsigned long previous_change = 0;

void setup() {
  ringColor.red = 255;  ringColor.green = 0;  ringColor.blue = 0;
  previousRingColor = ringColor;
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(kBrightnessPercent*255);

  Serial.begin(9600);

  // configure accelerometer "taps"
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!

  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive
  CircuitPlayground.setAccelTap(1, CLICKTHRESHHOLD);
  
  // have a procedure called when a tap is detected
  attachInterrupt(digitalPinToInterrupt(CPLAY_LIS3DH_INTERRUPT), tapTime, FALLING);

  for ( int i = 0; i < 10; i++ ) {
    for ( int j = 0; j < 10; j++ ) {
      if ( j <= i ) {
        CircuitPlayground.setPixelColor(j, ringColor.red, ringColor.green, ringColor.blue);        
      } else {
        CircuitPlayground.setPixelColor(j, 0, 0, 0);
      }
    }
    delay(100);
  }
  
}

void tapTime(void) {
  // do something :)
   Serial.print("Tap! ");
   Serial.println(millis()); // the time

  previousRingColor = ringColor;

  ringColor.red = random(255);
  ringColor.green = random(255);
  ringColor.blue = random(255);

  previous_change = millis();
}

void setAllPixelColors (uint8_t r, uint8_t g, uint8_t b) {
  for ( uint8_t i = 0; i < 10; i++ ) {
     CircuitPlayground.setPixelColor(i, r, g, b);
  }
}

static const int beatDuration = 1250;

void loop() {
  CircuitPlayground.clearPixels();

  unsigned long now = millis();

  float brightness = (now%beatDuration)/((float)beatDuration);
  
  if ( brightness > 0.5 ) {
    brightness = 1-brightness;
  }
//  brightness = brightness * 0.95 + 0.05;  // add min brightness 
  Serial.println(brightness);
  

//  Serial.print(now%beatDuration);
//  Serial.print(": ");
//  Serial.println(brightness);

  if ( 1  ) {
    setAllPixelColors(ringColor.red * brightness,ringColor.green * brightness,ringColor.blue*brightness);
  } else {
    byte key = (now / 100) % 20;
    if ( key >= 10 ) {
      key = 20 - key;
    }
    for ( int i = 0; i < 10; i++ ) {
      if ( key <= i ) {
        CircuitPlayground.setPixelColor(i, ringColor.red, ringColor.green, ringColor.blue);
      } else {
          CircuitPlayground.setPixelColor(i, previousRingColor.red, previousRingColor.green, previousRingColor.blue);
      }
    }
  }

  delay(4);

}
