#!/bin/bash

read -p "Reset atmega16u2 and press return"

sudo dfu-programmer atmega16u2 erase
sudo dfu-programmer atmega16u2 flash Arduino-usbserial-atmega16u2-Uno-Rev3.hex 
sudo dfu-programmer atmega16u2 reset

echo "Unplug usb"

