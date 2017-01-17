#include <CapacitiveSensor.h>
#include "FastLED.h"

#define CAPACITIVE_SEND 22
#define CAPACITIVE_RECEIVE 23
#define CAPACITIVE_SAMPLES 30
#define CAPACITIVE_THRESHOLD 5000
#define LONG_PRESS_MS 500

#define DEBUG false

#define FPS 60
#define DATA_PIN    2
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    36
CRGB leds[NUM_LEDS];

CapacitiveSensor cs = CapacitiveSensor(CAPACITIVE_SEND, CAPACITIVE_RECEIVE);
// millis() time when the current press started, or 0 if not pressed.
unsigned long pressedStart = 0;
// true if the current press has already been reported as long.
bool longRegistered = false;
// if true, there's a long press that needs handling.
bool longPressed = false;
// if true, there's a short press that needs handling.
bool shortPressed = false;
long recentCS = 0;

byte mode = 0;
bool newMode = false;

void setup() {
  if (DEBUG) {
    Serial.begin(9600);
    Serial.println("HELLO WORLD!");
  }
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.setMaxPowerInVoltsAndMilliamps(5,2000); 
  FastLED.setBrightness(63);

  // Initialize the capacitive environment for a while.
  recentCS = 0;
  int i;
  for (i = 0; i < 20; i++) {
    recentCS += cs.capacitiveSensor(CAPACITIVE_SAMPLES);
    delay(10);
  }
  recentCS /= i;
}

void loop() {
  if (newMode) {
    mode++;
  }
  if (mode == 0) {
    fill_solid(leds, NUM_LEDS, CHSV(50, 180, 255));   
  } else if (mode == 1) {
    fill_rainbow(leds, NUM_LEDS, millis() >> 4, 2);
  //} else if (mode == 2) {
    // START HERE
  } else {
    mode = 0;
  }
  FastLED.delay(1000 / FPS);
  newMode = false;


  EVERY_N_MILLISECONDS(10) {
    long cap = cs.capacitiveSensor(CAPACITIVE_SAMPLES);
    // Are we pressed right now?
    bool pressed = cap / recentCS > 5;// = cap > CAPACITIVE_THRESHOLD;
    if (DEBUG) { Serial.print(cap); Serial.print(" "); Serial.print(recentCS); Serial.print(" "); Serial.print(cap / recentCS); Serial.print(pressed ? "Y " : "N "); }
    if (!pressed) {
      recentCS = (9*recentCS + cap) / 10;
    }
    /*if (pressedStart) {
      // We were pressed before, so we're looking for a decline in the capacitive value.
      pressed = recentCS / cap > 3;
    } else {
      pressed = cap / recentCS > 3;
    }*/
    if (DEBUG) { Serial.print(pressed); Serial.print(" "); }
    // If we're pressed and this is the start, remember start time.
    if (pressed && !pressedStart) {
      pressedStart = millis();
      if (DEBUG) { Serial.print("PS "); }
    } else if (pressedStart) {
      // We're already pressed. Have we passed the LONG_PRESS_MS threshold?
      if (millis() - pressedStart > LONG_PRESS_MS) {
        // Yes, but have we already told someone about the long press for this
        // general press?
        if (!longRegistered) {
          // Nope: report the long press and remember that this press has already
          // generated the longPressed event.
          // Handler for this triggered below to keep code separate.
          longPressed = true;
          longRegistered = true;
        }
      } else if (!pressed) {
        // Current press is ending and it isn't long, so it's short.
        // Handler for this triggered below to keep code separate.
        shortPressed = true;
      }
    }
    if (!pressed) {
      pressedStart = 0;
      longRegistered = false;
    }

    // Keep this separate from the logic above, but it could actually go right in
    // there to avoid the use of shortPressed.
    if (shortPressed) {
      if (DEBUG) { Serial.print("short........................."); }
      byte bright = FastLED.getBrightness();
      bright = bright ? (bright >= 31 ? bright >> 1 : 0) : 255;
      //Serial.println(bright);
      FastLED.setBrightness(bright);
      shortPressed = false;
    }
    // Similarly, keep this logic separate
    if (longPressed) {
      if (DEBUG) { Serial.print("LONG!!!!!!!!!!!!!!!!!!!!!!"); }
      newMode = true;
      longPressed = false;
    }

    if (DEBUG) { Serial.println(); }
  }
}



