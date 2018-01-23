#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "put.h"
#include "sdtmf.h"
#include "tx-var.h"

static uint8_t havechar(void)
{
  if (UCSR0A & _BV(RXC0)) return(1);
  return(0);
}
static void getchar(void)
{
  char unused;
  while (!havechar()) ;
  unused = UDR0;
}

/*
  Need timer 2 to keep track of time.
 */
void clock_init(void)
{
  OCR2A = 156;                        /* TOP value, 9.984ms */
  OCR2B = 0;
  TCNT2 = 1;
  TCCR2A = _BV(WGM21);                /* CTC mode */
  /* /1024 => 15.625kHz, 64us per tick */
  TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);
  TIMSK2 |= _BV(OCIE2B);
}

/* time in 10s of milliseconds, > 1 hour*/
volatile uint16_t clock = 0;

ISR(TIMER2_COMPB_vect)
{
  static uint8_t count = 0;
  count++;
  if (!(count &3))
    OCR2A = 157;                      /* account for error */
  else
    OCR2A = 156;
  clock++;
}

void delay(uint8_t ms_x_10)
{
  uint16_t lim;
  for (lim = clock + ms_x_10; clock < lim; );
}

int8_t mode_manual;

void transmit(uint8_t duration, uint8_t tones[17])
{
  uint16_t t;
  uint8_t i;

  for (i = 0; i < 17; i++) {
    if (mode_manual) {
      char ch;
      uint16_t lo, hi;
      getchar();
      ch = sdtmf_info(i, &lo, &hi);
      putstr("Tone "); putdec(i); putch(' ');
      if (ch) {
        putdec(lo); putstr(" Hz+");
        putdec(hi); putstr(" Hz (\"");
	putch(ch); putstr("\")");
      } else
        putstr("(no modulation)");
      putstr("\r\n");
    }

    if (tones)
      sdtmf_modulate(tones[i]);
    else
      sdtmf_modulate(i);
    if (!mode_manual) {
      t = clock + duration;
      while (clock < t) ;
    }
  }
}

int
main(void)
{
  uint16_t vcc;
  int8_t i;

  put_init();

  mode_manual = 0;
  for (i = 3; i >= 0; i--) {
    putstr("Manual mode? ");
    putdec(i);
    putstr("\r\n");
    _delay_ms(1000);
    if (havechar()) {
      getchar();
      mode_manual = 1;
      break;
    }
  }

  sei();
  tx_var_init();
  clock_init();
  sdtmf_init();

  putstr("\r\nStarting\r\n");

  /*
    Send out a very long symbol, to indicate startup (preamble).
   */
  tx_var_set(2*256);
  sdtmf_modulate(3);
  while (clock != 400);  /* about 4 seconds long */

  for ( ; ; ) {
    /* 2 volts to 0.25 volts */
    for(vcc = (uint16_t)(2*256); vcc > 32; vcc /= 2) {
      uint8_t duration;
      tx_var_set(vcc);
      putstr("Power at ");
      putdec((vcc * 125L) / 32);
      putstr( "mV\r\n");
      delay(100);   /* wait 1 second, let power settle */
      for (duration = 64; duration > 1; duration /= 2) {
        putstr("Duration = ");
	putdec(duration);
	putstr("0 ms\r\n");
        transmit(duration, 0);
        transmit(duration, 0);
        /* transmit(duration, 0); */
      }
    }
  }

  for ( ; ; );
}
