#  Skill 10: RTOS - Hardware Interrupt

Author: Nnenna Eze

Date: 2020-09-22
-----

## Summary
This skill proved to be one of the most difficult so far. It required implementing three tasks, and then using rtos functions such as xTaskcreate() in order to schedule each task in the main function. It was simple to implement the alphanumeric display task, as well as the binary task, but it took some debuggin to properly implement the pushbutton task. Finally, I used a pin solely for input (A3) to read from the pushbutton and was able to successfully change the direction of the binary count. I als had an issue with the binary count not changing direction immediately; it would finish counting (either at 15 or 0) and then change direction. I elimitaed a while loop I was using the this task, and fixed the issue. 


## Sketches and Photos


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
