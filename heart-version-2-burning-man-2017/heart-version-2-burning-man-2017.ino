/* LED Blink, Teensyduino Tutorial #1
   http://www.pjrc.com/teensy/tutorial.html
 
   This example code is in the public domain.
*/

#include <Adafruit_NeoPixel.h>
#include <OctoWS2811.h>

// Comment out line below to disable the input switches.
//#define DISABLE_INPUT_SWITCHES


//#define USE_DEBUG_ANIMATION


/**
 * Pin Declerations
 * 
 * NOTE: Pin 2 on the Teensy is used for driving the LEDs inside the mirror. 
 * The Octolibrary hard-codes using this pin, so it does not need to be
 * declared here.
 */

const int switch0InputPin = 18;
const int switch1InputPin = 9;
const int potentiometerPin = 16;
const int enclosureNeopixelsPin = 15;
const int ledPin = 13;

/**
 * Misc Variables
 */

// Number of LEDs and strip properties
//#define LEDS_ON_RING 12
#define LEDS_ON_ENCLOSURE_SINGLE_STRIP 141
#define NUM_STRIPS 2
const int ledsPerStrip = LEDS_ON_ENCLOSURE_SINGLE_STRIP;
const int ledsTotal = ledsPerStrip * NUM_STRIPS;

// Switches (input)
static int switch0State, switch1State;
unsigned long last_light_mode_change = 0;  // initially high to prevent trigger on 1st run
unsigned long last_switch_1_change = 0;  // initially high to prevent trigger on 1st run

/** 
 * Light modes as enum. 
 * NOTE: LIGHT_MODE_OFF is after the LIGHT_MODE_COUNT so it isn't cycled through.
 */
typedef enum {LIGHT_MODE_RED = 0, LIGHT_MODE_ORANGE, LIGHT_MODE_GREEN, LIGHT_MODE_BLUE, LIGHT_MODE_COUNT, LIGHT_MODE_OFF} LightMode;
#define DEFAULT_LIGHT_MODE LIGHT_MODE_RED
static int lightMode = DEFAULT_LIGHT_MODE;

// Pin 13 blinking properties
#define BLINK_DURATION 100    // duration of blink when it's on
#define BLINK_INTERVAL 5000   // interval in ms that blinks happen at
#define BLINK_DURATION_DURING_SETUP 100    // duration of blink when it's on, during setup
#define BLINK_INTERVAL_DURING_SETUP 200   // interval in ms that blinks happen at, during setup

#define SENSOR_BUFFER_LENGTH 4
static int sensor_readings[SENSOR_BUFFER_LENGTH];

// used in Sparkle animation
static byte sparkleBuffer[ledsTotal];


/**
 * Enclosure LED Setup
 */
 
const int enclosureNeopixelsCount = 1;    // Number of Neopixels built into the Teensy enclosure.
Adafruit_NeoPixel enclosure_pixels = Adafruit_NeoPixel(enclosureNeopixelsCount, enclosureNeopixelsPin, NEO_RGB + NEO_KHZ800);
static unsigned long last_enclosure_led_change = 0;
uint32_t enclosureColor = 0;


/**
 * Octo Library Setup
 */

DMAMEM int displayMemory[ledsTotal*6];
int drawingMemory[ledsTotal*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsTotal, displayMemory, drawingMemory, config);

/**
 * Main Code
 */
 
void setup() {

  pinMode(switch0InputPin, INPUT_PULLUP);
  pinMode(switch1InputPin, INPUT_PULLUP);
  
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
  switch0State = digitalRead(switch0InputPin);
  Serial.print("Initial Switch 0 State: "); Serial.println(switch0State); 
  switch1State = digitalRead(switch1InputPin);
  Serial.print("Initial Switch 1 State: "); Serial.println(switch1State); 

  // initalize sensor buffer
  int sensor_initial_value = analogRead(potentiometerPin);
  for ( int i = 0; i < SENSOR_BUFFER_LENGTH; i++ ) {
    sensor_readings[i] = sensor_initial_value;
  }

  // clear sparkle buffer
  for (int i = 0; i < ledsTotal; i++ ) {
    sparkleBuffer[i] = 0;
  }

  delay(1000);
  bootUpLEDs();
  Serial.println("Boot-up complete!");
  
}

static int potentiometer_value; // 0 to 123 inclusive

void updatePotentiometerValue() {
  
  int sensorValue = analogRead(potentiometerPin);
  static int sensor_reading_index = 0;

  sensor_reading_index = (sensor_reading_index+1) % SENSOR_BUFFER_LENGTH;

  sensor_readings[sensor_reading_index] = sensorValue;

  int t = 0;
  for ( byte i = 0; i < SENSOR_BUFFER_LENGTH; i++ ) {
    t += sensor_readings[i];
  }
  t /= SENSOR_BUFFER_LENGTH;

  t = min(1023, t);   // restrain to 0-1023
  t = max(0, t);
  
  potentiometer_value = t;
  

//  Serial.print("Sensor value is "); Serial.println(t);  
}

void loop() 
{

  // blink Teensy LED
  blinkPin13LED(BLINK_DURATION,BLINK_INTERVAL);

  // update light mode and check switch inputs for mode changes
  loopLightMode();

  updatePotentiometerValue();

  static int last_light_mode_log = -1;
  if ( lightMode != last_light_mode_log ) {
    logLightMode(lightMode);
    last_light_mode_log = lightMode;
  }

  #ifdef USE_DEBUG_ANIMATION
//  Serial.println("Using debug animation");
  lightAnimationDebug();
  delay(10);
  return;
#endif


  switch (lightMode) {
    
    case LIGHT_MODE_RED:
      lightAnimationRed();
      break;


    case LIGHT_MODE_ORANGE:
      lightAnimationOrange();
      break;

    case LIGHT_MODE_GREEN:
      lightAnimationGreen();
      break;

    case LIGHT_MODE_BLUE:
      lightAnimationBlue();
      break;

    case LIGHT_MODE_OFF:
      lightAnimationOff();
      break;
    
    break;
    default:
      Serial.print("Unknown light mode "); Serial.println(lightMode);
      allColor(0x000000);
    break;
  }

  delay(10);
}

/****************************************************************************************
 *
 * Light Animations
 *
 ***************************************************************************************/

uint32_t dimColor(uint32_t c, float bright) {
  byte r,g,b;

  r = (c >> 16) && 0xFF;
  g = (c >> 8) && 0xFF;
  b = (c >> 0) && 0xFF;

  return color((byte)(r * bright), (byte)(g * bright), (byte)(b * bright));
}

/**
 * Blue animation
 */

#define NUM_BLUE_ANIMATION_COLORS 7
uint32_t blueAnimationColors[NUM_BLUE_ANIMATION_COLORS] = {0xFF0020, 0x8B20BB, 0xF0F100,0xFF7E02, 0xAEC8EF, 0xC0F4FF, 0x003399};

void lightAnimationUnfinished() {
  // 141 lights total
  // 6 per pattern
  int color_index = 0;

  int t = millis() / 1000;
  byte strip_on = (t%2);
  
  for ( byte j=0; j<2; j++ ) {
    
    color_index = 0;
    
    for ( int i = 0; i < ledsPerStrip; i+= 6 ) {
   
      uint32_t color = (j==strip_on) ? blueAnimationColors[color_index] : 0;    
      setPixelInStrip(i  , 0, j);
      setPixelInStrip(i+1, 0, j);
      setPixelInStrip(i+2, dimColor(color, 30), j);
      setPixelInStrip(i+3, color, j);
      setPixelInStrip(i+4, color, j);
      setPixelInStrip(i+5, dimColor(color, 30), j);
    }
    color_index = (color_index+1)%(byte)NUM_BLUE_ANIMATION_COLORS;
  }
  
  leds.show();

  
}

const static int SPARKLE_BUFFER_RANDOM_PROB = ledsTotal * 10;

void lightAnimationBlue() {
  return lightAnimationSparkle();
}

void lightAnimationSparkle() {

  float wheel_percent = potentiometer_value / 1023.0f;  // between 0 and 1

  int range = ledsTotal + (wheel_percent * ledsTotal * 10);

  
  int newPixel = random(range);
  for ( int i = 0; i < ledsTotal; i++ ) {
    if ( i == newPixel ) {
      sparkleBuffer[i] = 255;
    } else {
      if ( sparkleBuffer[i] < 5 ) {
        sparkleBuffer[i] = 0;
      } else {
        sparkleBuffer[i] = sparkleBuffer[i] - 4;
      }
    }
    leds.setPixel(i, color(sparkleBuffer[i],sparkleBuffer[i],sparkleBuffer[i]));
  }
  leds.show();
}


/**
 * Animation that can be powered by USB from MacBook
 */
void lightAnimationDebug() {


  // 1 pixel on at a time

/**
  static int j = 0;
  for ( int i = 0; i < ledsTotal; i++ ) {
    if ( i == j ) {
      leds.setPixel(i, color(0x33,0x33,0x33));
    } else {
      leds.setPixel(i, color(0,0,0));
    }
  }
  Serial.print("J = "); Serial.println(j);
  j = (j+1)%ledsTotal;
  leds.show();
  */

/**
  static int j = 0;
  for ( int i = 0; i < ledsPerStrip; i++ ) {
    if ( i == j ) {
      setPixelInStrip(i, color(0x33,0x33,0x33), 0);
      setPixelInStrip(i, color(0x33,0x33,0x33), 1);
    } else {
      setPixelInStrip(i, color(0,0,0), 0);
      setPixelInStrip(i, color(0,0,0), 1);
    }
  }
  Serial.print("J = "); Serial.println(j);
  j = (j+1)%ledsPerStrip;
  leds.show();
  */

/*
  const int kIntervalDebugShow = 3;
  static int j = 0;
  for ( int i = 0; i < ledsPerStrip; i++ ) {
    if ( i % kIntervalDebugShow == j ) {
      setPixelInStrip(i, color(0x33,0x33,0x33), 0);
      setPixelInStrip(i, color(0x33,0x33,0x33), 1);
    } else {
      setPixelInStrip(i, color(0,0,0), 0);
      setPixelInStrip(i, color(0,0,0), 1);
    }
  }
  j = (j+1)%kIntervalDebugShow;
  Serial.print("J = "); Serial.println(j);
  leds.show();
  delay(40);
  */

//  lightAnimationGreen();

}

void lightAnimationDimTest() {

//
//  int target_pixel = 40;
//
//  for ( int i = 0; i < ledsPerStrip; i++ ) {
//    int distance_to_pixel = target_pixel - i;
//    if ( distance_to_pixel > ledsPerStrip / 2 ) {
//      distance_to_pixel = ledsPerStrip - distance_to_pixel;
//    }
//    if ( i == 20 || i == 120 ) {
//      Serial.print("LED "); Serial.print(i); Serial.print("= "); Serial.println(distance_to_pixel);
//    }
//  }
 
  
}


void lightAnimationOrange() {

  float wheel_percent = potentiometer_value / 1023.0f;
  byte wheel_position  = wheel_percent * 255;
  uint32_t c = Wheel(wheel_position);
  
  for ( int i = 0; i < ledsPerStrip; i++ ) {

    if ( i % 2 == 0 ) {
      setPixelInStrip(i, c, 0);
      setPixelInStrip(i, 0, 1);    
    } else if ( i % 2 == 1 ) {
      setPixelInStrip(i, 0, 0);
      setPixelInStrip(i, c, 1); 
    } 
//    else {
//      setPixelInStrip(i, 0, 0);
//      setPixelInStrip(i, 0, 1);
//    }

  }
  leds.show();

}

int distanceCircular(int x, int y, int total) {

  int d= abs(x-y);

  if ( d < total / 2 ) {
    return d;
  } else {
    return total - d;
  }  
}


/**
 * Measured current is 0.62 amps (Saturday @ 3:44pm)
 */
#define GREEN_WIDTH 12
void lightAnimationGreen() {

  int target_pixel = (millis() / 40) % 47;
  uint32_t defaultcolor = color(0,0,10);
  
  for ( int i = 0; i < ledsPerStrip; i++ ) {

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

    
//    if ( i == 30 || i == ledsPerStrip-1 ) {
//      Serial.print("LED "); Serial.print(i); Serial.print("= "); Serial.println(distance_to_pixel);
//    }
  }
  leds.show();
}

void lightAnimationOff() {
  allColor(0x000000);
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
  for (int i=0; i < ledsPerStrip; i++) {

    if ( i % RED_ANIMATION_SPACING == key_index ) {
      setPixelInStrip(i, Wheel((byte)pos_int), 0);
    } else {
      setPixelInStrip(i, color(0,0,0), 0);
    }
  }

  // Strip 1
  for (int i=0; i < ledsPerStrip; i++) {

    if ( i % RED_ANIMATION_SPACING == offkey_index ) {
      setPixelInStrip(i, Wheel((byte)pos_int), 1);
    } else {
      setPixelInStrip(i, color(0,0,0), 1);
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

#define SWITCH_0_POWER_ON 0

/**
 * Switch 0 is the mode switch
 * Switch 1 is TBD
 */
void loopLightMode() {

#ifdef DISABLE_INPUT_SWITCHES
  return;
#endif

  unsigned long time_now = millis();

  int currentSwitch0State = digitalRead(switch0InputPin);
  int currentSwitch1State = digitalRead(switch1InputPin);
  bool state_changed = false;
//  Serial.print("Digital Input: "); Serial.println(currentSwitchState);

  /*
   * Update the state of switch 0
   */
  if ( currentSwitch0State != switch0State && time_now - last_light_mode_change > MIN_TIME_BETWEEN_INPUT_CHANGES ) {
    switch0State = currentSwitch0State;
    Serial.print("Switch 0 state changed. It is now ");   Serial.println(currentSwitch0State  ? "HIGH" : "LOW");

    // only update light mode if the other switch is LOW (aka power is on)
    if ( switch1State == SWITCH_0_POWER_ON ) {
      lightMode = ((lightMode+1) % LIGHT_MODE_COUNT);
      last_light_mode_change = time_now;
      logLightMode(lightMode);
      state_changed = true;
    }
  }


  /*
   * Update the state of switch 1
   */
  if ( currentSwitch1State != switch1State && time_now - last_switch_1_change > MIN_TIME_BETWEEN_INPUT_CHANGES ) {
    switch1State = currentSwitch1State;
    Serial.print("Switch 1 state changed. It is now ");   Serial.println(currentSwitch1State  ? "HIGH" : "LOW");
    last_switch_1_change = time_now;

    static int saved_mode = DEFAULT_LIGHT_MODE;
    state_changed = true;

    if ( switch1State != SWITCH_0_POWER_ON ) {   // if HIGH turn off the lights, else turn them on
      // turn off
      saved_mode = lightMode;
      lightMode = LIGHT_MODE_OFF;
      last_light_mode_change = time_now;
      logLightMode(lightMode);

    } else {
      // turn lights on
      lightMode = saved_mode % LIGHT_MODE_COUNT;  // do mod() to make sure we restore to a good value.
      last_light_mode_change = time_now;
      logLightMode(lightMode);
    }
  }

  if ( state_changed ) {
    Serial.println("Setting enclosure color again.");
    setEnclosureColor(colorForLightMode(lightMode));
  }  

  /**
   * Handle enclosure LED state
   */
  
  if ( time_now < last_enclosure_led_change + LIGHT_MODE_BLINK_DURATION || time_now < SHOW_LIGHT_MODE_DURATION_ON_BOOTUP) {  
    
    // show mode for first SHOW_LIGHT_MODE_DURATION_ON_BOOTUP ms of boot-up or when the state changes
    enclosure_pixels.setPixelColor(0, enclosureColor); 
       
  } else {

    // off
    enclosure_pixels.setPixelColor(0, enclosure_pixels.Color(0, 0, 0));
    
  }
  enclosure_pixels.show();
  
}

void setEnclosureColor(uint32_t c) {
  enclosureColor = c;
  last_enclosure_led_change = millis();
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
    case LIGHT_MODE_BLUE:
      return enclosure_pixels.Color(0,0,5.0);
      break;
    case LIGHT_MODE_OFF:
      return enclosure_pixels.Color(0x11,0x11,0x11);
      break;  
    default:
      return enclosure_pixels.Color(0x88,0x88,0x88);
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
      pixel_id += ledsPerStrip; // assume 2 strips
    }
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

