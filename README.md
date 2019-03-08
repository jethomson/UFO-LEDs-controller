# UFO-LEDs-controller

Arduino project that animates WS2812 LEDs decorating a UFO prop by an IR remote control.  

[Write-up](https://jethomson.wordpress.com/2018/10/31/halloween-2018-lilliputian-alien-abduction/)

[A video of all of the patterns combined with the overlays](https://youtu.be/NvCj_wDtI0o)

[Simplified code to make it more easy to demo all of the patterns](https://github.com/jethomson/ReAnimator-demo)

N.B.
--This projects compiles to a large hex file that only fits on an Arduino with 32k of program storage space and that uses a bootloader that is 0.5k (i.e. optiboot with the boot flash section size = 256 words). It should fit fine on an Uno. I developed the code on a Nano, but had to change its fuse settings and bootloader.
--This code hasn't been tested on an LED strip longer than 60 LEDs. It might have problems with an LED strip longer than 255 LEDs.


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
10: LEFT1 - Orbit from right to left (RIGHT1 reversed).  
11: RIGHT1 - Orbit from left to right.  
12: Loop - Alternate between clockwise and counterclockwise for the current pattern.  
13: C3 - Select a new dynamic color. The dynamic color evolves from the selected starting color.  
14: LEFT2 - Theater Chase from right to left.  
15: RIGHT2 - Theater Chase from left to right.  
16: Flash - Turns off effects.  
17: C7  - Select a new static rim color.  
18: LEFT3 - Running Lights from right to left.  
19: RIGHT3 - Running Lights from left to right.  
20: Jump - Select the next effect.  
21: C16 - Select a new static beam color.  
22: LEFT4 - Shooting star from right to left.  
23: RIGHT4 - Shooting star from left to right.  
24: Meteor - Cylon/Larson Scanner.  
