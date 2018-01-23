#ifndef SINE_H
#define SINE_H
#include <avr/pgmspace.h>
extern const uint8_t sintab[] PROGMEM;
/*
  There are 256 slivers in a circle.
 */
extern int8_t sine(uint8_t slivers);
#endif /* SINE_H */
