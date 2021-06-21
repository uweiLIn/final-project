# final-project

I combine HW4 into this time final project code.

In openMV I would run two function one is line detection the other one is apirltag.
If line detection is running then send 'L' through uart to mbed or send 'l'so as apirltag sending 'A' and 'a'
The other data structure in uart share the same code with HW4. 

This is the procedure that my car go.(Result)

Run in line detection mode . If line end, it may meet a obstacle in front of it.
The car would turn 90 degrees and run a little distance  run in clockwise until finish a circle.
Car will go straight moving toward apirltag. After certain distance, car would turn 90 dergrees as my setting, left.
Do line detection again, then whole process end.
