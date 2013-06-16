#!/bin/bash

PROJECT=triaclight

avr-gcc -Wall -Wextra -pedantic -mmcu=atmega1280 -Os -DF_CPU=16000000UL *.c -o $PROJECT.o 

if [ $? != 0 ]; then exit; fi;

avr-size -C --mcu=atmega1280 $PROJECT.o
avr-objcopy -j .text -j .data -O ihex $PROJECT.o $PROJECT.hex
