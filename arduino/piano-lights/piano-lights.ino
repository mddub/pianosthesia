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
#define NUM_RIPPLES 30
#define FRAME_DELAY 20
#define INITIAL_RIPPLE_TICKS 70
#define INITIAL_RIPPLE_VELOCITY 1.6
#define MINIMUM_RIPPLE_VELOCITY 0.05
#define RIPPLE_DECELERATION 0.88

static uint16_t ripple_start[NUM_RIPPLES];
static float ripple_velocity[NUM_RIPPLES];
static float ripple_distance[NUM_RIPPLES];
static uint8_t ripple_color[NUM_RIPPLES];
static uint8_t ripple_ticks[NUM_RIPPLES];

static bool led_on[255];
static uint8_t led_solid_color[255];

static uint8_t next_index = 0;
static unsigned long last_frame = 0;

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
      return strip.Color(9 - WheelPos * 0.1, 0, WheelPos * 0.1);
    }
    if(WheelPos < 170) {
      WheelPos -= 85;
      return strip.Color(0, WheelPos * 0.1, 9 - WheelPos * 0.1);
    }
    WheelPos -= 170;
    return strip.Color(WheelPos * 0.1, 9 - WheelPos * 0.1, 0);
  }
}

void step_ripple() {
  strip.clear();
  for(uint8_t i = 0; i < NUM_RIPPLES; i++) {
    if (ripple_ticks[i] != 0) {
      if (ripple_ticks[i] == INITIAL_RIPPLE_TICKS) { // first hit
        strip.setPixelColor(ripple_start[i], Wheel(ripple_color[i], true));
      } else {
        strip.setPixelColor((uint16_t)(ripple_start[i] + ripple_distance[i]), Wheel(ripple_color[i], false));
        strip.setPixelColor((uint16_t)(ripple_start[i] - ripple_distance[i]), Wheel(ripple_color[i], false));
      }

      ripple_distance[i] = ripple_distance[i] + ripple_velocity[i];
      ripple_velocity[i] = ripple_velocity[i] * RIPPLE_DECELERATION;
      if(ripple_velocity[i] < MINIMUM_RIPPLE_VELOCITY) {
        ripple_velocity[i] = MINIMUM_RIPPLE_VELOCITY;
      }
      ripple_ticks[i]--;
    }
  }
  for(uint8_t i = 0; i < 255; i++) {
    if(led_on[i]) {
      strip.setPixelColor(i, Wheel(led_solid_color[i], true));
    }
  }
  strip.show();
}

void setup() {
  last_frame = millis();
  Serial.begin(560800);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  /*
  reset_note(0, 1100);
  reset_note(1, 800);
  reset_note(2, 600);
  reset_note(3, 900);
  */
}

void reset_ripple(uint8_t i, uint16_t start_pos,  uint8_t color) {
  ripple_start[i] = start_pos;
  ripple_velocity[i] = INITIAL_RIPPLE_VELOCITY;
  ripple_distance[i] = 0;
  ripple_color[i] = color;
  ripple_ticks[i] = INITIAL_RIPPLE_TICKS;
}

void loop() {
  unsigned long now = millis();

  if ((now - last_frame) > FRAME_DELAY) {
    last_frame = now;
    step_ripple();
    Serial.write(1);
    while (!Serial.available()) {}
    uint8_t num_pairs = Serial.read();
    for(uint8_t i = 0; i < num_pairs; i++) {
      char inChar = Serial.read();
      char on = Serial.read();

      uint8_t start_led = (16094 - 149.0 * inChar) / 89.0;
      uint8_t color = (inChar % 12) * 21;

      if (on) {
        led_on[start_led] = true;
        led_solid_color[start_led] = color;
        reset_ripple(next_index, start_led, color);
        next_index = (next_index + 1) % NUM_RIPPLES;
      } else {
        led_on[start_led] = false;
      }
    }
  }
}
