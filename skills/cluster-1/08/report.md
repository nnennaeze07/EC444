#  Skill 8: Alphanumeric Display

Author: Nnenna Eze

Date: 2020-09-22
-----

## Summary
The Adafruit 14-LED alphanumeric display was used for this to display a word inputted by the user. Initially the wiring was an obstacle, but I soon realized that because the esp32 contained pull up resistors within the pins SCL and SDA, there was no need for external resistors. In the code, I used a table found on an Adafruit LEd Backpack github in order to help converteach character into a 16 bit integer that could then be displayed. I also used scanf to receive input from a user, and then stored this into a varibale that I used to index into the table and find the corresponding 16 bit integer. 


## Sketches and Photos


## Modules, Tools, Source Used Including Attribution
Alphanumeric Display
Pull up Resistors

## Supporting Artifacts


-----
