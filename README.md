# UFO-LEDs-controller

Arduino project that animates WS2812 LEDs decorating a UFO prop by an IR remote control

https://jethomson.wordpress.com/2018/10/31/halloween-2018-lilliputian-alien-abduction/

The code can create 22 different animations (referred to as patterns in the code) on the strip of LEDs that run around the rim of the UFO. Some of the patterns can be run in both directions and have variable speed. The pattern names are orbit, theater chase, running lights, shooting star, cylon, solid, juggle, mitosis, bubbles, sparkle, matrix, weave, starship race, pac man, bouncing balls, halloween colors fade, halloween colors orbit, sound ribbons, sound ripple, sound orbit, sound blocks, and dynamic rainbow. There are also effects (called overlays in the code) that can be added to a pattern to make it more dynamic. The overlays are breath, flicker, glitter, and fade randomly. The color of a pattern can be set as a static color, a color that evolves, or a random color on every draw.


Remote Key
----------
[01][02][03][04]  
[05][06][07][08]  
[09][10][11][12]  
[13][14][15][16]  
[17][18][19][20]  
[21][22][23][24]  

01: Up - Increase brightness.  
02: Down - Decrease brightness.  
03: W/WW - Set brightness to default.  
04: Power - On/Off.  
05: IC Set - Set sound gain.  
06: Green Curtain Opening - Select a sound activated pattern.  
07: Blue Curtain Closing - Select a non-sound activated pattern.  
08: Auto - Cycle through all of the patterns.  
09: CS - Selects a random color that changes periodically.  
10: LEFT1 - Orbit CCW (counterclockwise).  
11: RIGHT1 - Orbit CW (clockwise).  
12: Loop - Alternate between the current pattern and the next pattern.  
13: C3 - Select a new dynamic color. The dynamic color evolves from the selected starting color.  
14: LEFT2 - Theater Chase CCW.  
15: RIGHT2 - Theater Chase CW.  
16: Flash - Turns off effects.  
17: C7  - Select a new static rim color.  
18: LEFT3 - Running Lights CCW.  
19: RIGHT3 - Running Lights CW.  
20: Jump - Select the next effect.  
21: C16 - Select a new static beam color.  
22: LEFT4 - Shooting star CCW.  
23: RIGHT4 - Shooting star CW.  
24: Meteor - Cylon/Larson Scanner.  
