#include <MIDI.h>

#include <EasyTransfer.h>

EasyTransfer wireless;

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midiA);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, midiB);


struct RECEIVE_DATA_STRUCTURE {
  uint8_t statusByte;
  uint8_t note;
  uint8_t velocity;
  uint8_t pressure;
  uint8_t controller;
  uint8_t controlValue;
  uint8_t LSB;
  uint8_t MSB;
};

boolean USB = true;

RECEIVE_DATA_STRUCTURE packet;

void setup() {
  pinMode(22, INPUT_PULLUP);
  midiA.begin(MIDI_CHANNEL_OMNI);
  midiB.begin(MIDI_CHANNEL_OMNI);
  Serial1.begin(57600);
  Serial3.begin(57600);
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
    midiA.sendNoteOff(packet.note,0,channel);
    }
    if (packet.statusByte > 0x8F && packet.statusByte < 0xA0) { // note on
      midiA.sendNoteOn(packet.note,packet.velocity,channel);      
    }
    if  (packet.statusByte > 0xAF && packet.statusByte < 0xC0) { //continuous controller
      midiA.sendControlChange(packet.controller,packet.controlValue,channel);
    }
    if  (packet.statusByte > 0xCF && packet.statusByte < 0xE0) { //channel pressure
      midiA.sendAfterTouch(packet.pressure,channel);
    }
    if  (packet.statusByte > 0xDF && packet.statusByte < 0xF0) { //pitch bend
      Serial.write(packet.statusByte);
      Serial.write(packet.MSB);
      Serial.write(packet.LSB);
    }   
  }
}
void send5Pin() {
  if (wireless.receiveData()) {
    channel = (bitRead(packet.statusByte,0)+1);
    if (packet.statusByte > 0x7F && packet.statusByte < 0x90) { // note off
    midiB.sendNoteOff(packet.note,0,channel);
    }
    if (packet.statusByte > 0x8F && packet.statusByte < 0xA0) { // note on
      midiB.sendNoteOn(packet.note,packet.velocity,channel);      
    }
    if  (packet.statusByte > 0xAF && packet.statusByte < 0xC0) { //continuous controller
      midiB.sendControlChange(packet.controller,packet.controlValue,channel);
    }
    if  (packet.statusByte > 0xCF && packet.statusByte < 0xE0) { //channel pressure
      midiB.sendAfterTouch(packet.pressure,channel);
    }
    if  (packet.statusByte > 0xDF && packet.statusByte < 0xF0) { //pitch bend
      Serial2.write(packet.statusByte);
      Serial2.write(packet.MSB);
      Serial2.write(packet.LSB);
    }   
  }
}
