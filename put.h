#ifndef PUT_H
#define PUT_H
#include <stdint.h>
void put_init(void);
void putch(char ch);
void putstr(char * s);
void putx(uint8_t x);
void putdec(uint16_t d);
#endif
