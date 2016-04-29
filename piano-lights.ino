#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor acrossffff
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#define MIDDLE_LED 94
#define NUM_RIPPLES 40

static bool ripple_active[NUM_RIPPLES];
static uint16_t ripple_start[NUM_RIPPLES];
static uint16_t ripple_velocity[NUM_RIPPLES];
static uint16_t ripple_distance[NUM_RIPPLES];
static uint8_t ripple_color[NUM_RIPPLES];
static uint8_t ripple_ticks[NUM_RIPPLES];

static bool note_on[255];

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  /*
  reset_note(0, 1100);
  reset_note(1, 800);
  reset_note(2, 600);
  reset_note(3, 900);
  */
}

void reset_ripple(uint8_t i, uint16_t start_pos, uint16_t velocity, uint8_t color, uint8_t ticks_remaining) {
  strip.setPixelColor(ripple_start[i] / 10, 0);
  strip.setPixelColor((ripple_start[i] + ripple_distance[i]) / 10, 0);
  strip.setPixelColor((ripple_start[i] - ripple_distance[i]) / 10, 0);
  ripple_active[i] = true;
  ripple_start[i] = start_pos;
  ripple_velocity[i] = velocity;
  ripple_distance[i] = 0;  
  ripple_color[i] = color;
  ripple_ticks[i] = ticks_remaining;
}

static uint8_t next_index;

void loop() {
  if (Serial.available() > 0) {
    char inChar = Serial.read();
    uint8_t start_led = (93 + (60 - inChar) * 1.5);
    uint8_t color = (inChar % 12) * 21;
    if (inChar > 0) {
      bool on = Serial.read();
      if (on) {
        next_index = (next_index + 1) % NUM_RIPPLES;
        // TODO log scale or something - bass notes should be super slow
        uint16_t velocity = inChar >= 60 ? inChar / 5 : inChar / 4;
        reset_ripple(next_index, start_led * 10, velocity, color, 500 / velocity);
        note_on[start_led] = true;
      } else {
        note_on[start_led] = false;
        strip.setPixelColor(start_led, 0);
      }
    }
  }
  step_ripple(25);
}

void step_ripple(uint8_t wait) {
  for(uint8_t i = 0; i < NUM_RIPPLES; i++) {
    if (ripple_active[i]) {
      ripple_ticks[i]--;
      if (note_on[ripple_start[i] / 10]) {
        strip.setPixelColor(ripple_start[i] / 10, Wheel(ripple_color[i], true));
      } else {
        strip.setPixelColor(ripple_start[i] / 10, 0);
      }
      strip.setPixelColor((ripple_start[i] + ripple_distance[i]) / 10, Wheel(ripple_color[i], false));
      strip.setPixelColor((ripple_start[i] - ripple_distance[i]) / 10, Wheel(ripple_color[i], false));
    }
  }
  strip.show();
  delay(wait);
  for(uint8_t i = 0; i < NUM_RIPPLES; i++) {
    if (ripple_active[i]) {
      strip.setPixelColor((ripple_start[i] + ripple_distance[i]) / 10, 0);
      strip.setPixelColor((ripple_start[i] - ripple_distance[i]) / 10, 0);
      ripple_distance[i] = ripple_distance[i] + ripple_velocity[i];
      ripple_velocity[i] = (float)ripple_velocity[i] * 0.99999;
      if (ripple_velocity[i] < 1) {
        ripple_velocity[i] = 1;
      }
      // TODO this should be a max distance instead of max ticks,
      // or a min percentage of the original velocity
      if (ripple_ticks[i] == 0) {
        ripple_active[i] = false;
        // strip.setPixelColor(ripple_start[i] / 10, 0);
        // reset_note(i, 500 + random(100) * 10);
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos, bool bright) {
  if (bright) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
      return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if(WheelPos < 170) {
      WheelPos -= 85;
      return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
      return strip.Color(17 - WheelPos * 0.2, 0, WheelPos * 0.2);
    }
    if(WheelPos < 170) {
      WheelPos -= 85;
      return strip.Color(0, WheelPos * 0.2, 17 - WheelPos * 0.2);
    }
    WheelPos -= 170;
    return strip.Color(WheelPos * 0.2, 17 - WheelPos * 0.2, 0);
  }
}
