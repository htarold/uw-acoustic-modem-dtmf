/*

Code assumes carrier is 36kHz, corresponding to 444 ticks of
timer 1 (no prescaler).

Fast PWM mode 14 is used, which has ICR1 as the TOP value.  PB1(OC1A)
is configured as the output.  OCR1A, which controls the duty
cycle, is always set to half of ICR1 (50% duty cycle always).

36kHz corresponds to ICR1 holding value of 444.  In effect, ICR1
is the period of the waveform in ticks.  Changing this value
is the same as modulating the frequency.

However, the TOP value in ICR1 can only be modified at a safe
time, and the safest time is just after the timer overflows.

An ISR is set up to run whenever the timer overflows.  This ISR
is responsible for updating the value in ICR1 i.e. the period
for the next time step:

There are 2 angles th_lo and th_hi defined, which correspond
to the current angles of the low and high tones.

At each timer tick, th_lo and th_hi are incremented to reflect the
new angle of their respective tones.  The sine of each of the new
angles is taken; these are added to the base carrier period
(scaling as needed) to form the new output frequency for that
time step.

Some arithmetic must be done in order to determine how much to
increment th_lo and th_hi.  It cannot be a compile-time
constant, because the time elapsed since the last overflow event
is not constant.  The increment is calculated like this:
  increment = actual_period_ticks/(F_CPU/hz) in full circles.
The sine table uses 256 slivs per circle, so:
  increment = 256 * actual_period_ticks * hz / F_CPU in slivs
The increment will be very small, and will accumulate error, so
it is scaled yet again:
  increment = 16*256*256*actual_period_ticks*hz/F_CPU in scaled slivs
Actual_period_ticks is factored out, and the rest is stored in an
array as d_lo and d_hi fields in the dtmf_info structure.

By always incrementing th_lo and th_hi, the phase is maintained
when a new tone pair is used.

Care must be taken so that the arithmetic does not overflow.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "sdtmf.h"
#include "sine.h"

/*
  By using a PWM mode, we can leave the carrier on even if not
  modulating.  Otherwise we have to manually twiddle the output.
 */
void sdtmf_init(void)
{
  /* Fast PWM mode 14! */
  TCCR1A = _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12);
  TCCR1A |= _BV(COM1A1);              /* = pin PB1 */
  ICR1 = (F_CPU/SDTMF_CARRIER_HZ);    /* = 444 for 36kHz & 16MHz */
  OCR1A = ICR1 / 2;                   /* ALWAYS 50% duty */
  DDRB |= _BV(PB1);
  TIMSK1 |= _BV(TOIE1);               /* Enable overflow interrupt */
  TCCR1B |= _BV(CS10);                /* /1 */
}

void sdtmf_shutdown(void) { TCCR1B &= ~_BV(CS10); }

/* These are set by main thread */
static uint16_t dtheta_lo_x16_top, dtheta_hi_x16_top;
#define SHALLOW 32
ISR(TIMER1_OVF_vect)
{
  static uint16_t top = (F_CPU/SDTMF_CARRIER_HZ);
  static uint16_t th_lo, th_hi;
  uint16_t dtheta;
  int16_t modulation_lo, modulation_hi;
  /*
    Update the register values BEFORE calculating new values.
   */
  ICR1 = top;                         /* Full period */
  OCR1A = top/2;                      /* 50% duty cycle */

  dtheta = (top * dtheta_lo_x16_top)/16;
  th_lo += dtheta;                    /* lo's new angle */
  modulation_lo = sine(th_lo>>8);

  /*
    When received, both tones need to be of approx. equal
    amplitude, but the higher tones always come out attenuated.
    We try to equalise it here, but can't control the receiver's
    response.  We try anyway.
   */
  dtheta = (top * dtheta_hi_x16_top)/16;
  th_hi += dtheta;                    /* hi's new angle */
  modulation_hi = sine(th_hi>>8);
  /* This seems to equalise a bit better: */
  if (dtheta_hi_x16_top/2 > dtheta_lo_x16_top)
    modulation_hi *= 2;
  else
    modulation_hi = modulation_hi + modulation_hi/2;

  top = (F_CPU/SDTMF_CARRIER_HZ)
      + (modulation_lo + modulation_hi)/SHALLOW;
}

/*
  "Standard" order based on MT8870 output pins.
  d_lo is the DTMF low tone, in ticks, prescaled to allow easy
  computation of the new low tone angle when the timer
  ticks.  Similarly for d_hi.
 */
static struct {
  uint16_t lo, hi;                    /* frequencies in Hz */
  uint8_t d_lo, d_hi;                 /* angular increments, scaled */
  char name[2];
} dtmf_info[16] = {
#define DTHETA_X16_TOP(hz) (uint16_t)((hz*16.0*65536.0/F_CPU)+0.5)
#define RECORD(hz_lo, hz_hi, name) \
{ hz_lo, hz_hi, DTHETA_X16_TOP(hz_lo), DTHETA_X16_TOP(hz_hi), name, }
  RECORD(697, 1209, "1"),             /* code 0001 */
  RECORD(697, 1336, "2"),             /* code 0010 */
  RECORD(697, 1447, "3"),             /* code 0011 */
  RECORD(770, 1209, "4"),             /* code 0100 */
  RECORD(770, 1336, "5"),             /* code 0101 */
  RECORD(770, 1447, "6"),             /* code 0110 */
  RECORD(852, 1209, "7"),             /* code 0111 */
  RECORD(852, 1336, "8"),             /* code 1000 */
  RECORD(852, 1447, "9"),             /* code 1001 */
  RECORD(941, 1336, "0"),             /* code 1010 */
  RECORD(941, 1209, "*"),             /* code 1011 */
  RECORD(941, 1447, "#"),             /* code 1100 */
  RECORD(697, 1633, "A"),             /* code 1101 */
  RECORD(770, 1633, "B"),             /* code 1110 */
  RECORD(852, 1633, "C"),             /* code 1111 */
  RECORD(941, 1633, "D"),             /* code 0000 */
};

char sdtmf_info(uint8_t i, uint16_t * lo, uint16_t * hi)
{
  if (i >= sizeof(dtmf_info)/sizeof(*dtmf_info)) return(0);
  *lo = dtmf_info[i].lo;
  *hi = dtmf_info[i].hi;
  return(dtmf_info[i].name[0]);
}

void sdtmf_modulate(uint8_t pair)
{
  uint16_t d_lo, d_hi;

  if (pair < sizeof(dtmf_info)/sizeof(*dtmf_info)) {
    d_lo = dtmf_info[pair].d_lo;
    d_hi = dtmf_info[pair].d_hi;
  } else
    d_lo = d_hi = 0;                  /* stop modulating */

  cli();
  dtheta_lo_x16_top = d_lo;
  dtheta_hi_x16_top = d_hi;
  sei();
}
