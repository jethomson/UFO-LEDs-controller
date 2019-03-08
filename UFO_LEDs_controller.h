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

enum Pattern {            ORBIT = 0, THEATER_CHASE = 1,
                 RUNNING_LIGHTS = 2, SHOOTING_STAR = 3,
                 CYLON = 4, SOLID = 5, JUGGLE = 6, MITOSIS = 7, 
                 BUBBLES = 8, SPARKLE = 9, MATRIX = 10, WEAVE = 11,
                 STARSHIP_RACE = 12, PAC_MAN = 13, BALLS = 14, 
                 HALLOWEEN_FADE = 15, HALLOWEEN_ORBIT = 16, 
                 SOUND_RIBBONS = 17, SOUND_RIPPLE = 18, SOUND_BLOCKS = 19, SOUND_ORBIT = 20,
                 DYNAMIC_RAINBOW = 21};
enum Overlay {NO_OVERLAY = 0, GLITTER = 1, BREATHING = 2, CONFETTI = 3, FLICKER = 4, FROZEN_DECAY = 5};

#endif

