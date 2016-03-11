# teensy-RTOS-template

A prototype/sandbox for an teensy based eurorack module. 
It requires arduino-teensy toolchain, though very little of the official code is used.

## Compilation
- set up paths in the Makefile
- comment out `delay()` in the `_init_Teensyduino_internal_() ` (file `hardware/teensy/avr/cores/teensy3/pins_teensy.c`) as it doesn't work with FreeRTOS
- set desired PROJECT to be compiled to a name of a subdir in the root of a source directory
- not it should compile and work

## Developing your own app
- `src/` contains basic framework to use
- create a directory with your own code based on an `app_tmpl.h` prototype
