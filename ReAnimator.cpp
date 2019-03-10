/*
  This code is copyright 2019 Jonathan Thomson, jethomson.wordpress.com

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

#include "ReAnimator.h"


ReAnimator::ReAnimator(CRGB rim_leds_in[NUM_RIM_LEDS], CRGB beam_leds_in[NUM_BEAM_LEDS], CRGB helm_leds_in[NUM_HELM_LEDS], uint8_t *rim_hue_type, uint8_t *beam_hue_type, uint16_t led_strip_milliamps) : freezer(*this) {
    rim_leds = rim_leds_in;
    beam_leds = beam_leds_in;
    helm_leds = helm_leds_in;

    selected_rim_hue = rim_hue_type;
    selected_beam_hue = beam_hue_type;
    selected_led_strip_milliamps = led_strip_milliamps;

    homogenized_brightness = 255;

    pattern = ORBIT;
    transient_overlay = NO_OVERLAY;
    persistent_overlay = NO_OVERLAY;

#if !defined(LEFT_TO_RIGHT_IS_FORWARD) || LEFT_TO_RIGHT_IS_FORWARD
    direction_fp = &ReAnimator::forwards;
    antidirection_fp = &ReAnimator::backwards;
#else
    direction_fp = &ReAnimator::backwards;
    antidirection_fp = &ReAnimator::forwards;
#endif

    reverse = false;

    last_pattern_ran = NULL;

    autocycle_enabled = false;
    autocycle_previous_millis = 0;
    autocycle_interval = 30000;

    flipflop_enabled = false;
    flipflop_previous_millis = 0;
    flipflop_interval = 6000;

    previous_sample = 0;
    sample_peak = 0;
    sample_average = 0;
    sample_threshold = 20;
    sound_value_gain = SOUND_VALUE_GAIN_INITIAL;
}


ReAnimator::Freezer::Freezer(ReAnimator &r) : parent(r) {
    m_frozen = false;
    m_frozen_previous_millis = 0;
}


// When FastLED's power management functions are used FastLED dynamically adjusts the brightness level to be as high as possible while
// keeping the power draw near the specified level. This can lead to the brightness level of an animation noticeably increasing when
// fewer LEDs are lit and the brightness noticeably dipping when more LEDs are lit or their colors change.
// homogenize_brightness() learns the lowest brightness level of all the animations and uses it across every animation to keep a consistent
// brightness level. This will lead to dimmer animations and power usage almost always a good bit lower than what the FastLED power
// management function was set to aim for. Set the #define for HOMOGENIZE_BRIGHTNESS to false to disable this feature.
void ReAnimator::homogenize_brightness() {
    uint8_t max_brightness = calculate_max_brightness_for_power_vmA(rim_leds, NUM_RIM_LEDS, homogenized_brightness, LED_STRIP_VOLTAGE, selected_led_strip_milliamps);
    if (max_brightness < homogenized_brightness) {
        homogenized_brightness = max_brightness;
    }
}


void ReAnimator::set_selected_rim_hue(uint8_t *hue_type) {
    selected_rim_hue = hue_type;
}


void ReAnimator::set_selected_beam_hue(uint8_t *hue_type) {
    selected_beam_hue = hue_type;
}


void ReAnimator::set_selected_led_strip_milliamps(uint16_t led_strip_milliamps) {
    if (led_strip_milliamps > selected_led_strip_milliamps) {
        // normally homogenized_brightness only goes down but since the power is increased we need to reset homogenized_brightness so it
        // learn the new brightness level that makes all the animations have a consistent brightness
        homogenized_brightness = calculate_max_brightness_for_power_vmA(rim_leds, NUM_RIM_LEDS, 255, LED_STRIP_VOLTAGE, led_strip_milliamps);
    }
    else {
        homogenized_brightness = calculate_max_brightness_for_power_vmA(rim_leds, NUM_RIM_LEDS, homogenized_brightness, LED_STRIP_VOLTAGE, led_strip_milliamps);
    }
    selected_led_strip_milliamps = led_strip_milliamps;
}


Pattern ReAnimator::get_pattern() {
    return pattern;
}


int8_t ReAnimator::set_pattern(Pattern pattern_in) {
    return set_pattern(pattern_in, false, true);
}


int8_t ReAnimator::set_pattern(Pattern pattern_in, bool reverse) {
    return set_pattern(pattern_in, reverse, true);
}


int8_t ReAnimator::set_pattern(Pattern pattern_in, bool reverse_in, bool disable_autocycle_flipflop) {
    Pattern pattern_out = NULL;
    Overlay overlay_out = NULL;
    int8_t retval = 0;

    switch(pattern_in) {
        default:
            retval = INT8_MIN;
            // fall through to next case
        case ORBIT:
            pattern_out = ORBIT;
            overlay_out = NO_OVERLAY;
            break;
        case THEATER_CHASE:
            pattern_out = THEATER_CHASE;
            overlay_out = NO_OVERLAY;
            break;
        case RUNNING_LIGHTS:
            pattern_out = RUNNING_LIGHTS;
            overlay_out = NO_OVERLAY;
            break;
        case SHOOTING_STAR:
            pattern_out = SHOOTING_STAR;
            overlay_out = NO_OVERLAY;
            break;
        case CYLON:
            pattern_out = CYLON;
            overlay_out = NO_OVERLAY;
            break;
        case SOLID:
            pattern_out = SOLID;
            overlay_out = NO_OVERLAY;
            break;
        case JUGGLE:
            pattern_out = JUGGLE;
            overlay_out = NO_OVERLAY;
            break;
        case MITOSIS:
            pattern_out = MITOSIS;
            overlay_out = NO_OVERLAY;
            break;
        case BUBBLES:
            pattern_out = BUBBLES;
            overlay_out = NO_OVERLAY;
            break;
        case SPARKLE:
            pattern_out = SPARKLE;
            overlay_out = NO_OVERLAY;
            break;
        case MATRIX:
            //FastLED[0].clearLeds(NUM_RIM_LEDS); // this doesn't seem to work
            //FastLED[0].clearLedData();  // this works but I don't want to use multiple controllers
            fill_solid(rim_leds, NUM_RIM_LEDS, CRGB::Black); // clear once before starting
            pattern_out = MATRIX;
            overlay_out = NO_OVERLAY;
            break;
        case WEAVE:
            pattern_out = WEAVE;
            overlay_out = NO_OVERLAY;
            break;
        case STARSHIP_RACE:
            pattern_out = STARSHIP_RACE;
            overlay_out = NO_OVERLAY;
            break;
        case PAC_MAN:
            pattern_out = PAC_MAN;
            overlay_out = NO_OVERLAY;
            break;
        case BALLS:
            pattern_out = BALLS;
            overlay_out = NO_OVERLAY;
            break;
        case HALLOWEEN_FADE:
            pattern_out = HALLOWEEN_FADE;
            overlay_out = NO_OVERLAY;
            break;
        case HALLOWEEN_ORBIT:
            pattern_out = HALLOWEEN_ORBIT;
            overlay_out = NO_OVERLAY;
            break;
        case SOUND_RIBBONS:
            pattern_out = SOUND_RIBBONS;
            overlay_out = NO_OVERLAY;
            break;
        case SOUND_RIPPLE:
            pattern_out = SOUND_RIPPLE;
            overlay_out = NO_OVERLAY;
            break;
        case SOUND_ORBIT:
            pattern_out = SOUND_ORBIT;
            overlay_out = NO_OVERLAY;
            break;
        case SOUND_BLOCKS:
            pattern_out = SOUND_BLOCKS;
            overlay_out = NO_OVERLAY;
            break;
        case DYNAMIC_RAINBOW:
            pattern_out = DYNAMIC_RAINBOW;
            overlay_out = NO_OVERLAY;
            break;
    }

    pattern = pattern_out;
    set_overlay(overlay_out, false);

    reverse = reverse_in;

    if (disable_autocycle_flipflop) {
        set_autocycle_enabled(false);
        set_flipflop_enabled(false);
    }

    return retval;
}


int8_t ReAnimator::increment_pattern() {
    return increment_pattern(true);
}


int8_t ReAnimator::increment_pattern(bool disable_autocycle_flipflop) {
    return set_pattern(pattern+1, reverse, disable_autocycle_flipflop);
}


Overlay ReAnimator::get_overlay(bool is_persistent) {
    if (is_persistent) {
        return persistent_overlay;
    }
    else {
        return transient_overlay;
    }
}


int8_t ReAnimator::set_overlay(Overlay overlay_in, bool is_persistent) {
    Overlay overlay_out = NULL;
    int8_t retval = 0;

    switch(overlay_in) {
        default:
            retval = INT8_MIN;
            // fall through to next case
        case NO_OVERLAY:
            overlay_out = NO_OVERLAY;
            break;
        case GLITTER:
            overlay_out = GLITTER;
            break;
        case BREATHING:
            overlay_out = BREATHING;
            break;
        case CONFETTI:
            overlay_out = CONFETTI;
            break;
        case FLICKER:
            overlay_out = FLICKER;
            break;
        case FROZEN_DECAY:
            overlay_out = FROZEN_DECAY;
            break;
    }

    if (is_persistent) {
        persistent_overlay = overlay_out;
    }
    else {
        transient_overlay = overlay_out;
    }

    return retval;
}


void ReAnimator::increment_overlay(bool is_persistent) {
    if (is_persistent) {
        set_overlay(persistent_overlay+1, is_persistent);
    }
    else {
        set_overlay(transient_overlay+1, is_persistent);
    }
}


void ReAnimator::set_sound_value_gain(uint8_t gain) {
    sound_value_gain = gain;
}


uint32_t ReAnimator::get_autocycle_interval() {
    return autocycle_interval;
}


void ReAnimator::set_autocycle_interval(uint32_t inteval) {
    autocycle_interval = inteval;
    autocycle_previous_millis = 0; // set to zero so autocycling will start without waiting
}


bool ReAnimator::get_autocycle_enabled() {
    return autocycle_enabled;
}


void ReAnimator::set_autocycle_enabled(bool enabled) {
    autocycle_enabled = enabled;
    autocycle_previous_millis = 0; // set to zero so autocycling will start without waiting
}


// loop through all of the patterns
void ReAnimator::autocycle() {
    if((millis() - autocycle_previous_millis) > autocycle_interval) {
        autocycle_previous_millis = millis();
        DEBUG_PRINTLN("autocycle started");
        if (increment_pattern(false) == INT8_MIN) {
            // autocycle has looped back around to the first pattern so reverse them
            reverse = !reverse;
        }
    }
}


uint32_t ReAnimator::get_flipflop_interval() {
    return flipflop_interval;
}


void ReAnimator::set_flipflop_interval(uint32_t inteval) {
    flipflop_interval = inteval;
    flipflop_previous_millis = 0; // set to zero so flipfloping will start without waiting
}


bool ReAnimator::get_flipflop_enabled() {
    return flipflop_enabled;
}


void ReAnimator::set_flipflop_enabled(bool enabled) {
    flipflop_enabled = enabled;
    flipflop_previous_millis = 0; // set to zero so flipfloping will start without waiting
}


// alternate between running a pattern forwards or backwards
void ReAnimator::flipflop() {
    if((millis() - flipflop_previous_millis) > flipflop_interval) {
        flipflop_previous_millis = millis();
        DEBUG_PRINTLN("flip flop loop started");
        reverse = !reverse;
    }
}


void ReAnimator::reanimate() {
    if (autocycle_enabled) {
        autocycle();
    }

    if (flipflop_enabled) {
        flipflop();
    }

    process_sound();

    if (!freezer.is_frozen()) {
        run_pattern(pattern);
        last_pattern_ran = pattern;
    }

    apply_overlay(transient_overlay);
    apply_overlay(persistent_overlay);

    helm(200);
    tractor_beam(25);

    //print_dt();

#if HOMOGENIZE_BRIGHTNESS
    homogenize_brightness();
#endif

    if (transient_overlay != BREATHING && transient_overlay != FLICKER && persistent_overlay != BREATHING && persistent_overlay != FLICKER) {
        FastLED.setBrightness(homogenized_brightness);
    }
}


void ReAnimator::run_pattern(Pattern pattern) {
    int8_t retval = 0;
    uint8_t orbit_delta = !reverse ? 1 : -1;
    uint16_t(ReAnimator::*dfp)(uint16_t) = !reverse ? direction_fp : antidirection_fp;

    switch(pattern) {
        default:
            retval = INT8_MIN;
            // fall through to next case
        case ORBIT:
            orbit(20, orbit_delta);
            break;
        case THEATER_CHASE:
            //theater_chase(350, dfp);
            accelerate_decelerate_pattern(200, 10, 1000, &ReAnimator::theater_chase, dfp);
            break;
        case RUNNING_LIGHTS:
            //running_lights(30, dfp);
            accelerate_decelerate_pattern(30, 2, 1000, &ReAnimator::running_lights, dfp);
            //accelerate_decelerate_pattern(97, 2, 1000, &ReAnimator::running_lights, dfp); // for filming
            break;
        case SHOOTING_STAR:
            shooting_star(5, 5, 40, 50, dfp);
            break;
        case CYLON:
            cylon(20, dfp);
            break;
        case SOLID:
            solid(200);
            break;
        case JUGGLE:
            juggle();
            break;
        case MITOSIS:
            mitosis(50, 1);
            break;
        case BUBBLES:
            bubbles(100, dfp);
            break;
        case SPARKLE:
            sparkle(20, false, 32);
            break;
        case MATRIX:
            matrix(50);
            break;
        case WEAVE:
            weave(60);
            break;
        case STARSHIP_RACE:
            starship_race(88, dfp);
            break;
        case PAC_MAN:
            pac_man(150, dfp);
            break;
        case BALLS:
            bouncing_balls(40, dfp);
            break;
        case HALLOWEEN_FADE:
            halloween_colors_fade(50);
            break;
        case HALLOWEEN_ORBIT:
            halloween_colors_orbit(20, orbit_delta);
            break;
        case SOUND_RIBBONS:
            sound_ribbons(30);
            break;
        case SOUND_RIPPLE:
            sound_ripple(100, (sample_peak==1));
            break;
        case SOUND_BLOCKS:
            sound_blocks(50, (sound_value > 32));
            break;
        case SOUND_ORBIT:
            sound_orbit(30, dfp);
            break;
        case DYNAMIC_RAINBOW:
            //accelerate_decelerate_pattern(30, 2, 1000, &ReAnimator::dynamic_rainbow, dfp);
            dynamic_rainbow(50, dfp);
            break;
    }

    return retval;
}


void ReAnimator::apply_overlay(Overlay overlay) {
    int8_t retval = 0;

    switch(overlay) {
        default:
            retval = INT8_MIN;
            // fall through to next case
        case NO_OVERLAY:
            break;
        case GLITTER:
            glitter(700);
            break;
        case BREATHING:
            breathing(10);
            break;
        case CONFETTI:
            sparkle(20, true, 0);
            break;
        case FLICKER:
            flicker(150);
            break;
        case FROZEN_DECAY:
            freezer.timer(7000);
            if (freezer.is_frozen()) {
                fade_randomly(7, 100);
            }
            break;
    }

    return retval;
}


// ++++++++++++++++++++++++++++++
// ++++++++++ PATTERNS ++++++++++
// ++++++++++++++++++++++++++++++

void ReAnimator::orbit(uint16_t draw_interval, int8_t delta) {
    static uint16_t pos = NUM_RIM_LEDS;
    static uint8_t loop_num = 0;

    if (pattern != last_pattern_ran) {
        pos = NUM_RIM_LEDS;
    }

    if (is_wait_over(draw_interval)) {
        fadeToBlackBy(rim_leds, NUM_RIM_LEDS, 20);

        if (delta > 0) {
            pos = pos % NUM_RIM_LEDS;
        }
        else {
            // pos underflows after it goes below zero
            if (pos > NUM_RIM_LEDS-1) {
                pos = NUM_RIM_LEDS-1;
            }
        }

        rim_leds[pos] = CHSV(*selected_rim_hue, 255, 255);
        pos = pos + delta;

        loop_num = (pos == NUM_RIM_LEDS) ? loop_num+1 : loop_num; 
    }
}


void ReAnimator::theater_chase(uint16_t draw_interval, uint16_t(ReAnimator::*dfp)(uint16_t)) {
    static uint16_t delta = 0;

    if (is_wait_over(draw_interval)) {
        fadeToBlackBy(rim_leds, NUM_RIM_LEDS, 230);

        for (uint16_t i = 0; i+delta < NUM_RIM_LEDS; i=i+3) {
            rim_leds[(this->*dfp)(i+delta)] = CHSV(*selected_rim_hue, 255, 255);
        }

        delta = (delta + 1) % 3;
    }
}


void ReAnimator::running_lights(uint16_t draw_interval, uint16_t(ReAnimator::*dfp)(uint16_t)) {
    const uint8_t num_waves = 3; // results in three full sine waves across LED strip
    static uint16_t delta = 0;

    if (is_wait_over(draw_interval)) {
        for (uint16_t i = 0; i < NUM_RIM_LEDS; i++) {
            uint16_t a = num_waves*(i+delta)*255/(NUM_RIM_LEDS-1);
            // this pattern normally runs from right-to-left, so flip it by using negative indexing
            uint16_t ni = (NUM_RIM_LEDS-1) - i;
            rim_leds[(this->*dfp)(ni)] = CHSV(*selected_rim_hue, 255, sin8(a));
        }

        delta = (delta + 1) % (NUM_RIM_LEDS/num_waves);
    }
}


//star_size – the number of LEDs that represent the star, not counting the tail of the star.
//star_trail_decay - how fast the star trail decays. A larger number makes the tail short and/or disappear faster.
//spm - stars per minute
void ReAnimator::shooting_star(uint16_t draw_interval, uint8_t star_size, uint8_t star_trail_decay, uint8_t spm, uint16_t(ReAnimator::*dfp)(uint16_t)) {  
    static uint16_t start_pos = random16(0, NUM_RIM_LEDS/4);
    static uint16_t stop_pos = random16(star_size+(NUM_RIM_LEDS/2), NUM_RIM_LEDS);

    const uint16_t cool_down_interval = (60000-(spm*NUM_RIM_LEDS*draw_interval))/spm; // adds a delay between creation of new shooting stars
    static uint32_t cdi_pm = 0; // cool_down_interval_previous_millis

    static uint16_t pos = start_pos;

    if (pattern != last_pattern_ran) {
        start_pos = random16(0, NUM_RIM_LEDS/4);
        stop_pos = random16(star_size+(NUM_RIM_LEDS/2), NUM_RIM_LEDS);
        pos = start_pos;
        //cdi_pm doesn't need to be reset here because it's a cool down timer
    }

    if (is_wait_over(draw_interval)) {
        fade_randomly(128, star_trail_decay);

        if ( (millis() - cdi_pm) > cool_down_interval ) {
            for (uint8_t i = 0; i < star_size; i++) {
                rim_leds[(this->*dfp)(pos+(star_size-1)-i)] += CHSV(*selected_rim_hue, 255, 255);
                // we have to subtract 1 from star_size because one piece goes at pos
                // example, if star_size = 3: [*]  [*]  [*]
                //                            pos pos+1 pos+2
            }
            pos++;
            if (pos+(star_size-1) >= stop_pos+1) {
                start_pos = random16(0, NUM_RIM_LEDS/4);
                stop_pos = random16(star_size+(NUM_RIM_LEDS/2), NUM_RIM_LEDS);
                pos = start_pos;
                cdi_pm = millis();
            }
        }
    }
}


void ReAnimator::cylon(uint16_t draw_interval, uint16_t(ReAnimator::*dfp)(uint16_t)) {
    static uint16_t pos = 0;
    static int8_t delta = 1;

    if (pattern != last_pattern_ran) {
        pos = 0;
        delta = 1;
    }

    if (is_wait_over(draw_interval)) {
        fadeToBlackBy(rim_leds, NUM_RIM_LEDS, 20);

        rim_leds[(this->*dfp)(pos)] += CHSV(*selected_rim_hue, 255, 192);

        pos = pos + delta;
        if (pos == 0 || pos == NUM_RIM_LEDS-1) {
            delta = -delta;
        }
    }
}


void ReAnimator::solid(uint16_t draw_interval) {
    if (is_wait_over(draw_interval)) {
        fill_solid(rim_leds, NUM_RIM_LEDS, CHSV(*selected_rim_hue, 255, 255));
    }
}


// borrowed from FastLED/examples/DemoReel00.ino -Mark Kriegsman, December 2014
void ReAnimator::juggle() {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(rim_leds, NUM_RIM_LEDS, 20);
    byte dothue = 0;
    for(uint8_t i = 0; i < 8; i++) {
        rim_leds[beatsin16( i+7, 0, NUM_RIM_LEDS-1 )] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}


void ReAnimator::mitosis(uint16_t draw_interval, uint8_t cell_size) {
    const uint16_t start_pos = NUM_RIM_LEDS/2;
    static uint16_t pos = start_pos;

    if (pattern != last_pattern_ran) {
        pos = start_pos;
    }

    if (is_wait_over(draw_interval)) {
        fadeToBlackBy(rim_leds, NUM_RIM_LEDS, 30);

        for (uint8_t i = 0; i < cell_size; i++) {
            uint16_t pi = pos+(cell_size-1)-i;
            uint16_t ni = (NUM_RIM_LEDS-1) - pi;
            rim_leds[pi] = CHSV(*selected_rim_hue, 255, 255);
            rim_leds[ni] = CHSV(*selected_rim_hue, 255, 255);
        }
        pos++;
        if (pos+(cell_size-1) >= NUM_RIM_LEDS) {
            pos = start_pos;
        }
    }
}


void ReAnimator::bubbles(uint16_t draw_interval, uint16_t(ReAnimator::*dfp)(uint16_t)) {
    const uint8_t num_bubbles = 8;
    static uint8_t bubble_time[num_bubbles] = {};

    if (pattern != last_pattern_ran) {
        bubble_time[num_bubbles] = memset(bubble_time, 0, sizeof(bubble_time));
    }

    if (is_wait_over(draw_interval)) {
        fill_solid(rim_leds, NUM_RIM_LEDS, CRGB::Black);

        for (uint8_t i = 0; i < num_bubbles; i++) {
            if (bubble_time[i] == 0 && random8(33) == 0) {
                bubble_time[i] = random8(1, sqrt16(UINT16_MAX/NUM_RIM_LEDS));
            }

            if (bubble_time[i] > 0) {
                uint8_t t = bubble_time[i];

                uint16_t d = t*(t+1);

                if (t == UINT8_MAX) {
                    d = UINT16_MAX;
                }

                uint16_t pos = lerp16by16(0, NUM_RIM_LEDS-1, d);
                rim_leds[(this->*dfp)(pos)] += CHSV(i*(256/num_bubbles) + *selected_rim_hue, 255, 192);
                motion_blur((3*pos)/NUM_RIM_LEDS, pos, dfp);

                if (t < UINT8_MAX) {
                    t+=10;
                    if (t > bubble_time[i]) { // make sure overflow didn't happen
                        bubble_time[i] = t;
                    }
                    else {
                        bubble_time[i] = UINT8_MAX;
                    }
                }
                else {
                    bubble_time[i] = 0;
                }
            }        
        }
    }
}


void ReAnimator::sparkle(uint16_t draw_interval, bool random_color, uint8_t fade) {
    uint8_t hue = (random_color) ? random8() : *selected_rim_hue;

    // it's necessary to use finished_waiting() here instead of is_wait_over()
    // because sparkle can be an overlay
    if (finished_waiting(draw_interval)) {
        fadeToBlackBy(rim_leds, NUM_RIM_LEDS, fade);

        rim_leds[random16(NUM_RIM_LEDS)] = CHSV(hue, 255, 255);
    }
}


// resembles the green code from The Matrix
void ReAnimator::matrix(uint16_t draw_interval) {
    if (is_wait_over(draw_interval)) {
        memmove(&rim_leds[1], &rim_leds[0], (NUM_RIM_LEDS-1)*sizeof(CRGB));

        if (random8() > 205) {
            rim_leds[0] = CHSV(HUE_GREEN, 255, 255);
        }
        else {
            rim_leds[0] = CRGB::Black;
        }
    }
}


void ReAnimator::weave(uint16_t draw_interval) {
    static uint16_t pos = 0;

    if (pattern != last_pattern_ran) {
        pos = 0;
    }

    if (is_wait_over(draw_interval)) {
        fadeToBlackBy(rim_leds, NUM_RIM_LEDS, 20);

        rim_leds[pos] += CHSV(*selected_rim_hue, 255, 128);
        rim_leds[NUM_RIM_LEDS-1-pos] += CHSV(*selected_rim_hue+(HUE_PURPLE-HUE_ALIEN_GREEN), 255, 128);

        pos = (pos + 2) % NUM_RIM_LEDS;
    }
}


void ReAnimator::starship_race(uint16_t draw_interval, uint16_t(ReAnimator::*dfp)(uint16_t)) {
    const uint16_t race_distance = (11*UINT8_MAX)/2; // 7/2 -> 3.5 laps
    const uint8_t total_starships = 5;
    // UINT8_MAX/NUM_RIM_LEDS is the speed required for a starship to move one LED per redraw
    const uint8_t range = ceil(static_cast<float>(UINT8_MAX)/NUM_RIM_LEDS);
    const uint8_t speed_boost_period = 4; // every N redraws speed_boost is increased

    static Starship starships[total_starships];
    static bool go = true;
    static uint8_t redraw_count = 0;
    static uint8_t speed_boost = 0;
    static uint8_t count_down = 0;

    if (pattern != last_pattern_ran) {
        for (uint8_t i = 0; i < total_starships; i++) {
            starships[i].distance = 0;
            starships[i].color = i*(256/total_starships);
        }
        go = true;
        redraw_count = 0;
        speed_boost = 0;
        count_down = 0;
    }

    if (is_wait_over(draw_interval)) {
        if (go) {
            fill_solid(rim_leds, NUM_RIM_LEDS, CRGB::Black);

            for (uint8_t i = 0; i < total_starships; i++) {
                // current_total_distance = previous_total_distance + speed*delta_time, delta_time is always 1
                starships[i].distance = starships[i].distance + random8(speed_boost, (range+speed_boost)+1);
            }

            // sort starships by distance travelled in descending order
            qsort(starships, total_starships, sizeof(Starship), compare);

            for (uint8_t i = 0; i < total_starships; i++) {
                uint16_t pos = lerp16by8(0, NUM_RIM_LEDS-1, starships[i].distance);

                // we don't want multiple starships' position to be on the same LED
                // if an LED is already lit then a starship is already there so
                // move backwards until we find an unlit LED
                while (rim_leds[(this->*dfp)(pos)] != CRGB(CRGB::Black) && pos > 0) {
                    pos--;
                }
                rim_leds[(this->*dfp)(pos)] = CHSV(starships[i].color, 255, 255);
            }

            redraw_count++;
            if (redraw_count == speed_boost_period) {
                redraw_count = 0;
                speed_boost++;
            }
        }
        else {
            // next race will happen after count_down*draw_interval has elapsed
            count_down--;
            if (count_down == 0) {
                go = true;
            }
        }

        if (starships[0].distance >= race_distance) {
            // race is finished
            fill_solid(rim_leds, NUM_RIM_LEDS, CHSV(starships[0].color, 255, 255));

            for (uint8_t i = 0; i < total_starships; i++) {
                starships[i].distance = 0;
                starships[i].color = i*(256/total_starships);
            }

            go = false;
            redraw_count = 0;
            speed_boost = 0;
            count_down = 10;
        }

    }
}


void ReAnimator::pac_man(uint16_t draw_interval, uint16_t(ReAnimator::*dfp)(uint16_t)) {
    static uint16_t pac_man_pos = 0;
    static int8_t pac_man_delta = 1;

    static uint16_t blinky_pos = (-2 + NUM_RIM_LEDS) % NUM_RIM_LEDS;
    static uint16_t pinky_pos  = (-3 + NUM_RIM_LEDS) % NUM_RIM_LEDS;
    static uint16_t inky_pos   = (-4 + NUM_RIM_LEDS) % NUM_RIM_LEDS;
    static uint16_t clyde_pos  = (-5 + NUM_RIM_LEDS) % NUM_RIM_LEDS;
    static uint8_t blinky_visible = 1;
    static uint8_t pinky_visible = 1;
    static uint8_t inky_visible = 1;
    static uint8_t clyde_visible = 1;
    static int8_t ghost_delta = 1;

    static uint16_t power_pellet_pos = 0;
    static bool power_pellet_flash_state = 1;
    static uint8_t pac_dots[NUM_RIM_LEDS] = {};

    if (pattern != last_pattern_ran) {
        pac_man_pos = 0;
    }

    if (is_wait_over(draw_interval)) {
        fill_solid(rim_leds, NUM_RIM_LEDS, CRGB::Black);

        if (pac_man_pos == 0) {
            blinky_pos = (-2 + NUM_RIM_LEDS) % NUM_RIM_LEDS;
            pinky_pos  = (-3 + NUM_RIM_LEDS) % NUM_RIM_LEDS;
            inky_pos   = (-4 + NUM_RIM_LEDS) % NUM_RIM_LEDS;
            clyde_pos  = (-5 + NUM_RIM_LEDS) % NUM_RIM_LEDS;
            blinky_visible = 1;
            pinky_visible = 1;
            inky_visible = 1;
            clyde_visible = 1;
            pac_man_delta = 1;
            ghost_delta = 1;

            // the power pellet must be at least 16 leds forward of led[0]
            // from 18 to (3/4)*NUM_RIM_LEDS, multiply makes it even so that it falls on a pac_dot led
            power_pellet_pos = 2*random16(9, (3*NUM_RIM_LEDS)/8 + 1); 

            for (uint8_t i = 0; i < NUM_RIM_LEDS; i+=2) {
                pac_dots[i] = 1;
            }
            pac_dots[power_pellet_pos] = 2;
        }

        for (uint8_t i = 0; i < NUM_RIM_LEDS; i+=2) {
            rim_leds[(this->*dfp)(i)] = (pac_dots[i] == 1) ? CRGB::White : CRGB::Black;
        }

        if (pac_dots[power_pellet_pos] == 2) {
            if (power_pellet_flash_state) {
                power_pellet_flash_state = !power_pellet_flash_state;
                rim_leds[(this->*dfp)(power_pellet_pos)] = CHSV(HUE_RED, 255, 255);
            }
            else {
                power_pellet_flash_state = !power_pellet_flash_state;
                rim_leds[(this->*dfp)(power_pellet_pos)] = CRGB::Black;
            }
        }

        if (pac_dots[power_pellet_pos] == 2) {
            rim_leds[(this->*dfp)(blinky_pos)] = CHSV(HUE_RED, 255, blinky_visible*255);
            rim_leds[(this->*dfp)(pinky_pos)]  = CHSV(HUE_PINK, 255, pinky_visible*255);
            rim_leds[(this->*dfp)(inky_pos)]   = CHSV(HUE_AQUA, 255, inky_visible*255);
            rim_leds[(this->*dfp)(clyde_pos)]  = CHSV(HUE_ORANGE, 255, clyde_visible*255);
        }
        else if (blinky_visible || pinky_visible || inky_visible || clyde_visible) {
            pac_man_delta = -3;
            ghost_delta = -2;

            if (pac_man_pos == blinky_pos) {
                blinky_visible = 0;
            }
            else if (pac_man_pos == pinky_pos) {
                pinky_visible = 0;
            }
            else if (pac_man_pos == inky_pos) {
                inky_visible = 0;
            }
            else if (pac_man_pos == clyde_pos) {
                clyde_visible = 0;
            }

            rim_leds[(this->*dfp)(blinky_pos)] = CHSV(HUE_BLUE, 255, blinky_visible*255);
            rim_leds[(this->*dfp)(pinky_pos)]  = CHSV(HUE_BLUE, 255, pinky_visible*255);
            rim_leds[(this->*dfp)(inky_pos)]   = CHSV(HUE_BLUE, 255, inky_visible*255);
            rim_leds[(this->*dfp)(clyde_pos)]  = CHSV(HUE_BLUE, 255, clyde_visible*255);

        }
        else {
            pac_man_delta = 1;
        }

        blinky_pos = blinky_pos + ghost_delta;
        blinky_pos = (NUM_RIM_LEDS+blinky_pos) % NUM_RIM_LEDS;
        pinky_pos = pinky_pos + ghost_delta;
        pinky_pos = (NUM_RIM_LEDS+pinky_pos) % NUM_RIM_LEDS;
        inky_pos = inky_pos + ghost_delta;
        inky_pos = (NUM_RIM_LEDS+inky_pos) % NUM_RIM_LEDS;
        clyde_pos = clyde_pos + ghost_delta;
        clyde_pos = (NUM_RIM_LEDS+clyde_pos) % NUM_RIM_LEDS;

        rim_leds[(this->*dfp)(pac_man_pos)] = CHSV(HUE_YELLOW, 255, 255);
        pac_dots[pac_man_pos] = 0;

        pac_man_pos = pac_man_pos + pac_man_delta;
        pac_man_pos = (NUM_RIM_LEDS+pac_man_pos) % NUM_RIM_LEDS;
    }

}


// a ball will move one LED when its height changes by (2^16)/NUM_RIM_LEDS
// the fastest movement will be from led[0] to led[1] and t=0 to t=ball_time_delta
// we want the fastest movement to take one draw interval so the function isn't called unnecessarily fast
// therefore we want ball_time_delta to result in h increasing by (2^16)/NUM_RIM_LEDS
// for 52 LEDS (2^16)/NUM_RIM_LEDS = 1260 and if ball_vi = vi_max = 510
// -1*t^2 + 510*t - 1260 = 0
// minimum ball_time_delta = (-510 + sqrt(510^2 - 4*(-1*-1260))/(2*-1) = 2.48
// increasing ball_time_delta lets you increase the draw_interval therefore decreasing the frequency of redraws
void ReAnimator::bouncing_balls(uint16_t draw_interval, uint16_t(ReAnimator::*dfp)(uint16_t)) {
    const uint16_t vi_max = 510; // initial velocity, 512 will make h exceed UINT16_MAX
    const uint8_t blur_length = 3;
    const uint8_t num_balls = 5;
    const uint8_t ball_time_delta = 4;
    static uint16_t ball_time[num_balls] = {};
    static uint16_t ball_vi[num_balls] = {};

    if (pattern != last_pattern_ran) {
        ball_time[num_balls] = memset(ball_time, 0, sizeof(ball_time));
        ball_vi[num_balls] = memset(ball_vi, 0, sizeof(ball_vi));
    }

    if (is_wait_over(draw_interval)) {
        fill_solid(rim_leds, NUM_RIM_LEDS, CRGB::Black);

        for (uint8_t i = 0; i < num_balls; i++) {
            uint16_t t = ball_time[i];

            uint16_t h = (ball_vi[i] - t)*t;
            int16_t v = ball_vi[i] - 2*t;

            if (t >= ball_vi[i]) {
                // the ball has hit the ground
                h = 0;
                v = 0;
                ball_time[i] = 0;
                ball_vi[i] = random16(vi_max/3, vi_max+1); // vi_max+1 for up to and including vi_max
            }

            uint16_t pos = lerp16by16(0, NUM_RIM_LEDS-1, h);
            rim_leds[(this->*dfp)(pos)] += CHSV(i*(256/num_balls), 255, 192);
            motion_blur((blur_length*(int32_t)v)/(int32_t)vi_max, pos, dfp);

            ball_time[i] = ball_time[i] + ball_time_delta;
        }
    }
}


void ReAnimator::halloween_colors_fade(uint16_t draw_interval) {
    CRGBPalette16 halloween_colors;
    halloween_colors = CRGBPalette16(CHSV(HUE_ORANGE, 255, 255),
                                   CHSV(HUE_PURPLE, 255, 255),
                                   CHSV(HUE_RED, 255, 255),
                                   CHSV(HUE_ALIEN_GREEN, 255, 255));

    static uint8_t delta = 0;

    if (is_wait_over(draw_interval)) {
        //fill_palette(rim_leds, NUM_RIM_LEDS, delta, 6, halloween_colors, 255, LINEARBLEND);
        for(uint16_t i = 0; i < NUM_RIM_LEDS; i++) {
            rim_leds[i] = ColorFromPalette(halloween_colors, delta, 255);
        }
        delta++;
    }
}


void ReAnimator::halloween_colors_orbit(uint16_t draw_interval, int8_t delta) {
    const uint8_t num_hues = 6;
    static uint8_t hi = 0;
    uint8_t hues[num_hues] = {HUE_ORANGE, HUE_PURPLE, HUE_ORANGE, HUE_RED, HUE_ORANGE, HUE_ALIEN_GREEN};

    static uint16_t pos = NUM_RIM_LEDS;

    if (pattern != last_pattern_ran) {
        pos = 0;
    }

    if (is_wait_over(draw_interval)) {
        if (delta > 0) {
            pos = pos % NUM_RIM_LEDS;
        }
        else {
            // pos underflows after it goes below zero
            if (pos > NUM_RIM_LEDS-1) {
                pos = NUM_RIM_LEDS-1;
            }
        }

        rim_leds[pos] = CHSV(hues[hi], 255, 255);
        pos = pos + delta;
        if (pos == NUM_RIM_LEDS) {
            hi = (hi+1) % num_hues;
        }
    }
}


void ReAnimator::sound_ribbons(uint16_t draw_interval) {
    if (is_wait_over(draw_interval)) {
        fadeToBlackBy(rim_leds, NUM_RIM_LEDS, 20);

        rim_leds[NUM_RIM_LEDS/2] = CHSV(*selected_rim_hue, 255, sound_value);
        rim_leds[(NUM_RIM_LEDS/2)-1] = CHSV(*selected_rim_hue, 255, sound_value);
        fission();
    }                                                                                
}


// derived from this code https://gist.github.com/suhajdab/9716635
void ReAnimator::sound_ripple(uint16_t draw_interval, bool trigger) {
    static bool enabled = true;
    const uint16_t max_delta = 16;
    static uint16_t delta = 0;
    static uint16_t center = NUM_RIM_LEDS/2;

    if (pattern != last_pattern_ran) {
        enabled = true;
        delta = 0;
        center = NUM_RIM_LEDS/2;
    }

    if (trigger) {
        enabled = true;
    }

    if (is_wait_over(draw_interval)) {
        fadeToBlackBy(rim_leds, NUM_RIM_LEDS, 170);

        if (enabled) {
            // waves created by primary droplet
            rim_leds[(NUM_RIM_LEDS+center+delta) % NUM_RIM_LEDS] = CHSV(*selected_rim_hue, 255, pow(0.8, delta)*255);
            rim_leds[(NUM_RIM_LEDS+center-delta) % NUM_RIM_LEDS] = CHSV(*selected_rim_hue, 255, pow(0.8, delta)*255);

            if (delta > 3) {
                // waves created by rebounded droplet
                rim_leds[(NUM_RIM_LEDS+center+(delta-3)) % NUM_RIM_LEDS] = CHSV(*selected_rim_hue, 255, pow(0.8, delta - 2)*255);
                rim_leds[(NUM_RIM_LEDS+center-(delta-3)) % NUM_RIM_LEDS] = CHSV(*selected_rim_hue, 255, pow(0.8, delta - 2)*255);
            }

            delta++;
            if (delta == max_delta) {
                delta = 0;
                center = random16(NUM_RIM_LEDS);
                enabled = false;
            }
        }
    }
}


void ReAnimator::sound_orbit(uint16_t draw_interval, uint16_t(ReAnimator::*dfp)(uint16_t)) {
    if (is_wait_over(draw_interval)) {
        for(uint16_t i = NUM_RIM_LEDS-1; i > 0; i--) {
            rim_leds[(this->*dfp)(i)] = rim_leds[(this->*dfp)(i-1)];
        }

        rim_leds[(this->*dfp)(0)] = CHSV(*selected_rim_hue, 255, sound_value);
    }
}


void ReAnimator::sound_blocks(uint16_t draw_interval, bool trigger) {
    uint8_t hue = random8();

    static bool enabled = true;

    if (trigger) {
        enabled = true;
    }

    if (is_wait_over(draw_interval)) {
        fadeToBlackBy(rim_leds, NUM_RIM_LEDS, 5);

        if (enabled) {
            uint16_t block_start = random16(NUM_RIM_LEDS);
            uint8_t block_size = random8(3,8);
            for (uint8_t i = 0; i < block_size; i++) {
                uint16_t pos = (NUM_RIM_LEDS+block_start+i) % NUM_RIM_LEDS;
                rim_leds[pos] = CHSV(hue, 255, 255);
            }
            enabled = false;
        }
    }
}


void ReAnimator::dynamic_rainbow(uint16_t draw_interval, uint16_t(ReAnimator::*dfp)(uint16_t)) {
    static uint16_t delta = 0;

    if (is_wait_over(draw_interval)) {
        for(uint16_t i = NUM_RIM_LEDS-1; i > 0; i--) {
            rim_leds[(this->*dfp)(i)] = rim_leds[(this->*dfp)(i-1)];
        }

        rim_leds[(this->*dfp)(0)] = CHSV(((NUM_RIM_LEDS-1-delta)*255/NUM_RIM_LEDS), 255, 255);

        delta = (delta + 1) % NUM_RIM_LEDS;
    }
}


void ReAnimator::helm(uint16_t draw_interval) {
    static uint16_t pos = 0;
    static int8_t delta = 1;
    static uint8_t cycles = 0;
    const uint8_t cycles_limit = 5;

    static uint32_t pm = 0; // previous millis
    if ( (millis() - pm) > draw_interval ) {
        pm = millis();
        //fadeToBlackBy(helm_leds, NUM_HELM_LEDS, 8);

        // these will blink randomly
        helm_leds[1] = CHSV(HUE_RED, 255, 255);
        helm_leds[2] = CHSV(HUE_GREEN, 255, 255);
        helm_leds[3] = CHSV(HUE_BLUE, 255, 255);
        helm_leds[5] = CHSV(HUE_ORANGE, 255, 255);
        helm_leds[6] = CHSV(HUE_PURPLE, 255, 255);
        helm_leds[random8(7)] = CRGB::Black;
        helm_leds[random8(7)] = CRGB::Black;

        // these will stay solid
        helm_leds[0] = CHSV(*selected_rim_hue, 255, 255);
        helm_leds[4] = CHSV(*selected_beam_hue, 255, 255);

        //pos = pos + delta;
    }
}


void ReAnimator::tractor_beam(uint16_t draw_interval) {
    static uint16_t pos = 0;
    static int8_t delta = 1;
    static uint8_t cycles = 0;
    const uint8_t cycles_limit = 5;

    static uint32_t pm = 0; // previous millis
    if ( (millis() - pm) > draw_interval ) {
        pm = millis();
        fadeToBlackBy(beam_leds, NUM_BEAM_LEDS, 8);

        beam_leds[pos] = CHSV(*selected_beam_hue, 255, 255);
        pos = pos + delta;

        // if delta is positive pos is NUM_BEAM_LEDS
        // if delta is negative pos underflowed to UINT16_MAX
        if (pos > NUM_BEAM_LEDS-1) {
            cycles++;
            if (cycles == cycles_limit) {
                cycles = 0;
                delta = -delta;
            }
            if (delta > 0) {
                pos = 0;
            }
            else {
                pos = NUM_BEAM_LEDS-1;
            }
        }
    }
}


// ++++++++++++++++++++++++++++++
// ++++++++++ OVERLAYS ++++++++++
// ++++++++++++++++++++++++++++++

void ReAnimator::breathing(uint16_t interval) {
    const uint8_t min_brightness = 2;
    static uint8_t delta = 0; // goes up to 255 then overflows back to 0

    if (finished_waiting(interval)) {
        // since FastLED is managing the maximum power delivered use the following function to find the _actual_ maximum brightness allowed for
        // these power consumption settings. setting brightness to a value higher that max_brightness will not actually increase the brightness.
        uint8_t max_brightness = calculate_max_brightness_for_power_vmA(rim_leds, NUM_RIM_LEDS, homogenized_brightness, LED_STRIP_VOLTAGE, selected_led_strip_milliamps);
        uint8_t b = scale8(triwave8(delta), max_brightness-min_brightness)+min_brightness;

        DEBUG_PRINTLN(b);
        FastLED.setBrightness(b);

        delta++;
    }
}


void ReAnimator::flicker(uint16_t interval) {
    fade_randomly(10, 150);

    // an on or off period less than 16 ms probably can't be perceived
    if (finished_waiting(interval)) {
        //FastLED.setBrightness((random8(1,11) > 4)*255);
        FastLED.setBrightness((random8(1,11) > 4)*homogenized_brightness);
    }
}


void ReAnimator::glitter(uint16_t chance_of_glitter) {
    if (chance_of_glitter > random16()) {
        rim_leds[random16(NUM_RIM_LEDS)] += CRGB::White;
    }
}


void ReAnimator::fade_randomly(uint8_t chance_of_fade, uint8_t decay) {
    for (uint16_t i = 0; i < NUM_RIM_LEDS; i++) {
        if (chance_of_fade > random8()) {
            rim_leds[i].fadeToBlackBy(decay);
        }
    }
}


// ++++++++++++++++++++++++++++++
// ++++++++++ HELPERS +++++++++++
// ++++++++++++++++++++++++++++++

uint16_t ReAnimator::forwards(uint16_t index) {
    return index;
}


uint16_t ReAnimator::backwards(uint16_t index) {
    return (NUM_RIM_LEDS-1)-index;
}


// If two functions running close to each other both call is_wait_over()
// the one with the shorter interval will reset the timer such that the
// function with the longer interval will never see its interval has
// elapsed, therefore a second function that does the same thing as
// is_wait_over() has been added. This is only a concern when a pattern
// function and an overlay function are both called at the same time.
// Patterns should use is_wait_over() and overlays should use finished_waiting(). 
bool ReAnimator::is_wait_over(uint16_t interval) {
    static uint32_t pm = 0; // previous millis
    if ( (millis() - pm) > interval ) {
        pm = millis();
        return true;
    }
    else {
        return false;
    }
}


bool ReAnimator::finished_waiting(uint16_t interval) {
    static uint32_t pm = 0; // previous millis
    if ( (millis() - pm) > interval ) {
        pm = millis();
        return true;
    }
    else {
        return false;
    }
}


void ReAnimator::accelerate_decelerate_pattern(uint16_t draw_interval_initial, uint16_t delta_initial, uint16_t update_period, void(ReAnimator::*pfp)(uint16_t, uint16_t(ReAnimator::*dfp)(uint16_t)), uint16_t(ReAnimator::*dfp)(uint16_t)) {
    static uint16_t draw_interval = draw_interval_initial;
    static int8_t delta = delta_initial;

    if (pattern != last_pattern_ran) {
        draw_interval = draw_interval_initial;
        delta = delta_initial;
    }

    if (finished_waiting(update_period)) {
        draw_interval = draw_interval - delta;
        // if you are filming the strip at 30 fps you don't want to draw any faster than once every 67 ms
        //if (draw_interval <= 67 || draw_interval >= draw_interval_initial) {
        if (draw_interval <= 0 || draw_interval >= draw_interval_initial) {
            delta = -1*delta;
        }
    }

    (this->*pfp)(draw_interval, dfp);
}


// derived from this code https://github.com/atuline/FastLED-Demos/blob/master/soundmems_demo/soundmems.h
void ReAnimator::process_sound() {
    const uint16_t DC_OFFSET = 513;  // measured
    const uint8_t NUM_SAMPLES = 64;

    static int16_t sample_buffer[NUM_SAMPLES];
    static uint16_t sample_sum = 0;
    static uint8_t i = 0;

    int16_t sample = 0;

    sample_peak = 0;

    sample = analogRead(MIC_PIN) - DC_OFFSET;
    sample = abs(sample);

    if (sample < sample_threshold) {
        sample = 0;
    }

    sample_sum += sample - sample_buffer[i]; // add newest sample and subtract oldest sample from the sum
    sample_average = sample_sum / NUM_SAMPLES;
    sample_buffer[i] = sample;  // overwrite oldest sample with newest sample
    i = (i + 1) % NUM_SAMPLES;

    sound_value = sound_value_gain*sample_average;
    sound_value = min(sound_value, 255);

    if (sample > (sample_average + sample_threshold) && (sample < previous_sample)) {
        sample_peak = 1;
    }
  
    previous_sample = sample;
}


void ReAnimator::motion_blur(int8_t blur_num, uint16_t pos, uint16_t(ReAnimator::*dfp)(uint16_t)) {
    if (blur_num > 0) {
        for (uint8_t i = 1; i < blur_num+1; i++) {
            if (pos >= pos-i) {
                rim_leds[(this->*dfp)(pos-i)] += rim_leds[(this->*dfp)(pos)];
                rim_leds[(this->*dfp)(pos-i)].fadeToBlackBy(120+(i*120/blur_num));
            }
        }
    }
    else if (blur_num < 0) {
        for (uint8_t i = 1; i < abs(blur_num)+1; i++) {
            if (pos+i < NUM_RIM_LEDS) {
                rim_leds[(this->*dfp)(pos+i)] += rim_leds[(this->*dfp)(pos)];
                rim_leds[(this->*dfp)(pos+i)].fadeToBlackBy(120+(i*120/abs(blur_num)));
            }
        }
    }
}


void ReAnimator::fission() {
    for (uint16_t i = NUM_RIM_LEDS-1; i > NUM_RIM_LEDS/2; i--) {
        rim_leds[i] = rim_leds[i-1];
    }

    for (uint16_t i = 0; i < NUM_RIM_LEDS/2; i++) {
        rim_leds[i] = rim_leds[i+1];
    }
}


// freeze_interval must be greater than m_failsafe_timeout
void ReAnimator::Freezer::timer(uint16_t freeze_interval) {
    static uint32_t pm = millis() - freeze_interval; // previous millis

    if ((millis() - pm) > freeze_interval) {
        pm = millis();
        m_frozen = true;
        m_frozen_previous_millis = millis();
    }
}


bool ReAnimator::Freezer::is_frozen() {
    static bool all_black = false;
    static uint16_t frozen_duration = m_failsafe_timeout;

    if ((millis() - m_frozen_previous_millis) > frozen_duration) {
        m_frozen = false;
        all_black = false;
        frozen_duration = m_failsafe_timeout;
    }
    else if (!all_black) {
        for (uint16_t i = 0; i < NUM_RIM_LEDS; i++) {
            all_black = true;
            if (parent.rim_leds[i] != CRGB(CRGB::Black)) {
                all_black = false;
                break;
            }
        }
        if (all_black && ((m_frozen_previous_millis + m_failsafe_timeout) - millis()) > m_after_all_black_pause) {
            // after all the LEDs after found to be dark unfreeze after a short pause
            m_frozen_previous_millis = millis();
            frozen_duration = m_after_all_black_pause;
        }
    }

    return m_frozen;
}


static int ReAnimator::compare(const void *a, const void *b) {
  Starship *StarshipA = (Starship *)a;
  Starship *StarshipB = (Starship *)b;

  return (StarshipB->distance > StarshipA->distance) - (StarshipA->distance > StarshipB->distance); // descending order
}


/*
void ReAnimator::print_dt() {
    static uint32_t pm = 0; // previous millis
    Serial.print("dt: ");
    Serial.println(millis() - pm);
    pm = millis();
}
*/
