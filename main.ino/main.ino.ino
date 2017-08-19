/* LED Blink, Teensyduino Tutorial #1
   http://www.pjrc.com/teensy/tutorial.html
 
   This example code is in the public domain.
*/

#include <Adafruit_NeoPixel.h>
#include <OctoWS2811.h>

// Comment out line below to disable the input switches.
//#define DISABLE_INPUT_SWITCHES


#define TEST_ENCLOSURE_PIXELS

/**
 * Pin Declerations
 * 
 * NOTE: Pin 2 on the Teensy is used for driving the LEDs inside the mirror. 
 * The Octolibrary hard-codes using this pin, so it does not need to be
 * declared here.
 */

// input switch is 18 & 20
// pot in is 17
const int switchInputPin = 18;
const int enclosureNeopixelsPin = 15;
const int ledPin = 13;
const int potentiometerPin = 16;

/**
 * Misc Variables
 */
 
static int switch0State;
unsigned long last_light_mode_change = 0;  // initially high to prevent trigger on 1st run
typedef enum {LIGHT_MODE_RED = 0, LIGHT_MODE_ORANGE, LIGHT_MODE_GREEN, LIGHT_MODE_COUNT} LightMode;
static int lightMode = LIGHT_MODE_RED;

// Pin 13 blinking properties
#define BLINK_DURATION 100    // duration of blink when it's on
#define BLINK_INTERVAL 5000   // interval in ms that blinks happen at
#define BLINK_DURATION_DURING_SETUP 100    // duration of blink when it's on, during setup
#define BLINK_INTERVAL_DURING_SETUP 200   // interval in ms that blinks happen at, during setup


/**
 * Enclosure LED Setup
 */
 
const int enclosureNeopixelsCount = 1;    // Number of Neopixels built into the Teensy enclosure.
Adafruit_NeoPixel enclosure_pixels = Adafruit_NeoPixel(enclosureNeopixelsCount, enclosureNeopixelsPin, NEO_RGB + NEO_KHZ800);

/**
 * Octo Library Setup
 */

//#define LEDS_ON_RING 12
#define LEDS_ON_ENCLOSURE_SINGLE_STRIP 141
#define NUM_STRIPS 2

const int ledsPerStrip = LEDS_ON_ENCLOSURE_SINGLE_STRIP;
const int ledsTotal = ledsPerStrip * NUM_STRIPS;
DMAMEM int displayMemory[ledsTotal*6];
int drawingMemory[ledsTotal*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsTotal, displayMemory, drawingMemory, config);

/**
 * Main Code
 */
 
void setup() {

  pinMode(switchInputPin, INPUT_PULLUP);

  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);
  pinMode(enclosureNeopixelsPin, OUTPUT); // TODO: needed?

  leds.begin();
   
  enclosure_pixels.begin();
  enclosure_pixels.show(); // Initialize all pixels to 'off'

  allColor(0x000000);  // turn off light strip

  Serial.setTimeout(500);
  Serial.begin(9600);
  Serial.println("Starting...");

  // Initialize states
  switch0State = digitalRead(switchInputPin);
  Serial.print("Initial Switch State: "); Serial.println(switch0State); 

  delay(1000);
  bootUpLEDs();
  Serial.println("Boot-up complete!");
  
}

void loop() 
{

  // blink Teensy LED
  blinkPin13LED(BLINK_DURATION,BLINK_INTERVAL);

  // update light mode and check switch inputs for mode changes
  loopLightMode();

  lightAnimationRed();

  int sensorValue = analogRead(potentiometerPin);
  Serial.print("Sensor value is "); Serial.println(sensorValue);

//  switch (lightMode) {
//    
//    case LIGHT_MODE_RED:
//      lightAnimationRed();
//
//
//    case LIGHT_MODE_ORANGE:
//      lightAnimationOrange();
//      
//    break;
//    default:
//      Serial.print("Unknown light mode "); Serial.print(lightMode);
//      allColor(0x000000);
//    break;
//  }

  delay(10);
}

/****************************************************************************************
 *
 * Light Animations
 *
 ***************************************************************************************/

void lightAnimationDebug() {


  leds.setPixel(10, 0x00FF00);
  leds.setPixel(ledsPerStrip + 10, 0x00FF00);

  leds.setPixel(50, 0x00FF00);
  leds.setPixel(ledsPerStrip + 50, 0x00FF00);
  
  static int j = 0;
  for ( int i = 0; i < ledsPerStrip; i++ ) {
    if ( i == j ) {
      setPixelInStrip(i, 0x003300, 0);
      setPixelInStrip(ledsPerStrip - i - 1, 0x003300, 1);
    } else {
      setPixelInStrip(i, 0x000000, 0);
      setPixelInStrip(ledsPerStrip - i - 1, 0x000000, 1);
    }
  }
  Serial.print("J = "); Serial.println(j);
  j = (j+1)%ledsPerStrip;
  leds.show();
}

void lightAnimationDimTest() {
  for ( int i = 0; i < ledsTotal; i++ ) {

    if ( i % 2 == 0 ) {
      leds.setPixel(i, 0xFFFFFF);
    } else {
      leds.setPixel(i, 0x000000);
    }
    
  }
  leds.show();
  
}

#define ORANGE_ANIMATION_RATE 500
#define ORANGE_ANIMATION_SPACING 5


void lightAnimationOrange() {
  for ( int i = 0; i < ledsPerStrip; i+= ORANGE_ANIMATION_SPACING) {
    uint32_t top = 0;
    uint32_t bottom = 0;

    byte value = (millis() / ORANGE_ANIMATION_RATE) % 2;

    top = (value == 0) ? color(255,180,0) : 0;
    bottom = (value != 0) ? color(255,180,0) : 0;


    setPixelInStrip(i, top, 0);
    setPixelInStrip(i, bottom, 1);
    
  }
}


#define RED_ANIMATION_SPACING 3
#define RED_ANIMATION_SPEED 100

void lightAnimationRed() {
  
  unsigned long pos_int = (millis() / 45) % 255;
  float p = pos_int / 255.0f;
  byte key_index = ((millis() / RED_ANIMATION_SPEED) % RED_ANIMATION_SPACING);
  byte offkey_index = RED_ANIMATION_SPACING - key_index - 1;

//  Serial.print("Key Index: "); Serial.print(key_index); Serial.print(". Offkey Index: "); Serial.println(offkey_index);

  // Strip 0
  for (int i=0; i < ledsPerStrip; i++) {

    if ( i % RED_ANIMATION_SPACING == key_index ) {
      setPixelInStrip(i, Wheel((byte)pos_int), 0);
    } else {
      setPixelInStrip(i, 0x000000, 0);
    }
  }

  // Strip 1
  for (int i=0; i < ledsPerStrip; i++) {

    if ( i % RED_ANIMATION_SPACING == offkey_index ) {
      setPixelInStrip(i, Wheel((byte)pos_int), 1);
    } else {
      setPixelInStrip(i, 0x000000, 1);
    }
  }
  leds.show();
}

/****************************************************************************************
 *
 * Enclosure LED & Mode Management
 *
 ***************************************************************************************/

#define LIGHT_MODE_BLINK_DURATION 3000
#define MIN_TIME_BETWEEN_INPUT_CHANGES 100
#define SHOW_LIGHT_MODE_DURATION_ON_BOOTUP 5000

void loopLightMode() {

#ifdef DISABLE_INPUT_SWITCHES
  return;
#endif

  unsigned long time_now = millis();

  int currentSwitchState = digitalRead(switchInputPin);
//  Serial.print("Digital Input: "); Serial.println(currentSwitchState);

  if ( currentSwitchState != switch0State && time_now - last_light_mode_change > MIN_TIME_BETWEEN_INPUT_CHANGES ) {
    switch0State = currentSwitchState;
    Serial.print("Switch state changed. It is now ");   Serial.println(currentSwitchState  ? "HIGH" : "LOW");
    lightMode = ((lightMode+1) % LIGHT_MODE_COUNT);
    last_light_mode_change = time_now;
    logLightMode(lightMode);
  }

  if ( time_now < last_light_mode_change + LIGHT_MODE_BLINK_DURATION || time_now < SHOW_LIGHT_MODE_DURATION_ON_BOOTUP) {  
    
    // show mode for first SHOW_LIGHT_MODE_DURATION_ON_BOOTUP ms of boot-up or when the state changes
    enclosure_pixels.setPixelColor(0, colorForLightMode(lightMode)); 
       
  } else {

    // off
    enclosure_pixels.setPixelColor(0, enclosure_pixels.Color(0, 0, 0));
    
  }
  enclosure_pixels.show();
  
}

/**
 * Returns color to show corresponding to a mode.
 */
uint32_t colorForLightMode(int mode) {
  switch(mode) {
    case LIGHT_MODE_RED:
      return enclosure_pixels.Color(50,0,0);
      break;
    case LIGHT_MODE_ORANGE:
      return enclosure_pixels.Color(50,30,0);
      break;
    case LIGHT_MODE_GREEN:
      return enclosure_pixels.Color(0,50,0);
      break;   
    default:
      return enclosure_pixels.Color(0xFF,0xFA,0xCD);
  }
}

/**
 * Prints the mode.
 */
void logLightMode(int mode) {
  Serial.print("Light mode is "); Serial.println(mode);
}


/****************************************************************************************
 *
 * Helper Functions and other misc. stuff
 *
 ***************************************************************************************/


void setPixelInStrip(int pixel_id, uint32_t color, int strip_id) {
  if ( strip_id > 0 ) {
    pixel_id += ledsPerStrip;
  }
//  if ( color ) {
//    Serial.print("Strip "); Serial.print(strip_id); Serial.print("["); Serial.print(pixel_id); Serial.println("] has color");    
//  }
  leds.setPixel(pixel_id, color);
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
        return enclosure_pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if(WheelPos < 170)
    {
        WheelPos -= 85;
        return enclosure_pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else
    {
        WheelPos -= 170;
        return enclosure_pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}

/*
 * Sets all pixels to a color.
 */
void allColor(unsigned int c) {
  for (int i=0; i < ledsTotal; i++) {
    leds.setPixel(i, c);
  }
  leds.show();
}

uint32_t color(byte r, byte g, byte b) {
  return enclosure_pixels.Color(r,g,b);
}

/**
 * Turns on some strip LEDs in a boot-up animation sequence.
 */
#define BOOTUP_WHEEL_RATE 20
#define BOOTUP_TIME 4000
void bootUpLEDs() {

  unsigned long boottup_start = millis();
  unsigned long time_now = boottup_start;


  while ( time_now - boottup_start < BOOTUP_TIME ) {

    unsigned long time_diff = time_now - boottup_start;
    byte pixel_on = time_diff / 1000;
    
    for (int j=0; j < ledsTotal; j++) {
      if ( j <= pixel_on ) {
        leds.setPixel(j, 0x000400);
      } else {
        leds.setPixel(j, 0x000000);
      }
    }
    leds.show();

    byte wheel_position = ((byte)(time_now / BOOTUP_WHEEL_RATE))%255;
    uint32_t color = Wheel(wheel_position);
    
    enclosure_pixels.setPixelColor(0, color);
    enclosure_pixels.show();
//    Serial.print("Boot up step "); 
//    Serial.print(wheel_position);
//    Serial.print(", ");
//    Serial.println(pixel_on);    

    time_now = millis();

    // blink LED at a different interval during bootup
    blinkPin13LED(BLINK_DURATION_DURING_SETUP,BLINK_INTERVAL_DURING_SETUP);

    delay(10);
  }
  

    
}

/**
 * Blinks the red/orange LED on the Teensy board at an interval.
 */
void blinkPin13LED(unsigned long duration, unsigned long blink_interval) {
  
  unsigned long t = millis();

  if ( t % blink_interval < duration ) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}


/**
 * //    float led_float_value = (float)i / (float)ledsPerStrip;
//    float difference = (led_float_value > p) ? (led_float_value - p) : (p - led_float_value);
//    if ( difference > 0.5 ) {
//      difference = 1-difference;
//    }
//    if ( i == 2 ) {
//      Serial.print(led_float_value);
//      Serial.print(" / ");
//      
//      Serial.print(pos_int);
//      Serial.print(" / ");
//      
//      Serial.print(p);
//      Serial.print(" / ");
//      Serial.println(difference);
//    }

//    if ( difference < 0.05f ) {
//    if ( i < 5 || i > ledsPerStrip - 5 ) {
//if ( 1 ) {

*/

