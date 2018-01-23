#ifndef SDTMF_H
#define SDTMF_H

#define SDTMF_CARRIER_HZ 36000L

/*
  Timer 1 is used here so don't use it.  Interrupts must be on.
 */

/* Start carrier */
void sdtmf_init(void);

/* Stop carrier */
void sdtmf_shutdown(void);

#define DTMF_1    0
#define DTMF_2    1
#define DTMF_3    2
#define DTMF_4    3
#define DTMF_5    4
#define DTMF_6    5
#define DTMF_7    6
#define DTMF_8    7
#define DTMF_9    8
#define DTMF_0    9
#define DTMF_STAR 10
#define DTMF_HASH 11
#define DTMF_A    12
#define DTMF_B    13
#define DTMF_C    14
#define DTMF_D    15

/*
  Send a code.  Caller is responsible for duration.  Can send
  another code after this, or _init() or _shutdown().
  Code is from 0 to 15, anything larger will turn off modulation
  (but carrier still on).
 */
void sdtmf_modulate(uint8_t pair);
char sdtmf_info(uint8_t i, uint16_t * lo, uint16_t * hi);

#endif /* SDTMF_H */
