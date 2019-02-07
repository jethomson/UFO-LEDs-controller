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

#ifndef UFO_LEDS_CONTROLLER_H
#define UFO_LEDS_CONTROLLER_H

#include <FastLED.h>
#include <stdint.h>
#include "Arduino.h"

//#define UFO_DEBUG

#ifdef UFO_DEBUG
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINTDEC(x)     Serial.print (x, DEC)
 #define DEBUG_PRINTHEX(x)     Serial.print (x, HEX)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTHEX(x)
 #define DEBUG_PRINTLN(x)
#endif


#define NUM_RIM_LEDS 52
#define NUM_BEAM_LEDS 24
#define NUM_HELM_LEDS 7 
#define LED_STRIP_VOLTAGE 5
#define SOUND_VALUE_GAIN_INITIAL 1
#define HUE_ALIEN_GREEN 112

enum Pattern {            ORBIT_LEFT = 0,          ORBIT_RIGHT = 1,
                  THEATER_CHASE_LEFT = 2,  THEATER_CHASE_RIGHT = 3,
                 RUNNING_LIGHTS_LEFT = 4, RUNNING_LIGHTS_RIGHT = 5,
                  SHOOTING_STAR_LEFT = 6,  SHOOTING_STAR_RIGHT = 7,
                 CYLON = 8, SOLID = 9, JUGGLE = 10, MITOSIS = 11, 
                 BUBBLES = 12, SPARKLE = 13, MATRIX = 14, WEAVE = 15,
                 STARSHIP_RACE = 16, PAC_MAN = 17, BALLS = 18, 
                 HALLOWEEN_FADE = 19, HALLOWEEN_ORBIT = 20, 
                 SOUND_RIBBONS = 21, SOUND_RIPPLE = 22, SOUND_BLOCKS = 23, SOUND_ORBIT = 24,
                 DYNAMIC_RAINBOW = 25};
enum Overlay {NO_OVERLAY = 0, GLITTER = 1, BREATH = 2, CONFETTI = 3, FLICKER = 4, DECAY = 5};

#endif

