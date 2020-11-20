#  Skill 30 - Buggy/Crawler

Author: Nnenna Eze

Date: 2020-11-20
-----

## Summary
This quest required setting up the buggy. I had to wire up the ESC (speed controller) and the steering servo. An H-bridge was used to power the steering servo, with the 5V coming from  the red wire of the ESC. The ESC had to be calibrated, by running a neutral signal and waiting 3 seconds. I then set the signal to ~1600 for the car to go at a steady speed. The backwards requirement of this skill was removed, so it is not shown in the demo video. The steering servo is dealt with just like any other servo, with the maximum degree set to 30. By chaning the "angle" variable I was able to turn the wheels left and right while still moving the car at a slow speed.

## Sketches and Photos
<img src="https://user-images.githubusercontent.com/44929220/99856322-bba0ee00-2b56-11eb-8f2a-9b5d7e466deb.jpg" height="300" width="300">

## Modules, Tools, Source Used Including Attribution
Servo

ESC

Buggy

## Supporting Artifacts and References
https://www.hobbywing.com/products/enpdf/QuicRunWP1625-WP860-WP1060.pdf

https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/mcpwm.html#_CPPv413mcpwm_timer_t




-----
