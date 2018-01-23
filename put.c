#include <avr/io.h>
#include "put.h"

void put_init(void)
{
  UCSR0B |= _BV(TXEN0) | _BV(RXEN0);
  UBRR0 = 16; /* 57600bps */
}
void putch(char ch)
{
  while (!(UCSR0A & _BV(UDRE0)));
  UDR0 = ch;
}
void putstr(char * s)
{
  while (*s) putch(*s++);
}
void putx(uint8_t x)
{
  putch('0');
  putch('X');
  putch("0123456789ABCDEF"[x>>4]);
  putch("0123456789ABCDEF"[x&0xf]);
}
void putdec(uint16_t d)
{
  char buf[10];
  uint8_t i;
  i = sizeof(buf)-1;
  buf[i--] = '\0';
  do { buf[i--] = "0123456789"[d%10]; } while ((d /= 10));
  putstr(buf + i + 1);
}
