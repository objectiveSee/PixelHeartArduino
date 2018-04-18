#ifndef DR_ANIMATION_H
#define DR_ANIMATION_H

#include <Adafruit_NeoPixel.h>


void allColor(unsigned int c);
uint32_t color(byte r, byte g, byte b);
uint32_t dimColor(uint32_t c, float bright);
uint32_t Wheel(byte WheelPos);
void setPixelInStrip(int pixel_id, uint32_t color, int strip_id);
void lightAnimationRed();
void lightAnimationOrange();
void lightAnimationGreen();
void lightAnimationBlue();
void lightAnimationRainbow();
void lightAnimationOff();


int distanceCircular(int x, int y, int total);

#endif

