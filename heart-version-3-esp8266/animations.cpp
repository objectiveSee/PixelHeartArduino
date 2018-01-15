#include "animations.h"
#include <Adafruit_NeoPixel.h>

extern Adafruit_NeoPixel strip;
extern int potentiometer_value;

// used in Sparkle animation
static byte sparkleBuffer[100];

void allColor(unsigned int c) {
  for (int i=0; i < LEDS_TOTAL; i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

uint32_t color(byte r, byte g, byte b) {
  return strip.Color(r,g,b);
}

uint32_t dimColor(uint32_t c, float bright) {
  byte r,g,b;

  r = (c >> 16) && 0xFF;
  g = (c >> 8) && 0xFF;
  b = (c >> 0) && 0xFF;

  return color((byte)(r * bright), (byte)(g * bright), (byte)(b * bright));
}

#define RED_ANIMATION_SPACING 3
#define RED_ANIMATION_SPEED 100

void lightAnimationRed() {
  
  unsigned long pos_int = (millis() / 45) % 255;
  float p = pos_int / 255.0f;
  byte key_index = ((millis() / RED_ANIMATION_SPEED ) % RED_ANIMATION_SPACING);
  byte offkey_index = RED_ANIMATION_SPACING - key_index - 1;

//  Serial.print("Key Index: "); Serial.print(key_index); Serial.print(". Offkey Index: "); Serial.println(offkey_index);

  // Strip 0
  for (int i=0; i < LEDS_PER_STRIP; i++) {

    if ( i % RED_ANIMATION_SPACING == key_index ) {
      setPixelInStrip(i, Wheel((byte)pos_int), 0);
    } else {
      setPixelInStrip(i, color(0,0,0), 0);
    }
  }

  if ( NUM_STRIPS > 1 ) {
    // Strip 1
    for (int i=0; i < LEDS_PER_STRIP; i++) {
  
      if ( i % RED_ANIMATION_SPACING == offkey_index ) {
        setPixelInStrip(i, Wheel((byte)pos_int), 1);
      } else {
        setPixelInStrip(i, color(0,0,0), 1);
      }
    }
  }
  strip.show();
}

void lightAnimationOrange() {

  float wheel_percent = potentiometer_value / 1023.0f;
  byte wheel_position  = wheel_percent * 255;
  uint32_t c = Wheel(wheel_position);
  
  for ( int i = 0; i < LEDS_PER_STRIP; i++ ) {

    if ( i % 2 == 0 ) {
      setPixelInStrip(i, c, 0);
      setPixelInStrip(i, 0, 1);    
    } else if ( i % 2 == 1 ) {
      setPixelInStrip(i, 0, 0);
      setPixelInStrip(i, c, 1); 
    } 
  }
  strip.show();
}


/**
 * Measured current is 0.62 amps (Saturday @ 3:44pm)
 */
#define GREEN_WIDTH 12
void lightAnimationGreen() {

  int target_pixel = (millis() / 40) % 47;
  uint32_t defaultcolor = color(0,0,10);
  
  for ( int i = 0; i < LEDS_PER_STRIP; i++ ) {

    int index_folded = i % 47;
    
    int distance_to_pixel = distanceCircular(index_folded, target_pixel, 47);

    if ( distance_to_pixel < GREEN_WIDTH ) {

      float percentage = (GREEN_WIDTH-distance_to_pixel)/((float) GREEN_WIDTH);
      percentage = percentage;
      
      byte r = (byte)(255.0f * percentage);
      byte g = (byte)(255.0f * percentage);
      uint32_t col1 = color( r, 0, 10);
      uint32_t col2 = color( 0, g, 10);
      setPixelInStrip(i, col1, 0);
      setPixelInStrip(i, col2, 1);
//      Serial.print(i); Serial.print(" "); Serial.println(r);
    
    } else {
      setPixelInStrip(i, defaultcolor, 0);
      setPixelInStrip(i, defaultcolor, 1);
    }

    
//    if ( i == 30 || i == LEDS_PER_STRIP-1 ) {
//      Serial.print("LED "); Serial.print(i); Serial.print("= "); Serial.println(distance_to_pixel);
//    }
  }
  strip.show();
}

const static int SPARKLE_BUFFER_RANDOM_PROB = LEDS_TOTAL * 10;

void lightAnimationSparkle() {

  float wheel_percent = potentiometer_value / 1023.0f;  // between 0 and 1

  int range = LEDS_TOTAL + (wheel_percent * LEDS_TOTAL * 10);

  
  int newPixel = random(range);
  for ( int i = 0; i < LEDS_TOTAL; i++ ) {
    if ( i == newPixel ) {
      sparkleBuffer[i] = 255;
    } else {
      if ( sparkleBuffer[i] < 5 ) {
        sparkleBuffer[i] = 0;
      } else {
        sparkleBuffer[i] = sparkleBuffer[i] - 4;
      }
    }
    strip.setPixelColor(i, color(sparkleBuffer[i],sparkleBuffer[i],sparkleBuffer[i]));
  }
  strip.show();
}
void lightAnimationBlue() {
  return lightAnimationSparkle();
}

void lightAnimationOff() {
  allColor(0x000000);
}

/****************************************************************************************
 *
 * Helper Functions and other misc. stuff
 *
 ***************************************************************************************/

int distanceCircular(int x, int y, int total) {

  int d= abs(x-y);

  if ( d < total / 2 ) {
    return d;
  } else {
    return total - d;
  }  
}

void setPixelInStrip(int pixel_id, uint32_t color, int strip_id) {
  if ( strip_id > 0 ) {
      pixel_id += LEDS_PER_STRIP; // assume 2 strips
    }
  strip.setPixelColor(pixel_id, color);
}

/**
 * Input a value 0 to 255 to get a color value.
 * The colours are a transition r - g - b - back to r.
 */
uint32_t Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85)
    {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if(WheelPos < 170)
    {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else
    {
        WheelPos -= 170;
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}
