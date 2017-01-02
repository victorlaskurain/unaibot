CC=avr-gcc
CXX=avr-g++
STL=/home/victor/PROIEKTUAK/arduino/libraries/stlport
STL=/home/victor/PROIEKTUAK/arduino/libraries/avr-stl/include
CXXFLAGS=-Wall -g -O3 -DF_CPU=$(F_CPU) -mmcu=$(MCU) -std=c++11 -I$(STL)
MCU=atmega328p
F_CPU=16000000UL

OBJCOPY=avr-objcopy
BIN_FORMAT=ihex

PORT=/dev/ttyACM0
BAUD=19200
PROTOCOL=arduino
PART=m328p
AVRDUDE=avrdude -F -V

AR=avr-ar
RM=rm -f

CXXSOURCES=$(wildcard *.cpp)
# CXXSOURCES=main.cpp
OBJECTS=$(patsubst %.cpp,%.o,$(CXXSOURCES))

program.hex: program.elf
program.elf: main.o libserial.a
	$(CXX) $(CXXFLAGS) -L. -s -o $@ main.o -lserial
libserial.a: $(subst main.o,,$(OBJECTS))
	$(AR) rcs $@ $^

.PHONY: all disassemble
all: main.hex serial0.hex

disassemble: program.elf
	avr-objdump -S $<

.PHONY: clean upload simul expand
clean:
	$(RM) *.elf *.hex *.s *.o

upload: program.hex
	$(AVRDUDE) -c $(PROTOCOL) -p $(PART) -P $(PORT) -U flash:w:$<

%.s: %.cpp ; $(CXX) $(CXXFLAGS) -S -o $@ $<

%.o: %.cpp ; $(CXX) $(CXXFLAGS) -c -o $@ $<

%.hex: %.elf ; $(OBJCOPY) -O $(BIN_FORMAT) -R .eeprom $< $@

simul: program.elf
	simulavr -d atmega328 -f program.elf -W 0x20,- -R 0x22,-

expand:
	$(CXX) $(CXXFLAGS) -E -C main.cpp
