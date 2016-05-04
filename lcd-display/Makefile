##############################################################################
# Makefile for the project avr-adctest
###############################################################################

## NOTE: GNU Make only

## General Flags
PROJECT = avrhwrng-lcd
MCU = atmega328p
TARGET = avrhwrng-lcd.elf
CC = avr-gcc
CXX = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU) 

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -Os -g -DF_CPU=16000000UL -D__AVR_ATmega328P__
CFLAGS += -MD -MP -MT $(*F).o -MF $(@F).d 

CXXFLAGS = $(COMMON)
CXXFLAGS += -Wall -Os -g -DF_CPU=16000000UL -D__AVR_ATmega328P__
CXXFLAGS += -MD -MP -MT $(*F).o -MF $(@F).d 

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map=avrhwrng-lcd.map

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Objects that must be built in order to link
OBJECTS = avrhwrng-lcd.o 

## Objects explicitly added by the user
## CAUTION: build libavrtools.a first
LINKONLYOBJECTS = AVRTools/libavrtools.a

## Build
all: $(TARGET) avrhwrng-lcd.hex avrhwrng-lcd.eep avrhwrng-lcd.lss size

## Compile

.c.o:
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) -c $< $(CXXFLAGS) $(LDFLAGS) -o $@

.S.o:
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

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) avrhwrng-lcd.elf dep/* avrhwrng-lcd.hex avrhwrng-lcd.eep avrhwrng-lcd.lss avrhwrng-lcd.map
