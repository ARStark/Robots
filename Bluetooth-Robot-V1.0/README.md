# Bluetooth Controlled Robot Ver. 1.0

This is the first implementation of the Bluetooth Controlled Robot.

The robot is run by a PIC32MX250F128B, a TB6612 motor driver breakout board, a Bluefruit LE UART module, 2 DC motors and a 4 AA battery pack.

The Pic is running a threading library known as Protothreads. To find out more about Protothreads click [here](http://people.ece.cornell.edu/land/courses/ece4760/PIC32/index_Protothreads.html).

The robot can be controlled from any terminal application that can connect to Bluetooth Low Energy devices. By sending single characters from your terminal application you can control the direction in which the robot moves.


## Robot Commands


| Character | Movement |
| --- | --- |
| 'u' | forward |
| 'd' | backward |
| 'l' | left |
| 'r' | right |
| 's' | stop|

## Robot Schematic 
![schematic](https://github.com/ARStark/Robots/blob/master/Bluetooth-Robot-V1.0/bluetooth_robot_2WD_schematic_final.jpg)
