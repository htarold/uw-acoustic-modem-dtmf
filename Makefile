# For compiling:
CC = avr-gcc
OBJCOPY = avr-objcopy
MCU=atmega328p
PARTNO=m328p
#CFLAGS= -DCOMPILED="\"`date +'Build %H:%M:%S %d/%m/%Y'`\"" -std=c99 -O -g -Wall
CFLAGS= -std=c99 -DF_CPU=16000000UL -O -g -Wall -mmcu=$(MCU)
#DUDEFLAGS=-v -v -F -p $(PARTNO) -cusbasp -F
DUDEFLAGS=-v -v -F -p $(PARTNO) -carduino -P /dev/ttyUSB0 -b 57600

%.list: %.c
	$(CC) -c $(CFLAGS) -E $< |less
%.o:%.c
	$(CC) -c $(CFLAGS) $*.c -o $@
%.elf:%.o
	$(CC) $(CFLAGS) $*.o -o $@
	avr-size $@
modulate.elf:modulate.o sine.o put.o
	$(CC) $(CFLAGS) $^ -o $@
	avr-size $@
moder.elf:moder.o
	$(CC) $(CFLAGS) $^ -o $@
	avr-size $@
test-sdtmf.elf:test-sdtmf.o put.o sdtmf.o sine.o tx-var.o
	$(CC) $(CFLAGS) $^ -o $@
	avr-size $@
test-manual.elf:test-manual.o put.o sdtmf.o sine.o tx-var2.o
	$(CC) $(CFLAGS) $^ -o $@
	avr-size $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@
%.bin: %.hex
	$(OBJCOPY) -I ihex -O binary $< $@
%.program:%.hex
	avrdude $(DUDEFLAGS) -U flash:w:$<

# Copy fuses (if there is a .fuse section in the executable)
%.fuses: %.elf
	$(OBJCOPY) -j .fuse -O binary $*.elf $@
%.lfuse: %.fuses
	dd bs=1  count=1 if=$< of=$@
%.hfuse: %.fuses
	dd bs=1  count=1 skip=1 if=$< of=$@
%.efuse: %.fuses
	dd bs=1  count=1 skip=2 if=$< of=$@
%.putfuses: %.lfuse %.hfuse %.efuse
	avrdude $(DUDEFLAGS) -U hfuse:w:$*.hfuse:r -U lfuse:w:$*.lfuse:r -U efuse:w:$*.efuse:r
%.partno: %.mcu
	sed -e 's/atmega\([0123456789]*\)[a]/m\1/i' \
	    -e 's/attiny\([0123456789]*\)[a]/t\1/i' <$< >$@

%.efuse.dump:
	avrdude $(DUDEFLAGS) -U efuse:r:$@:r
%.hfuse.dump:
	avrdude $(DUDEFLAGS) -U hfuse:r:$@:r
%.lfuse.dump:
	avrdude $(DUDEFLAGS) -U lfuse:r:$@:r

%.eeprom.dump:
	avrdude $(DUDEFLAGS) -U eeprom:r:$@:r
%.flash.dump:
	avrdude $(DUDEFLAGS) -U flash:r:$@:r

%.bitclean:
	rm -f $*.o $*.elf $*.hex core

.PRECIOUS:*.bitclean *.boardclean
%.clean:%.bitclean %.boardclean
	:

# For boards:
%.boardclean:
	rm -f $*.*.gbr $*.*.cnc $*.*.bak* $*.sch~ $*.pcb- $*.net \
	$*.bom $*.cmd *.'sch#' $*.xy $*.bom $*.new.pcb
%.project:
	echo schematics $*.sch >>$@
	echo output-name $* >>$@
	echo elements-dir $(HOME)/share/pcb/pcblib-newlib >>$@
%.pcb: %.sch %.project
	gsch2pcb -v --use-files $*.project

%.avi:%.AVI
	mencoder -o $@ -oac mp3lame -ovc lavc $<
