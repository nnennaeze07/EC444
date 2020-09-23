#  Skill 8: Alphanumeric Display

Author: Nnenna Eze

Date: 2020-09-22
-----

## Summary
The Adafruit 14-LED alphanumeric display was used for this to display a word inputted by the user. Initially the wiring was an obstacle, but I soon realized that because the esp32 contained pull up resistors within the pins SCL and SDA, there was no need for external resistors. In the code, I used a table found on an Adafruit LEd Backpack github in order to help converteach character into a 16 bit integer that could then be displayed. I also used scanf to receive input from a user, and then stored this into a varibale that I used to index into the table and find the corresponding 16 bit integer. 


## Sketches and Photos
<img src = "https://user-images.githubusercontent.com/44929220/93952843-e0741300-fd17-11ea-8dd0-0b70073d7840.jpg" width="250" height = "250">

<img src = "https://user-images.githubusercontent.com/44929220/93952850-e2d66d00-fd17-11ea-9c1d-e8e06d7cc83b.jpg" width="250" height = "250">


## Modules, Tools, Source Used Including Attribution
Alphanumeric Display

Pull up Resistors

## Supporting Artifacts


-----
