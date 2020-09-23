#  Skill 10: RTOS - Hardware Interrupt

Author: Nnenna Eze

Date: 2020-09-22
-----

## Summary
This skill proved to be one of the most difficult so far. It required implementing three tasks, and then using rtos functions such as xTaskcreate() in order to schedule each task in the main function. It was simple to implement the alphanumeric display task, as well as the binary task, but it took some debuggin to properly implement the pushbutton task. Finally, I used a pin solely for input (A3) to read from the pushbutton and was able to successfully change the direction of the binary count. I als had an issue with the binary count not changing direction immediately; it would finish counting (either at 15 or 0) and then change direction. I elimitaed a while loop I was using the this task, and fixed the issue. 


## Sketches and Photos
<img src = "https://user-images.githubusercontent.com/44929220/93952831-d6eaab00-fd17-11ea-9dce-1ae8a90ac46f.jpg" width="250" height = "250">

<img src = "https://user-images.githubusercontent.com/44929220/93952840-de11b900-fd17-11ea-8714-862a4b2f14d7.jpg" width="250" height = "250">

[EC444_skill10_video_demo](https://youtu.be/C7Y-YyB9pLc)

## Modules, Tools, Source Used Including Attribution
Scheduling

Tasks

RTOS

Alphanumeric Display

GPIO Pins

Bit Shifting (Binary Count)

LEDs

Pushbutton

## Supporting Artifacts


-----
