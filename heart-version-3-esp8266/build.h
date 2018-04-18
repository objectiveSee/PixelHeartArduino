#ifndef BUILD_H
#define BUILD_H


// ONLY DEFINE ONE OF THESE!
//#define DR_ESP_01??
//#define TARGET_PIXEL_HEART
#define TARGET_LUNAR_MIRROR
//#define DR_USE_NEOPIXEL_RING??

/**
 * Define properties of our boards. The ESP-01 is very limited and can use as much GPIO so
 * some features must be disabled.
 */
#if defined(DR_ESP_01)
  #define NEOPIXEL_PIN 2
  #define NUM_STRIPS 1
  #define LEDS_TOTAL 12
  #define LEDS_PER_STRIP 12

#elif defined(TARGET_PIXEL_HEART)
  #define USE_ONBOARD_LED
  #define NEOPIXEL_PIN D8
  #define LEDS_TOTAL 282
  #define LEDS_PER_STRIP 141
  #define NUM_STRIPS 2

#elif defined(TARGET_LUNAR_MIRROR)
  #define USE_ONBOARD_LED
  #define NEOPIXEL_PIN D8
  #define LEDS_PER_STRIP 81
  #define LEDS_TOTAL LEDS_PER_STRIP
  #define NUM_STRIPS 1

#else
  #error Must define ESP type
#endif


#endif
