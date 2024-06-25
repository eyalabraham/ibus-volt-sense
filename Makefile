#####################################################################################
#
#  This make file is for compiling the ibus voltage sensor code.
#
#  Use:
#    clean      - clean environment
#    all        - build all outputs
#
#####################################################################################

#------------------------------------------------------------------------------------
# AVRDude and avr-gcc
#------------------------------------------------------------------------------------
PORT = /dev/ttyS5

FRQ = 10000000UL

MCU = atmega328p
PART = m328p
DEVICE = __AVR_ATmega328P__

#MCU = attiny84
#PART = t84
#DEVICE = 

#MCU = attiny85
#PART = t85
#DEVICE = 

#MCU = attiny4313
#PART = t4313
#DEVICE = 

#MCU = atmega1284p
#PART = m1284p
#DEVICE = 

#------------------------------------------------------------------------------------
# project directories
#------------------------------------------------------------------------------------
INCDIR = .
SRCDIR = .
OUTDIR = ./Release

#------------------------------------------------------------------------------------
# build tool and options
#------------------------------------------------------------------------------------
CC = avr-gcc
CCDIR = /home/eyal/data/projects/bin/avr8-gnu-toolchain-linux_x86_64/bin

#OPT = -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=$(MCU) -DF_CPU=$(FRQ) -D$(DEVICE) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"
OPT = -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=$(MCU) -DF_CPU=$(FRQ) -D$(DEVICE)

#------------------------------------------------------------------------------------
# dependencies
#------------------------------------------------------------------------------------
OBJS = ibus_drv.o util.o ibusvsense.o

DEPS = ibus_drv.h util.h sensor_type.h
#_DEPS = $(patsubst %,$(INCDIR)/%,$(DEPS))

#------------------------------------------------------------------------------------
# build targets
#------------------------------------------------------------------------------------
%.o: %.c $(DEPS)
	$(CCDIR)/avr-gcc $(OPT) -c -o $@ $<

all: ibus-voltage-sensor.elf secondary-outputs

ibus-voltage-sensor.elf: $(OBJS)
	$(CCDIR)/avr-gcc -Wl,-Map,$(OUTDIR)/$(basename $@).map -mmcu=$(MCU) -o $(OUTDIR)/$(basename $@).elf $(OBJS)

ibus-voltage-sensor.lss: ibus-voltage-sensor.elf
	$(CCDIR)/avr-objdump -h -S $(OUTDIR)/$< > $(OUTDIR)/$@

ibus-voltage-sensor.hex: ibus-voltage-sensor.elf
	$(CCDIR)/avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex $(OUTDIR)/$<  $(OUTDIR)/$@

ibus-voltage-sensor.eep: ibus-voltage-sensor.elf
	$(CCDIR)/avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex $(OUTDIR)/$<  $(OUTDIR)/$@

sizedummy: ibus-voltage-sensor.elf
	$(CCDIR)/avr-size --format=avr --mcu=$(MCU) $(OUTDIR)/$<

secondary-outputs: ibus-voltage-sensor.lss ibus-voltage-sensor.hex ibus-voltage-sensor.eep sizedummy

prog: ibus-voltage-sensor.hex
	avrdude -P $(PORT) -c dasa -i 40 -p $(PART) -U flash:w:$(OUTDIR)/$<

#------------------------------------------------------------------------------------
# cleanup
#------------------------------------------------------------------------------------
.PHONY: clean

clean:
	rm -f $(OUTDIR)/*.elf
	rm -f $(OUTDIR)/*.lss
	rm -f $(OUTDIR)/*.map
	rm -f $(OUTDIR)/*.hex
	rm -f $(OUTDIR)/*.eep
	rm -f *.o
	rm -f *.bak
