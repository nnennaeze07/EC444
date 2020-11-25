#  Skill 31 - Lidar

Author: Nnenna Eze

Date: 2020-11-20
-----

## Summary
The Lidar is a type of sensor that helps the buggy avoid obstacles. My particular sensor was the LIDAR v3. It requires an I2C connection, and had 6 wire connections (only 4 of which had to be attached to the esp). The black wire is connected to ground and the red wire connected to 5V. There are also green and blue wires, which are connected to the SCL and SDA pins on the ESP. The orange and yellow wires do not need to be connected. Since I was working with the Garmin Lidar v3, the code required repeatedly reading the register 0x01 until the LSB went low. 

## Sketches and Photos


## Modules, Tools, Source Used Including Attribution
Lidar

I2C

Buggy

## Supporting Artifacts and References
http://static.garmin.com/pumac/LIDAR_Lite_v3_Operation_Manual_and_Technical_Specifications.pdf

https://www.robotshop.com/community/blog/show/lidar-lite-laser-rangefinder-simple-arduino-sketch-of-a-180-degree-radar

https://learn.sparkfun.com/tutorials/lidar-lite-v3-hookup-guide/all

https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html




-----
