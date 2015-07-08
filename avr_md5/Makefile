#############################################################################
# Makefile for the project arduino-md5test.c
###############################################################################

PROJECT = avrhwrng-md5test
MCU = atmega168
TARGET = avrhwrng-md5test.elf
CPP = avr-g++
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -O2 -g -DF_CPU=16000000UL
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map=avrhwrng-md5test.map

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Objects that must be built in order to link
OBJECTS = avrhwrng-md5test.o md5.o uart.o

## Objects explicitly added by the user
LINKONLYOBJECTS = md5.S

## Build
all: $(TARGET) avrhwrng-md5test.hex avrhwrng-md5test.eep avrhwrng-md5test.lss size

## Compile
%.o: %.cpp
	$(CPP) -c $< $(CFLAGS) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) -o $@

##Link
$(TARGET): $(OBJECTS)
	 $(CPP) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

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
	-rm -rf $(OBJECTS) avrhwrng-byte.elf dep/* avrhwrng-byte.hex avrhwrng-byte.eep avrhwrng-byte.lss avrhwrng-byte.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

