#include <MIDI.h>

#include <EasyTransfer.h>

EasyTransfer wireless;


MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, midi5pin);


struct RECEIVE_DATA_STRUCTURE {
  uint8_t statusByte;
  uint8_t note;
  uint8_t velocity;
  uint8_t pressure;
  uint8_t controller;
  uint8_t controlValue;
  uint8_t MSB;
  uint8_t LSB;
};

boolean USB = true;
int cable = 0;

RECEIVE_DATA_STRUCTURE packet;

void setup() {
  pinMode(22, INPUT_PULLUP);
  midi5pin.begin(MIDI_CHANNEL_OMNI);
  Serial1.begin(57600);
  wireless.begin(details(packet), &Serial1);
}

byte channel ;

void loop() {
  USB = digitalRead(22);
  if (USB == true) {
    sendUSB();
  }
  if (USB == false) {
    send5Pin();
  }

}

void sendUSB()  {
  if (wireless.receiveData()) {
    channel = (bitRead(packet.statusByte,0)+1);
    if (packet.statusByte > 0x7F && packet.statusByte < 0x90) { // note off
    usbMIDI.sendNoteOff(packet.note,0,channel,cable);
    }
    if (packet.statusByte > 0x8F && packet.statusByte < 0xA0) { // note on
      usbMIDI.sendNoteOn(packet.note,packet.velocity,channel,cable);      
    }
    if  (packet.statusByte > 0xAF && packet.statusByte < 0xC0) { //continuous controller
      usbMIDI.sendControlChange(packet.controller,packet.controlValue,channel,cable);
    }
    if  (packet.statusByte > 0xCF && packet.statusByte < 0xE0) { //channel pressure
      usbMIDI.sendAfterTouch(packet.pressure,channel,cable);
    }
    if  (packet.statusByte > 0xDF && packet.statusByte < 0xF0) { //pitch bend
      usbMIDI.send(packet.statusByte,packet.MSB, packet.LSB,channel,cable);
    }   
  }
}
void send5Pin() {
  if (wireless.receiveData()) {
    channel = (bitRead(packet.statusByte,0)+1);
    if (packet.statusByte > 0x7F && packet.statusByte < 0x90) { // note off
    midi5pin.sendNoteOff(packet.note,0,channel);
    }
    if (packet.statusByte > 0x8F && packet.statusByte < 0xA0) { // note on
      midi5pin.sendNoteOn(packet.note,packet.velocity,channel);      
    }
    if  (packet.statusByte > 0xAF && packet.statusByte < 0xC0) { //continuous controller
      midi5pin.sendControlChange(packet.controller,packet.controlValue,channel);
    }
    if  (packet.statusByte > 0xCF && packet.statusByte < 0xE0) { //channel pressure
      midi5pin.sendAfterTouch(packet.pressure,channel);
    }
    if  (packet.statusByte > 0xDF && packet.statusByte < 0xF0) { //pitch bend
      midi5pin.send(packet.statusByte,packet.MSB,packet.LSB,channel);  
    }   
  }
}
