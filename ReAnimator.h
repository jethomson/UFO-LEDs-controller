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

#ifndef REANIMATOR_H
#define REANIMATOR_H

#include "UFO_LEDs_controller.h"


// Conventions
// -----------
// Pixel 0 is the first and leftmost pixel as seen by the viewer (i.e. the viewer's left).
// Forward is from left to right.
// Pushing a right arrow button moves an pattern forward from left to right.
// Pushing a left arrow button moves an pattern backward from right to left.
#define LEFT_TO_RIGHT true

#define MIC_PIN    A1

class ReAnimator {

    CRGB *rim_leds;
    CRGB *beam_leds;
    CRGB *helm_leds;
    uint8_t *selected_rim_hue;
    uint8_t *selected_beam_hue;
    uint16_t selected_led_strip_milliamps;

    Pattern pattern;
    Overlay local_overlay;
    Overlay global_overlay;

    bool autocycle;
    uint32_t autocycle_previous_millis;
    const uint32_t autocycle_interval = 30000;

    bool flip_flop_animation;  // play forwards and backwards animation alternatingly in a loop
    uint32_t ffa_previous_millis;
    const uint32_t ffa_interval = 6000;

    uint16_t previous_sample;
    bool sample_peak;
    uint16_t sample_average;
    uint8_t sample_threshold;
    uint16_t sound_value;
    uint8_t sound_value_gain;

  public:
    ReAnimator(CRGB rim_leds[NUM_RIM_LEDS], CRGB beam_leds[NUM_BEAM_LEDS], CRGB helm_leds[NUM_HELM_LEDS], uint8_t *rim_hue_type, uint8_t *beam_hue_type, uint16_t led_strip_milliamps);
    void set_selected_rim_hue(uint8_t *rim_hue_type);
    void set_selected_beam_hue(uint8_t *beam_hue_type);
    void set_selected_led_strip_milliamps(uint16_t led_strip_milliamps);
    Pattern get_pattern();
    int8_t set_pattern(Pattern pattern);
    int8_t set_pattern(Pattern pattern, bool disable_autocycle_ffa);
    void increment_pattern();
    Overlay get_overlay(bool is_global);
    int8_t set_overlay(Overlay overlay, bool is_global);
    void increment_overlay(bool is_global);
    void set_sound_value_gain(uint8_t gain);
    bool get_autocycle();
    void set_autocycle(bool autocycle_in);
    bool get_flip_flop_animation();
    void set_flip_flop_animation(bool set_flip_flop_animation_in);
    void reanimate();

  private:
    uint16_t forwards(uint16_t index);
    uint16_t backwards(uint16_t index);

    void run_pattern(Pattern pattern);
    void apply_overlay(Overlay overlay);

    bool is_wait_over(uint16_t interval);
    bool finished_waiting(uint16_t interval);

// ++++++++++++++++++++++++++++++
// ++++++++++ PATTERNS ++++++++++
// ++++++++++++++++++++++++++++++
    void orbit(uint16_t draw_interval, int8_t delta);
    void theater_chase(uint16_t draw_interval, uint16_t(ReAnimator::*f)(uint16_t));
    void accelerate_decelerate_theater_chase(uint16_t(ReAnimator::*f)(uint16_t));
    void running_lights(uint16_t draw_interval, uint16_t(ReAnimator::*f)(uint16_t));
    void accelerate_decelerate_running_lights(uint16_t(ReAnimator::*f)(uint16_t));
    void shooting_star(uint16_t draw_interval, uint8_t star_size, uint8_t star_trail_decay, uint8_t spm, uint16_t(ReAnimator::*f)(uint16_t));
    void cylon(uint16_t draw_interval);

    void solid(uint16_t draw_interval);
    void juggle();
    void mitosis(uint16_t draw_interval, uint8_t cell_size);
    void bubbles(uint16_t draw_interval, uint16_t(ReAnimator::*f)(uint16_t));
    void sparkle(uint16_t draw_interval, bool random_color, uint8_t fade);
    void matrix(uint16_t draw_interval);
    void weave(uint16_t draw_interval);
    void starship_race(uint16_t draw_interval, uint16_t(ReAnimator::*f)(uint16_t));
    void pac_man(uint16_t draw_interval, uint16_t(ReAnimator::*f)(uint16_t));
    void bouncing_balls(uint16_t draw_interval, uint16_t(ReAnimator::*f)(uint16_t));

    void halloween_colors_fade(uint16_t draw_interval);
    void halloween_colors_orbit(uint16_t draw_interval, int8_t delta);

    void sound_ribbons(uint16_t draw_interval);
    void sound_ripple(uint16_t draw_interval, bool trigger);
    void sound_orbit(uint16_t draw_interval);
    void sound_blocks(uint16_t draw_interval, bool trigger);

    void dynamic_rainbow(uint16_t draw_interval);

    void helm(uint16_t draw_interval);
    void tractor_beam(uint16_t draw_interval);

// Pattern Helper Functions
    void process_sound();
    void motion_blur(int8_t blur_num, uint16_t pos, uint16_t(ReAnimator::*f)(uint16_t));
    void fission();




// ++++++++++++++++++++++++++++++
// ++++++++++ OVERLAYS ++++++++++
// ++++++++++++++++++++++++++++++
    void breath(uint16_t interval);
    void flicker(uint16_t interval);
    void glitter(uint16_t chance_of_glitter);
    void fade_randomly(uint8_t chance_of_fade, uint8_t decay);

// ++++++++++++++++++++++++++++++
// ++++++++++ SUPPORT ++++++++++
// ++++++++++++++++++++++++++++++
    void print_dt();

};


#endif
