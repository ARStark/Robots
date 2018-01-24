# Bluetooth Controlled Robot Ver. 1.1
## Updates from version 1.0
1. Now added PWM control of motors. Allows the user to change speed at which the robot is moving.

This implementation of the Bluetooth Controlled Robot will be the base version of any robot created in the future, unless stated otherwise.

The robot is run by a PIC32MX250F128B, a TB6612 motor driver breakout board, a Bluefruit LE UART module, 2 DC motors and a 4 AA battery pack.

The Pic is running a threading library known as Protothreads. To find out more about Protothreads click [here](http://people.ece.cornell.edu/land/courses/ece4760/PIC32/index_Protothreads.html).

## Controlling the PWM

To control this version of the robot, commands must be sent in the following format

command: *%s %f*

where *%s* is a single character that determines the direction the robot moves and *%f* is a number that determines the duty cycle. The number can be an integer or a floating point number and must range between 1 and 4. If a number is entered above the minimum and maximum threshold values, that value will be changed to 1 and 4 respectively; thhis will be done before the new duty cycle is implemented. The duty cycle only runs within the range of 20% - 80% to prevent the robot from either stopping altogher or moving too fast. Because the robot is controlled with a terminal application, and because a human can only enter in commands so fast, the duty cycle needed to be capped at 80% to prevent the robot from moving so fast forward or backward that it runs into an object. 

## Robot Commands

| Character | Movement |
| --- | --- |
| 'u' | forward |
| 'd' | backward |
| 'l' | left |
| 'r' | right |
| 's' | stop|


