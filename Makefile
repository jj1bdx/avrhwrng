##############################################################################
# Makefile for the project avrhwrng-v2
###############################################################################

## NOTE: GNU Make only

## General Flags
PROJECT = avrhwrng-v2
#MCU = atmega168
MCU = atmega328p
TARGET = avrhwrng-v2.elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)
# For OS X HomeBrew avr-gcc
COMMON += -I/usr/local/Cellar/avr-libc/1.8.1/avr/include
COMMON += -L/usr/local/Cellar/avr-libc/1.8.1/avr/lib
#COMMON += -L/usr/local/Cellar/avr-binutils/2.25/avr/lib
COMMON += -B/usr/local/Cellar/avr-libc/1.8.1/avr/lib/avr5

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -O3 -g -DF_CPU=16000000UL
CFLAGS += -MD -MP -MT $(*F).o -MF $(@F).d 

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map=avrhwrng-v2.map

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Objects that must be built in order to link
OBJECTS = avrhwrng-v2.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) avrhwrng-v2.hex avrhwrng-v2.eep avrhwrng-v2.lss size

## Compile

%.o: %.c
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) -o $@

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size ${TARGET}
#	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) avrhwrng-v2.elf dep/* avrhwrng-v2.hex avrhwrng-v2.eep avrhwrng-v2.lss avrhwrng-v2.map
