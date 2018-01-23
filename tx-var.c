#include <avr/io.h>
#include <avr/interrupt.h>
#include "tx-var.h"
/*
  We can vary the voltage to the transducer: PWM output on PD6
  (OC0A), read the voltage produced on A6 (ADC6).
 */

void tx_var_init(void)
{
  DDRD |= _BV(PD6);                   /* Arduino D6 */
  TCCR0A |= _BV(COM0A1);
  TCCR0A &= ~_BV(COM0A0);
  TCCR0A |= _BV(WGM01);
  TCCR0A |= _BV(WGM00);
  TCCR0B = 0;
  OCR0A = 128;
  TCCR0B |= _BV(CS01);                /* /8 XXX */
  TIMSK0 |= _BV(OCIE0A);              /* Change duty at a safe time */

  /*
    Set up ADC6
   */
  PRR &= ~_BV(PRADC);
  ADMUX = _BV(MUX2)
	| _BV(MUX1)                   /* ADC6 */
	| _BV(REFS0);
  ADCSRB = 0;
  ADCSRA = _BV(ADEN)
         | _BV(ADSC)
         | _BV(ADATE)                 /* free running mode */
	 | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
}

static uint16_t adc_setpoint;
ISR(TIMER0_COMPA_vect)
{
  static uint8_t counter;
  uint16_t adc;

  counter++;
  if (counter & 0x7f) return;

  adc = ADC;
  if (adc == adc_setpoint) return;
  if (adc < adc_setpoint)
    OCR0A++;
  else
    OCR0A--;
}

int8_t tx_var_set(uint16_t volts_x_256)
{
  uint16_t a;
  /*
     5V -> 1024 (Arduino)
   */
  /* adc_setpoint = volts_x_256*1024/5*256; */
  /* adc_setpoint = volts_x_256*4/5; */
  /* adc_setpoint = volts_x_256*0.8; */
  /* adc_setpoint = volts_x_256*0.110011b */
  volts_x_256 /= 2; a = volts_x_256;
  volts_x_256 /= 2; a += volts_x_256;
  volts_x_256 /= 2;
  volts_x_256 /= 2;
  volts_x_256 /= 2; a += volts_x_256;
  volts_x_256 /= 2; a += volts_x_256;
  volts_x_256 /= 2;
  volts_x_256 /= 2;
  volts_x_256 /= 2; a += volts_x_256;
  volts_x_256 /= 2; a += volts_x_256;
  if (a >= 1024) return(-1);
  cli();
  adc_setpoint = a;
  sei();
  return(0);
}

