/* LED Blink, Teensyduino Tutorial #1
   http://www.pjrc.com/teensy/tutorial.html
 
   This example code is in the public domain.
*/

#include <Adafruit_NeoPixel.h>
#include <OctoWS2811.h>

#define DISABLE_INPUT_SWITCHES

/**
 * Pin Declerations
 * 
 * NOTE: Pin 2 on the Teensy is used for driving the LEDs inside the mirror. 
 * The Octolibrary hard-codes using this pin, so it does not need to be
 * declared here.
 */
 
const int switchInputPin = 11;
const int enclosureNeopixelsPin = 9;
const int ledPin = 13;

/**
 * Misc Variables
 */
 
static int switch0State;
unsigned long last_light_mode_change = 0;  // initially high to prevent trigger on 1st run
typedef enum {LIGHT_MODE_RED = 0, LIGHT_MODE_ORANGE, LIGHT_MODE_GREEN, LIGHT_MODE_COUNT} LightMode;
static int lightMode = LIGHT_MODE_RED;


/**
 * Neopixel Setup
 */
 
const int enclosureNeopixelsCount = 1;
Adafruit_NeoPixel enclosure_pixels = Adafruit_NeoPixel(enclosureNeopixelsCount, enclosureNeopixelsPin, NEO_RGB + NEO_KHZ800);

/**
 * Octo Library Setup
 */

#define LEDS_ON_RING 12
#define LEDS_ON_ENCLOSURE_SINGLE_STRIP 141

const int ledsPerStrip = LEDS_ON_ENCLOSURE_SINGLE_STRIP;
const int ledsTotal = ledsPerStrip * 2;
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

  Serial.begin(9600);
//  Serial.setTimeout(2000);
  Serial.println("Starting...");

  // Initialize states
  switch0State = digitalRead(switchInputPin);

  delay(1000);
  bootUpLEDs();
  Serial.println("Boot-up complete!");
  
}

// the loop() methor runs over and over again,
// as long as the board has power

void loop() {

  static unsigned long lastBlink = 0;
  static boolean isBlinky = false;

  if ( millis() - lastBlink > 1000 ) {
    lastBlink = millis();
    digitalWrite(ledPin, isBlinky);
    isBlinky = !isBlinky;
    Serial.print("Blinky is now "); Serial.println(isBlinky);
  }


//  lightAnimationDimTest();
  lightAnimationDebug();
//  loopLightMode();

//  switch (lightMode) {
//    case LIGHT_MODE_RED:
//      lightAnimationRed();
//      
//    break;
//    default:
//      allColor(0x000000);
//    break;
//  }


  delay(10);


}

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

#define SPACING 48
#define SPEED 100


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

void lightAnimationRed() {
  
  unsigned long pos_int = (millis() / 45) % 255;
  float p = pos_int / 255.0f;
  byte key_index = ((millis() / SPEED) % SPACING);
  byte offkey_index = SPACING - key_index - 1;

//  Serial.print("Key Index: "); Serial.print(key_index); Serial.print(". Offkey Index: "); Serial.println(offkey_index);

  // Strip 0
  for (int i=0; i < ledsPerStrip; i++) {

    if ( i % SPACING == key_index ) {
      setPixelInStrip(i, Wheel((byte)pos_int), 0);
    } else {
      setPixelInStrip(i, 0x000000, 0);
    }
  }

  // Strip 1
  for (int i=0; i < ledsPerStrip; i++) {

    if ( i % SPACING == offkey_index ) {
      setPixelInStrip(i, Wheel((byte)pos_int), 1);
    } else {
      setPixelInStrip(i, 0x000000, 1);
    }
  }
  leds.show();
}

void setPixelInStrip(int pixel_id, uint32_t color, int strip_id) {
  if ( strip_id > 0 ) {
    pixel_id += ledsPerStrip;
  }
  if ( color ) {
    Serial.print("Strip "); Serial.print(strip_id); Serial.print("["); Serial.print(pixel_id); Serial.println("] has color");    
  }
  leds.setPixel(pixel_id, color);
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
  Serial.println(currentSwitchState);

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
    enclosure_pixels.setPixelColor(0, enclosure_pixels.Color(  0, 0, 0));
    
  }
  enclosure_pixels.show();
  
}

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

void logLightMode(int mode) {
//  switch(mode) {
//    case LIGHT_MODE_RED:
//  }
  Serial.print("Light mode is "); Serial.println(mode);
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
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

void allColor(unsigned int c) {
  for (int i=0; i < ledsTotal; i++) {
    leds.setPixel(i, c);
  }
  leds.show();
}

void bootUpLEDs() {
  for (int i=0; i < 10; i++) {
    for (int j=0; j < ledsTotal; j++) {
      if ( j >= i ) {
        leds.setPixel(i, 0x000400);
      } else {
        leds.setPixel(i, 0x000000);
      }
    }
    leds.show();
    delay(250);
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

