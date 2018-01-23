#ifndef TX_VAR_H
#define TX_VAR_H
#include <stdint.h>

/*
  Uses timer 0, don't use it for anything else. Also interrupts
  must be on.
 */
void tx_var_init(void);
/*
  Returns -1 on error.  Realistically, cannot output a voltage
  higher than 4.0 volts.
 */
int8_t tx_var_set(uint16_t volts_x_256);
#endif /* TX_VAR_H */
