/*
  This code is copyright 2018 Jonathan Thomson, jethomson.wordpress.com

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include <IRremote.h>
#include "UFO_LEDs_controller.h"
#include "ReAnimator.h"

#define SPEAKER_PIN 4

//#define IR_RECV_PIN 7
#define IR_RECV_PIN 12
#define PAUSE_FOR_IR_INTERVAL 2000

#define RIM_LEDS_DATA_PIN 2
#define BEAM_LEDS_DATA_PIN 10
#define HELM_LEDS_DATA_PIN 8
#define LED_STRIP_MIN_MILLIAMPS 25
#define LED_STRIP_INITIAL_MILLIAMPS 150
#define LED_STRIP_MAX_MILLIAMPS 400 // Don't draw more than 500 mA from the 5V pin of a Nano or the Schottky diode will burn up.
#define LED_STRIP_MILLIAMPS_STEP 25
#define FRAMES_PER_SECOND  120


const uint8_t PROGMEM HUE3_LUT[3] = {HUE_RED, HUE_ALIEN_GREEN, HUE_BLUE};
const uint8_t PROGMEM HUE16_LUT[16] = {HUE_RED, 16, HUE_ORANGE, 48, HUE_YELLOW, 80, HUE_GREEN, HUE_ALIEN_GREEN, HUE_AQUA, 144, HUE_BLUE, 176, HUE_PURPLE, 208, HUE_PINK, 240};

IRrecv irrecv(IR_RECV_PIN);
decode_results results;

CRGB rim_leds[NUM_RIM_LEDS];
CRGB beam_leds[NUM_BEAM_LEDS];
CRGB helm_leds[NUM_HELM_LEDS];

uint8_t gdynamic_hue = 0;
uint8_t gstatic_rim_hue = HUE_ALIEN_GREEN; // initialize to green since that is the color theme of the costume
uint8_t gstatic_beam_hue = HUE_ALIEN_GREEN;
uint8_t grandom_hue = 0;

uint8_t status_led_pin = 13;
uint8_t status_led_state = LOW;

enum Direction {DOWN = -1, NEUTRAL = 0, UP = 1};

//ReAnimator GlowSerum(rim_leds, &gstatic_hue, LED_STRIP_INITIAL_MILLIAMPS);
ReAnimator GlowSerum(rim_leds, beam_leds, helm_leds, &gdynamic_hue, &gstatic_beam_hue, LED_STRIP_INITIAL_MILLIAMPS);


void change_max_brightness(Direction direction) {

    static uint16_t selected_led_strip_milliamps = LED_STRIP_INITIAL_MILLIAMPS;

    if (direction == DOWN) {
        uint16_t new_led_strip_milliamps = selected_led_strip_milliamps - LED_STRIP_MILLIAMPS_STEP;
        if (new_led_strip_milliamps >= LED_STRIP_MIN_MILLIAMPS) {
            FastLED.setMaxPowerInVoltsAndMilliamps(LED_STRIP_VOLTAGE, new_led_strip_milliamps);
            selected_led_strip_milliamps = new_led_strip_milliamps;
            GlowSerum.set_selected_led_strip_milliamps(selected_led_strip_milliamps);
        }
    }
    else if (direction == NEUTRAL) {
        FastLED.setMaxPowerInVoltsAndMilliamps(LED_STRIP_VOLTAGE, LED_STRIP_INITIAL_MILLIAMPS);
        selected_led_strip_milliamps = LED_STRIP_INITIAL_MILLIAMPS;
        GlowSerum.set_selected_led_strip_milliamps(selected_led_strip_milliamps);

    }
    else if (direction == UP) {
        uint16_t new_led_strip_milliamps = selected_led_strip_milliamps + LED_STRIP_MILLIAMPS_STEP;
        if (new_led_strip_milliamps <= LED_STRIP_MAX_MILLIAMPS) {
            FastLED.setMaxPowerInVoltsAndMilliamps(LED_STRIP_VOLTAGE, new_led_strip_milliamps);
            selected_led_strip_milliamps = new_led_strip_milliamps;
            GlowSerum.set_selected_led_strip_milliamps(selected_led_strip_milliamps);
        }
    }

}

void change_sound_value_gain() {

    const uint8_t gains_size = 5;
    static uint8_t gains[gains_size] = {1, 2, 5, 10, 15};
    static uint8_t gi = 0;

    GlowSerum.set_sound_value_gain(gains[gi]);
    uint16_t start = (NUM_RIM_LEDS/2)-(gains[gi]/2);
    fill_solid(rim_leds, NUM_RIM_LEDS, CRGB::Black);
    for (uint8_t j = 0; j < gains[gi]; j++) {
        rim_leds[start+j] = CHSV(0, 255, 255);
    }
    FastLED.show();
    gi = (gi+1) % gains_size;
}


// EVERY_N_MILLISECONDS updates gdynamic_hue continuously
void select_dynamic_color() {

    static uint8_t h3i = 0;

    gdynamic_hue = pgm_read_word_near(HUE3_LUT + h3i);
    GlowSerum.set_selected_rim_hue(&gdynamic_hue);
    GlowSerum.set_selected_beam_hue(&gdynamic_hue);
    fill_solid(rim_leds, NUM_RIM_LEDS, CHSV(gdynamic_hue, 255, 255));
    fill_solid(beam_leds, NUM_BEAM_LEDS, CHSV(gdynamic_hue, 255, 255));
    h3i = (h3i+1) % 3;
}


// EVERY_N_MILLISECONDS() updates grandom_hue continuously
void select_random_rim_color() {

    grandom_hue = random8();
    GlowSerum.set_selected_rim_hue(&grandom_hue);

    //for (uint8_t i = 0; i < NUM_RIM_LEDS; i++) {
    //    rim_leds[i] = CHSV(random8(), 255, 255);
    //}
    fill_rainbow(rim_leds, NUM_RIM_LEDS, 0, 255/NUM_RIM_LEDS);
}


void select_static_rim_color() {

    static uint8_t h16i = 0;

    gstatic_rim_hue = pgm_read_word_near(HUE16_LUT + h16i);
    GlowSerum.set_selected_rim_hue(&gstatic_rim_hue);
    fill_solid(rim_leds, NUM_RIM_LEDS, CHSV(gstatic_rim_hue, 255, 255));
    h16i = (h16i+1) % 16;
}


void select_static_beam_color() {

    static uint8_t h16i = 0;

    gstatic_beam_hue = pgm_read_word_near(HUE16_LUT + h16i);
    GlowSerum.set_selected_beam_hue(&gstatic_beam_hue);
    fill_solid(beam_leds, NUM_BEAM_LEDS, CHSV(gstatic_beam_hue, 255, 255));
    h16i = (h16i+1) % 16;
}


void beep(int8_t beep_type) {

    if (beep_type < 0) {
        return;
    }

    static uint32_t dt = 0;
    static uint32_t pm = 0; // previous millis
    uint16_t interval = 1;
    uint8_t speaker_state = LOW;
    uint8_t cycle_count = 0;
    uint8_t total_cycles = 10;

    pinMode(SPEAKER_PIN, OUTPUT);

    if (beep_type == 1) {
      interval = 10;
    }

    while (cycle_count < total_cycles) {
        if ( (millis() - pm) > interval ) {
            pm = millis();

            speaker_state = !speaker_state;
            cycle_count = (speaker_state) ? cycle_count+1 : cycle_count;
            digitalWrite(SPEAKER_PIN, speaker_state);
        }
    }

    pinMode(SPEAKER_PIN, INPUT);
}


//void print_dt() {
//    static uint32_t pm = 0; // previous millis
//    Serial.print("dt: ");
//    Serial.println(millis() - pm);
//    pm = millis();
//}



void setup() {

    analogReference(EXTERNAL);

    pinMode(status_led_pin, OUTPUT);     

    Serial.begin(57600);
    irrecv.enableIRIn(); // Start the receiver

    FastLED.setMaxPowerInVoltsAndMilliamps(LED_STRIP_VOLTAGE, LED_STRIP_INITIAL_MILLIAMPS);
    FastLED.setCorrection(TypicalSMD5050);
    FastLED.addLeds<WS2812B, RIM_LEDS_DATA_PIN, GRB>(rim_leds, NUM_RIM_LEDS);
    FastLED.addLeds<WS2812B, BEAM_LEDS_DATA_PIN, GRB>(beam_leds, NUM_BEAM_LEDS);
    FastLED.addLeds<WS2812B, HELM_LEDS_DATA_PIN, GRB>(helm_leds, NUM_HELM_LEDS);

    beep(2);
}


void loop() {

    const static uint32_t pause_for_ir_interval = PAUSE_FOR_IR_INTERVAL;
    static uint32_t pause_for_ir_previous_millis = 0;
    static bool is_accepting_commands = false;
    static bool animations_paused = true;

    const uint8_t GBP_NUM = 4;
    const uint8_t BBP_NUM = 14;
    const Pattern green_button_patterns[GBP_NUM] = {SOUND_RIBBONS, SOUND_RIPPLE, SOUND_BLOCKS, SOUND_ORBIT};
    const Pattern blue_button_patterns[BBP_NUM] = {SOLID, JUGGLE, MITOSIS, BUBBLES, SPARKLE, SOLID, MATRIX,
                                                   WEAVE, STARSHIP_RACE, PAC_MAN, BALLS,
                                                   HALLOWEEN_FADE, HALLOWEEN_ORBIT,
                                                   DYNAMIC_RAINBOW};
    const Overlay blue_button_overlays[BBP_NUM] = {BREATH, NO_OVERLAY, NO_OVERLAY, NO_OVERLAY, NO_OVERLAY, FLICKER, NO_OVERLAY,
                                                   NO_OVERLAY, NO_OVERLAY, NO_OVERLAY, NO_OVERLAY,
                                                   NO_OVERLAY, NO_OVERLAY,
                                                   NO_OVERLAY};

    static uint8_t gbpi = 0;
    static uint8_t bbpi = 0;


    //while (!irrecv.isIdle()); // this might be faster than using if statement below. dt is about 3 ms for while, and about 4 ms for if

    if (irrecv.decode(&results)) {
        //DEBUG_PRINTHEX(results.value);
        digitalWrite(status_led_pin, !status_led_state);
        status_led_state = !status_led_state;

        if (is_accepting_commands) {
            static uint32_t previous_ir_code = 0x00000000; // remember previous code for repeat button presses
            static uint16_t button_held_count = 0;
            static uint32_t button_press_previous_millis = 0;
            static uint16_t button_released_interval = 300;
            uint32_t ir_code = results.value;
            uint8_t beep_type = 0;


            // 0xFFFFFFFF is the code for a button being held down
            if (ir_code == 0xFFFFFFFF) {
                if ((millis() - button_press_previous_millis) < button_released_interval) {
                    button_held_count++;
                    // often if a button is single pressed it will send 1 spurious held down code
                    if (button_held_count > 1) {
                        ir_code = previous_ir_code;
                    }
                    else {
                        ir_code = 0x00000000; // do nothing code
                    }
                }
                else {
                    button_held_count = 0;
                    ir_code = 0x00000000; // do nothing code
                }
            }
            else {
                button_held_count = 0;
            }
            button_press_previous_millis = millis();

            DEBUG_PRINT("ir_code is: ");
            DEBUG_PRINTHEX(ir_code);
            DEBUG_PRINTLN("");

            switch (ir_code) {
            //++++++++++ POWER BUTTON ++++++++++
                case 0xF7C03F: // Power
                    DEBUG_PRINTLN("Power Off");
                    is_accepting_commands = false;
                    previous_ir_code = 0x00000001;  // this ensures the held down code for the power button will do nothing
                    button_held_count = 0;
                    FastLED.clear();
                    FastLED.show();
                    break;
            //++++++++++ BRIGHTNESS BUTTONS ++++++++++
                case 0xF7807F: // Down Arrow
                    DEBUG_PRINTLN("Down");
                    previous_ir_code = ir_code;
                    animations_paused = true;  // pause animations to give time to pick a brightness level
                    pause_for_ir_previous_millis = millis();
                    change_max_brightness(DOWN);
                    break;
                case 0xF700FF: // Up Arrow
                    DEBUG_PRINTLN("Up");
                    previous_ir_code = ir_code;
                    animations_paused = true;
                    pause_for_ir_previous_millis = millis();
                    change_max_brightness(UP);
                    break;
                case 0xF740BF: // W/WW
                    DEBUG_PRINTLN("W/WW");
                    previous_ir_code = ir_code;
                    animations_paused = true;
                    pause_for_ir_previous_millis = millis();
                    change_max_brightness(NEUTRAL);
                    break;
            //++++++++++ COLOR SELECT BUTTONS ++++++++++
                case 0xF720DF: // IC Set:
                    DEBUG_PRINTLN("IC Set");
                    previous_ir_code = ir_code;
                    animations_paused = true; // pause animations to give time to pick a color
                    pause_for_ir_previous_millis = millis();
                    change_sound_value_gain();
                    break;
                case 0xF710EF: // CS: Show a rainbow. Selects a random color that changes periodically.
                    DEBUG_PRINTLN("CS");
                    previous_ir_code = ir_code;
                    animations_paused = true;
                    pause_for_ir_previous_millis = millis();
                    select_random_rim_color();
                    break;
                case 0xF730CF: // C3 (color select): Select a new dynamic color. The dynamic color evolves from the selected starting color.
                    DEBUG_PRINTLN("C3");
                    previous_ir_code = ir_code;
                    animations_paused = true;
                    pause_for_ir_previous_millis = millis();
                    select_dynamic_color();
                    break;
                case 0xF708F7: // C7 (color select): Select a new static rim color.
                    DEBUG_PRINTLN("C7");
                    previous_ir_code = ir_code;
                    animations_paused = true;
                    pause_for_ir_previous_millis = millis();
                    select_static_rim_color();
                    break;
                case 0xF728D7: // C16 (color select): Select a new static beam color.
                    DEBUG_PRINTLN("C16");
                    previous_ir_code = ir_code;
                    animations_paused = true;
                    pause_for_ir_previous_millis = millis();
                    select_static_beam_color();
                    break;
            //++++++++++ PATTERNS BUTTONS ++++++++++
                case 0xF7A05F: // Green Curtain Opening 
                    DEBUG_PRINTLN("Green Curtain Opening");
                    previous_ir_code = ir_code;
                    animations_paused = false;
                    GlowSerum.set_pattern(green_button_patterns[gbpi]);
                    gbpi = (gbpi+1) % GBP_NUM;
                    break;
                case 0xF7609F: // Blue Curtain Closing 
                    DEBUG_PRINTLN("Blue Curtain Closing");
                    previous_ir_code = ir_code;
                    animations_paused = false;
                    GlowSerum.set_pattern(blue_button_patterns[bbpi]);
                    GlowSerum.set_overlay(blue_button_overlays[bbpi], false);
                    bbpi = (bbpi+1) % BBP_NUM;
                    break;
                case 0xF7906F: // Left1
                    DEBUG_PRINTLN("Left1");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    GlowSerum.set_pattern(ORBIT_LEFT);
                    break;
                case 0xF750AF: // Right1
                    DEBUG_PRINTLN("Right1");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    GlowSerum.set_pattern(ORBIT_RIGHT);
                    break;
                case 0xF7B04F: // Left2
                    DEBUG_PRINTLN("Left2");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    GlowSerum.set_pattern(THEATER_CHASE_LEFT);
                    break;
                case 0xF7708F: // Right2
                    DEBUG_PRINTLN("Right2");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    GlowSerum.set_pattern(THEATER_CHASE_RIGHT);
                    break;
                case 0xF78877: // Left3
                    DEBUG_PRINTLN("Left3");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    GlowSerum.set_pattern(RUNNING_LIGHTS_LEFT);
                    break;
                case 0xF748B7: // Right3
                    DEBUG_PRINTLN("Right3");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    GlowSerum.set_pattern(RUNNING_LIGHTS_RIGHT);
                    break;
                case 0xF7A857: // Left4
                    DEBUG_PRINTLN("Left4");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    GlowSerum.set_pattern(SHOOTING_STAR_LEFT);
                    break;
                case 0xF76897: // Right4
                    DEBUG_PRINTLN("Right4");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    GlowSerum.set_pattern(SHOOTING_STAR_RIGHT);
                    break;
                case 0xF7E817: // Meteor: similar to Loop effect, but reverses at the end of the strip
                    DEBUG_PRINTLN("Meteor");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    GlowSerum.set_pattern(CYLON);
                    break;
            //++++++++++ RECURRENT BUTTONS ++++++++++
                case 0xF7E01F: // Auto: Cycle through all of the patterns
                    DEBUG_PRINTLN("Auto");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    GlowSerum.set_autocycle(true);
                    GlowSerum.set_flip_flop_animation(false);
                    break;
                case 0xF7D02F: // Loop (circular arrows button): Alternate between the current pattern and the next pattern
                    DEBUG_PRINTLN("Loop");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    GlowSerum.set_autocycle(false);
                    GlowSerum.set_flip_flop_animation(true);
                    break;
            //++++++++++ OVERLAYS BUTTONS ++++++++++
                case 0xF7F00F: // Flash
                    DEBUG_PRINTLN("Flash");
                    previous_ir_code = 0x00000001;
                    animations_paused = false;
                    //GlowSerum.set_overlay(NO_OVERLAY, false);
                    GlowSerum.set_overlay(NO_OVERLAY, true);
                    FastLED.setBrightness(255);
                    break;
                case 0xF7C837: // Jump
                    DEBUG_PRINTLN("Jump");
                    previous_ir_code = ir_code;
                    animations_paused = false;
                    GlowSerum.increment_overlay(true);
                    break;
            //++++++++++ NOT BUTTONS ++++++++++
                case 0x00000000: // Do Nothing
                    DEBUG_PRINTLN("Do Nothing");
                    beep_type = -1;
                    break;
                case 0x00000001: // Does Not Repeat
                    DEBUG_PRINTLN("Pressing And Holding This Button Does Nothing");
                    beep_type = -1;
                    break;
                default:
                    DEBUG_PRINTLN("Code Error - Misheard/Unrecognized Code");
                    animations_paused = true; // got a bad IR code so pause changing LEDs (i.e. allow interrupts) so a good code can be heard
                    pause_for_ir_previous_millis = millis();
                    beep_type = 1;
                    fill_solid(rim_leds, NUM_RIM_LEDS, CHSV(HUE_RED, 255, 255));
                    for (uint8_t i = 0; i < NUM_RIM_LEDS; i+=2) {
                        rim_leds[i] = CHSV(HUE_RED, 0, 255);
                    }
                    FastLED.show();
                    break;
            }

            beep(beep_type);
        }
        else if (results.value == 0xF7C03F) {
            DEBUG_PRINTLN("Power On");
            change_max_brightness(NEUTRAL);
            GlowSerum.set_sound_value_gain(SOUND_VALUE_GAIN_INITIAL);
            gbpi = 0;
            bbpi = 0;
            GlowSerum.set_pattern(RUNNING_LIGHTS_RIGHT);
            GlowSerum.set_overlay(NO_OVERLAY, true);
            GlowSerum.set_autocycle(false);
            GlowSerum.set_flip_flop_animation(true);
            is_accepting_commands = true;
            animations_paused = false;
            beep(0);
        }
        else {
            DEBUG_PRINTLN("Power is off. Not accepting commands.");
        }

        irrecv.resume(); // Receive the next value
    }

    if (animations_paused && (millis() - pause_for_ir_previous_millis) > pause_for_ir_interval) {
        pause_for_ir_previous_millis = millis();
        animations_paused = false;
    }

    if (is_accepting_commands && !animations_paused) {

        GlowSerum.reanimate();

        EVERY_N_MILLISECONDS(100) { gdynamic_hue+=3; }
        EVERY_N_MILLISECONDS(100) { grandom_hue = random8(); }
    }

    if (irrecv.isIdle()) {
        //FastLED.delay(1000/FRAMES_PER_SECOND);
        //FastLED[0].showLeds(FastLED.getBrightness());
        //FastLED[1].showLeds();
        FastLED.show(); // only want to use one controller so management of brightness and power usage is more easy
    }

    //print_dt();

}




