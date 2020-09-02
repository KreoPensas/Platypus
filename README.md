# Platypus

This is a 37 key wireless midi keyboard project that I built into a "key-tar" body. 
it uses 7 additional touch sensors for the user interface. 

Uses a teensy 4 for the transmitter.

Any Mega2560 board with Atmel USB chip and mocolufa firmware can be used for the receiver, or can be  run on a Teensy 3.2, 3.5, 3.6, or 4. using the appropriate firmware as stated below.

A pair of xbee radios set for 57600 baud  -or- can be easily adapted to use other wireless serial communication devices.

4x MPR121 12 electrode breakout boards, configured to addresses 0x5A, 0x5B, 0x5C and 0x5D.

1 ADS1015 breeakout board for the pich bend input.

2 100k photoresistors, and 2 LEDs for the pitch bend.

240x240 pixel ST7789 display.

Copper foil tape for the keys.

The radios get attached to the serial port referenced in the Easy Transfer 'begin' line.
You may change these as desired, but change the EasyTransfer 'begin' accordingly.

The files PlatypusXmit.ino and bitmap.h go together and are compiled into the Teensy 4

The file PlatypusRecv.ino is compiled for a  Mega2560

the files PlatypusRecvTeensy and name.c go together and are compiled into a Teensy, select Midi under USB type.

Also requires these libraries:

https://github.com/adafruit/Adafruit_MPR121

https://github.com/adafruit/Adafruit_ADS1X15

https://github.com/madsci1016/Arduino-EasyTransfer


While I support copying my physical build, at the time it is impractical to publish a guide. 
The user interface and pitch bend mechanisms can be readily changed to buttons and a commonly available joystick. 

Equally, with a small amount of effort, any display can be used, you'll just have to change the graphcs code.


![alt text](https://github.com/KreoPensas/Platypus/blob/master/IMG_20200901_182636618.jpg)
![alt text](https://github.com/KreoPensas/Platypus/blob/master/IMG_20200901_182652861.jpg)
![alt text](https://github.com/KreoPensas/Platypus/blob/master/IMG_20200901_182703185_2.jpg)
![alt text](https://github.com/KreoPensas/Platypus/blob/master/IMG-0969.jpg)
