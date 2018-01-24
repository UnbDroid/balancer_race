# Balancing Robot Race - RoboGames 2019

This is the Git repository containing all documents, code and schematics
used in the construction of DROID's robot.

### Cross-Compiling

Available at ```/ccdir```, the ```compile_rasp.sh``` file contains a script for
cross-compiling C code for the Raspberry Pi. It is a much faster process
than compiling code directly at the Raspberry.

Run ```$ sudo apt install gcc-arm-linux-gnueabi``` to install the ARM11 GCC.

All the library files needed must be provided. In this particular case,
we put them all in the same directory as the main code.

### Main Code and Threading

The main code, available at the ```main.c``` file under ```/code```, uses the ```pthread```
wrapper provided by the WiringPi library to parallelize the execution of the
code modules. Reference and documentation available at [WiringPi website](http://www.wiringpi.com).
