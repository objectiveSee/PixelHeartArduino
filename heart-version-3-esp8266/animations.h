#ifndef DR_ANIMATION_H
#define DR_ANIMATION_H

#include <Adafruit_NeoPixel.h>

/**
 * Neopixel LED Definitions
 */
#define DR_USE_NEOPIXEL_RING

#ifdef DR_USE_NEOPIXEL_RING
#define NUM_STRIPS 1
#define LEDS_TOTAL 12
#define LEDS_PER_STRIP 12
#else 
#define NUM_STRIPS 2
#define LEDS_TOTAL 282
#define LEDS_PER_STRIP 141
#endif


void allColor(unsigned int c);
uint32_t color(byte r, byte g, byte b);
uint32_t dimColor(uint32_t c, float bright);
uint32_t Wheel(byte WheelPos);
void setPixelInStrip(int pixel_id, uint32_t color, int strip_id);
void lightAnimationRed();
void lightAnimationOrange();
void lightAnimationGreen();
void lightAnimationBlue();
void lightAnimationOff();

int distanceCircular(int x, int y, int total);

#endif

