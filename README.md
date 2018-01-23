
Tue 23 Jan 10:12:02 +08 2018

## Overview
This is a low cost/low power underwater acoustic modem project.
So far it works on the bench, but tank testing has not been
successful due to lack of proper transducers (I've been using
parking sensors).

I'm putting this out there because I don't have time or the
proper transducers to work on this, but I'm happy to answer
questions.

## Description
The modem consists of a separate transmitter and receiver.  The
receiver preamp is described elsewhere.

The transmitter uses an Arduino Nano to modulate a 36kHz carrier
with DTMF.  This is done in software (see sdtmf.c).  36 kHz is
chosen because that is the resonant frequency of the transducers
(parking sensors) I used.
The output is
a frequency modulated square wave on a GPIO pin.  This can be used
to drive a piezo transducer directly, or it can be put through (what
amounts to) an emitter follower to reduce the amplitude (tx-var.*).
To give an idea of the drive required, on the bench, with transducers
on both ends of a water-filled tube, a few hundred mV p-p is needed.

The transmitter code is in regular C, it is not an Arduino sketch.

The receiver preamp is described elsewhere.  Briefly, it
contains a bandpass filter and automatic gain control.

The signal then passes to the PLL (CD4046).  This outputs the
frequency variations of the signal, i.e. the signal is
demodulated and DTMF is recovered.

The DTMF amplitude at this point depends not on the signal
strength but on the depth of modulation, which has been determined by
the transmitter code.

The MT8870 conveniently contains an amplifier stage, which
boosts the signal then decodes it.  An MCU can be added to read
the decoded signal but this is not necessary for testing and
simple applications.  (I will add receiver MCU code when I've
made it presentable).

## Design files
Gerbers are provided but the circuits are simple
enough and the frequencies low enough that everything can be
made on a breadboard.  All the ICs used are old enough that DIP
versions are easily available.

gEDA gschem and pcb were used for the design files, not Eagle
(sorry).

## Setting it up
Build the circuit in dtmf-pll.sch.
Compile test-sdtmf.elf.  Optionally build tx-var.sch and hook up
OC0A (D6) to T1, OC1A (D13) to T4, ADC6 (A6) to T6, and T5 to
the transducer (other end to ground).  Otherwise, just connect
T4 to the transducer (no amplitude control).

Compile test-sdtmf.elf ('make test-sdtmf.elf') and upload it
(fiddle with the Makefile parameters then 'make test-sdtmf.program').

Connect the receive transducer to the preamp and the preamp to
the receiver.  Keep the transmit and receive transmitters close
to each other, submerged.  Power on everything.

Start a serial terminal to the transmitter, 57600 8N1.  Let
the startup questions time out.  If all is well, you'll see the
receiver LEDs flash.

It's best to keep receiver and transmitter isolated, including
power supply.  Preamp gain is very high, the signal can carry
as acoustic or electrical interference.

## Thanks
Questions and comments to htarold@gmail.com.
Tue 23 Jan 11:01:31 +08 2018
