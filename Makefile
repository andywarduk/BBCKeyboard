ARDUINO_DIR=/usr/share/arduino
BOARD_TAG=micro
ARDUINO_PORT=/dev/ttyACM0
ARDUINO_LIBS=
USER_LIB_PATH=

default: all debug disas

include $(ARDUINO_DIR)/Arduino.mk

debug: $(OBJDIR)/$(TARGET).lss $(OBJDIR)/$(TARGET).sym

disas: $(OBJDIR)/$(TARGET).hexd $(OBJDIR)/$(TARGET).elfd

$(OBJDIR)/%.hexd: $(OBJDIR)/%.hex
	$(OBJDUMP) -j .sec1 -d -m avr5 $< > $@

$(OBJDIR)/%.elfd: $(OBJDIR)/%.elf
	$(OBJDUMP) -S $< > $@
