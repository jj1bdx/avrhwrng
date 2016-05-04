##############################################################################
# Makefile for building libavrtools.a
###############################################################################

## NOTE: GNU Make only

## General Flags
PROJECT = avrtools
MCU = atmega328p
TARGET = libavrtools.a
CC = avr-gcc
CXX = avr-g++
AS = avr-as
AR = avr-ar

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
LDFLAGS += -Wl,-Map=avr-lcdtest.map

## Archiver operation flags
ARFLAGS = rcs

## Objects that must be built in order to link
OBJECTS = \
    Analog2Digital.o \
    I2cLcd.o \
    I2cMaster.o \
    InitSystem.o \
    MemUtils.o \
    Pwm.o \
    Reader.o \
    RingBuffer.o \
    SPI.o \
    SimpleDelays.o \
    SystemClock.o \
    USART0.o \
    Writer.o \
    abi.o \
    new.o

## Build
all: $(TARGET) size

## Compile

.c.o:
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) -c $< $(CXXFLAGS) $(LDFLAGS) -o $@

.S.o:
	$(CC) -c $< $(ASFLAGS) $(LDFLAGS) -o $@

##Link
$(TARGET): $(OBJECTS)
	 $(AR) $(ARFLAGS) $(TARGET) $(OBJECTS) 

size: ${TARGET}
	@echo
	@avr-size ${TARGET}
#	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) 
