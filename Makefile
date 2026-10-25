MCU = attiny85
F_CPU = 8000000UL
CC = avr-gcc
CFLAGS = -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
DEPFLAGS = -MMD -MP

SRC_DIR = src
BUILD_DIR = build

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

TARGET = main
PROGRAMMER = avrispmkII

# Fuse values, decoded in README.md ("Fuse settings")
LFUSE = 0xe2
HFUSE = 0xd7
EFUSE = 0xff

all: $(BUILD_DIR)/$(TARGET).hex

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp Makefile | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	avr-objcopy -O ihex -R .eeprom $< $@

-include $(OBJECTS:.o=.d)

upload: $(BUILD_DIR)/$(TARGET).hex
	avrdude -p $(MCU) -c $(PROGRAMMER) -U flash:w:$<:i

read-fuses:
	avrdude -p $(MCU) -c $(PROGRAMMER) -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

fuses:
	avrdude -p $(MCU) -c $(PROGRAMMER) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m

# Boot counter: 16-bit little-endian word at EEPROM address 0, printed in decimal
read-counter:
	@avrdude -p $(MCU) -c $(PROGRAMMER) -U eeprom:r:-:r -q -q | od -An -tu2 -N2 | tr -d ' '

.PHONY: all upload read-fuses fuses read-counter clean

clean:
	rm -rf $(BUILD_DIR)/*
