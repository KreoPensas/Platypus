#include <ST7735_t3.h> // Hardware-specific library
#include <ST7789_t3.h>
//#include <Adafruit_GFX.h>
//#include <Fonts/FreeSans18pt7b.h>
//#include <Fonts/FreeSans12pt7b.h>
#include <EasyTransfer.h>
#include <SPI.h>
#include "bitmap.h"
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "Adafruit_MPR121.h"

#define upper  36
#define lower 6

#define TFT_MISO  12
#define TFT_MOSI  11  //a12
#define TFT_SCK   13  //a13
#define TFT_DC   9
#define TFT_CS   -1
#define TFT_RST  8

#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFC00
#define REDORANGE  0xF9E0
#define YELLOWGREEN  0xDFE0
#define GRASS  0x074E
#define YELLOWORANGE 0xFE20
#define TEAL 0x07F9
#define PERIWINKLE  0x921F
#define PURPLE  0x8010
#define SKY  0x05FF
#define ROYAL  0x681F
#define LAVENDER 0xD97F
#define HOTPINK 0xF810
#define GRAY 0x8C51

EasyTransfer wireless;
ST7789_t3 tft = ST7789_t3(TFT_CS,  TFT_DC, TFT_RST);
Adafruit_ADS1115 ads;
Adafruit_MPR121 cap0 = Adafruit_MPR121();
Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();
Adafruit_MPR121 cap3 = Adafruit_MPR121();



struct SEND_DATA_STRUCTURE {
  uint8_t statusByte;
  uint8_t note;
  uint8_t velocity;
  uint8_t pressure;
  uint8_t controller;
  uint8_t controlValue;
  uint8_t LSB;
  uint8_t MSB;
};
int center = 8192;
int pitchValue;
int16_t adc0, adc3;
byte octaves [9] = {0x00, 0x0C, 0x18, 0x24, 0x30, 0x3C, 0x48, 0x54, 0x60};
byte noteOn0 [12] = {};
byte notes0[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
byte noteOn1 [12] = {};
byte notes1[12] = {0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
byte noteOn2 [12] = {};
byte notes2[12] = {0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23};
byte noteOn3 ;
byte notes3 = 0x24;
byte scaler [7] = {0, 10, 20, 30, 40, 50, 60};
int scale = 0;
int octave = 3;
boolean pitchOff = false;
byte channel = 0;
byte modulation = 0;
boolean chPress = false;
boolean systemMenu = false;
byte Velocity = 119;
byte VelocityA = 119;
byte VelocityB = 119;
byte VelocityC = 119;
byte VelocityD = 119;
byte velCap = 119;
byte pres;

boolean busy = false;
elapsedMillis pause;

SEND_DATA_STRUCTURE packet;

void setup() {
  Serial1.begin(57600);
  Serial.begin(57600); //debug
  pinMode(15, INPUT_PULLUP); 
  wireless.begin(details(packet), &Serial1);
  Wire.begin();
  Wire.setClock(400000);
  ads.setGain(GAIN_ONE);
  ads.begin();
  cap0.begin(0x5A);
  cap1.begin(0x5B);
  cap2.begin(0x5C);
  cap3.begin(0x5D);
  keyReset();
  tft.init(240, 240, SPI_MODE2);
  tft.setRotation(2);
  boilerplate();
  pitchValue = center;
}

void loop() {
  noteFire();
  readAnalog();
  userInterface();
}

void boilerplate() {
  tft.fillScreen(BLACK);
  tft.drawBitmap(0, 0, bitmap, 86, 79, YELLOWGREEN);
  tft.setTextSize(3);
  tft.setCursor(90, 24);
  tft.setTextColor(HOTPINK);
  tft.print("Platypus");
  tft.setTextSize(2);
  tft.setCursor(110, 55);
  tft.setTextColor(YELLOW);
  tft.print("Key-Tar");
  tft.drawRect(0, 90, 240, 29, WHITE);
  tft.setCursor(60, 94);
  tft.setTextColor(GREEN);
  tft.print("Channel ");
  tft.setCursor(156, 94);
  tft.print(channel + 1);
  tft.drawRect(0, 120, 240, 29, WHITE);
  tft.setCursor(65, 125);
  tft.setTextColor(MAGENTA);
  tft.print("Octave ");
  tft.setCursor (156, 125);
  tft.setTextColor(YELLOWORANGE);
  tft.print(octave);
  tft.drawRect(0, 150, 240, 29, WHITE);
  tft.setCursor(30, 156);
  tft.setTextColor(SKY);
  tft.print("Ch Pressure  ");
  tft.setCursor ( 165, 156);
  if (chPress == true) {
    tft.setTextColor(GREEN);
    tft.print(" On");
  }
  if (chPress == false) {
    tft.setTextColor(RED);
    tft.print(" Off");
  }
  tft.drawRect(0, 180, 240, 29, WHITE);
  tft.setCursor(45, 186);
  tft.setTextColor(TEAL);
  tft.print("Velocity");
  tft.setCursor( 150, 186);
  tft.setTextColor(GRASS);

  if (scale == 1) {
    tft.print("Touch");
  }
  if (scale == 0) {
    tft.print(Velocity);
  }
}

void userInterface() {
  panic();
  if (systemMenu == false) {
    //modulation
    if (cap3.touched() & (1 << 5)) { // modulaton up
      ++modulation;
      if (modulation > 127) {
        modulation = 127;
      }
      packet.statusByte = 0xB0 | channel;
      packet.controller = 0x01;
      packet.controlValue = modulation;
      wireless.sendData();
    }
    if (cap3.touched() & (1 << 7)) { // modulation down
      --modulation;
      if (modulation > 127) {
        modulation = 0;
      }
      packet.statusByte = 0xB0 | channel;
      packet.controller = 0x01;
      packet.controlValue = modulation;
      wireless.sendData();
    }
    // octave
    if (cap3.touched() & (1 << 1) && pause > 250) { // octave down
      --octave;
      if (octave < 0) {
        octave = 0;
      }
      pause = 0;
      redrawOctave();
    }
    if (cap3.touched() & (1 << 6) && pause > 250) { //octave up
      ++octave;
      if (octave > 9) {
        octave = 9;
      }
      pause = 0;
      redrawOctave();
    }
    //channel
    if (cap3.touched() & (1 << 4) && pause > 250 ) { // channel down
      --channel;
      if (channel > 15) {
        channel = 0;
      }
      pause = 0;
      redrawChannel();
    }
    if (cap3.touched() & (1 << 3) && pause > 250) { // channel up
      ++channel;
      if (channel > 15) {
        channel = 15;
      }
      pause = 0;
      redrawChannel();
    }
    if (cap3.touched() & (1 << 2) && pause > 250) {
      systemMenu = true;
      pause = 0;
      systemScreen();
    }
  }
  //system menu
  if (systemMenu == true) {

    if (cap3.touched() & (1 << 2) && pause > 250 ) {
      systemMenu = false;
      pause = 0;
      boilerplate();
    }
    if (cap3.touched() & (1 << 1) && pause > 250 ) {
      keyReset();
      tft. fillRect(0, 220, 20, 20, RED);
      pause = 0;
    }
    if (cap3.touched() & (1 << 6) && pause > 250 ) {
      keyReset();
      tft. fillRect(0, 220, 20, 20, RED);
      pause = 0;
    }
    if (cap3.touched() & (1 << 5) ) {
      ++Velocity;
      if (Velocity > 127) {
        Velocity = 127;
      }
      redrawVelocity();
    }
    if (cap3.touched() & (1 << 7) ) {
      --Velocity;
      if (Velocity > 127) {
        Velocity = 0;
      }
      redrawVelocity();
    }
    if (cap3.touched() & (1 << 3) ) { //scaler up
      ++scale;
      if (scale > 1) {
        scale = 1;
      }
      tft.fillRect(0, 157, 240, 20, BLACK);
      tft.setCursor(0, 158);
      tft.setTextColor(GRASS);
      tft.print("Touch Vel = ");
      
      if (scale == 1) {
        tft.print("On");
      }
      if (scale == 0) {
        tft.setTextColor(RED);
        tft.print("Off");
      }
    }
    if (cap3.touched() & (1 << 4) ) { //scaler down
      --scale;
      if (scale < 0) {
        scale = 0;
      }
      tft.fillRect(0, 157, 240, 20, BLACK);
      tft.setCursor(0, 158);
      tft.setTextColor(GRASS);
      tft.print("Touch Vel = ");      
      if (scale == 1) {
        tft.print("On");
      }
      if (scale == 0) {
        tft.setTextColor(RED);
        tft.print("Off");
      }
    }
    if (scale == 0) {
      chPress = false;
    }
    if (scale > 0) {
      chPress = true;
    }
  }
}

void noteFire() {
  //begin poll of card A
  for (int i = 0 ; i < 12; i ++) {
    if (cap0.touched() & (1 << i)) {
      if (noteOn0[i] == 0 ) {
        packet.statusByte = 0x90 | channel;
        packet.note = (notes0[i] + octaves[octave]);
        if (scale > 0) {
          int touchCapA = cap0.filteredData(i);
          if (touchCapA > 300 && touchCapA < 500) {
            VelocityA = 127;
          }
          if (touchCapA > 500 && touchCapA < 550) {
            VelocityA = 119;
          }
          if (touchCapA > 550 && touchCapA < 600) {
            VelocityA = 90;
          }
          if (touchCapA > 600 && touchCapA < 650)  {
            VelocityA = 40;
          }
          if (touchCapA > 650 )  {
            VelocityA = 25;
          }
          packet.velocity = VelocityA;
        }
        if ( scale > 0 ) {
          packet.velocity = Velocity;
        }
        busy = true;
        wireless.sendData();
        busy = false;
        noteOn0[i] = 1;
      }
    }
    if (noteOn0[i] == 1) {
      if (chPress == true) {
          int touchCap = cap0.filteredData(i);
          if (touchCap > 300 && touchCap < 500) {
            pres = 127;
          }
          if (touchCap > 500 && touchCap < 550) {
            pres = 119;
          }
          if (touchCap > 550 && touchCap < 600) {
            pres = 90;
          }
          if (touchCap > 600 && touchCap < 650)  {
            pres = 40;
          }
          if (touchCap > 650 )  {
            pres = 25;
          }        
        packet.pressure = pres;
        packet.statusByte = 0xD0 | channel;
        wireless.sendData();
      }
      if (!(cap0.touched() & (1 << i))) {
        packet.statusByte = 0x80 | channel;
        packet.note = (notes0[i] + octaves[octave]);
        packet.velocity = 0x00;
        busy = true;
        wireless.sendData();
        busy = false;
        noteOn0[i] = 0 ;
      }
    }
  }  // end A
  //begin poll of card B
  for (int i = 0 ; i < 12; i ++) {
    if (cap1.touched() & (1 << i)) {
      if (noteOn1[i] == 0 ) {
        packet.statusByte = 0x90 | channel;
        packet.note = (notes1[i] + octaves[octave]);
        if ( scale > 0 ) {
          int touchCapB = cap1.filteredData(i);
          if (touchCapB > 300 && touchCapB < 500) {
            VelocityB = 127;
          }
          if (touchCapB > 500 && touchCapB < 550) {
            VelocityB = 119;
          }
          if (touchCapB > 550 && touchCapB < 600) {
            VelocityB = 90;
          }
          if (touchCapB > 600 && touchCapB < 650)  {
            VelocityB = 40;
          }
          if (touchCapB > 650 )  {
            VelocityB = 25;
          }
          packet.velocity = VelocityB;
        }
        if ( scale > 0 ) {
          packet.velocity = Velocity;
        }
        busy = true;
        wireless.sendData();
        busy = false;
        noteOn1[i] = 1;
      }
    }
    if (noteOn1[i] == 1) {
      if (chPress == true) {
        int touchCap = cap1.filteredData(i);
          if (touchCap > 300 && touchCap < 500) {
            pres = 127;
          }
          if (touchCap > 500 && touchCap < 550) {
            pres = 119;
          }
          if (touchCap > 550 && touchCap < 600) {
            pres = 90;
          }
          if (touchCap > 600 && touchCap < 650)  {
            pres = 40;
          }
          if (touchCap > 650 )  {
            pres = 25;
          }       
        packet.pressure = pres;
        packet.statusByte = 0xD0 | channel;
        wireless.sendData();
      }
      if (!(cap1.touched() & (1 << i))) {
        packet.statusByte = 0x80 | channel;
        packet.note = (notes1[i] + octaves[octave]);
        packet.velocity = 0x00;
        busy = true;
        wireless.sendData();
        busy = false;
        noteOn1[i] = 0 ;
      }
    }
  } //end B
  //begin poll of card C
  for (int i = 0 ; i < 12; i ++) {
    if (cap2.touched() & (1 << i)) {
      if (noteOn2[i] == 0 ) {
        packet.statusByte = 0x90 | channel;
        packet.note = (notes2[i] + octaves[octave]);
        if ( scale > 0) {
          int touchCapC = cap2.filteredData(i);
          if (touchCapC > 300 && touchCapC < 500) {
            VelocityC = 127;
          }
          if (touchCapC > 500 && touchCapC < 550) {
            VelocityC = 119;
          }
          if (touchCapC > 550 && touchCapC < 600) {
            VelocityC = 90;
          }
          if (touchCapC > 600 && touchCapC < 650)  {
            VelocityC = 40;
          }
          if (touchCapC > 650 )  {
            VelocityC = 25;
          }
          packet.velocity = VelocityC;
        }
        if ( scale > 0 ) {
          packet.velocity = Velocity;
        }
        busy = true;
        wireless.sendData();
        busy = false;
        noteOn2[i] = 1;
      }
    }
    if (noteOn2[i] == 1) {
      if (chPress == true) {
        int touchCap = cap2.filteredData(i);
          if (touchCap > 300 && touchCap < 500) {
            pres = 127;
          }
          if (touchCap > 500 && touchCap < 550) {
            pres = 119;
          }
          if (touchCap > 550 && touchCap < 600) {
            pres = 90;
          }
          if (touchCap > 600 && touchCap < 650)  {
            pres = 40;
          }
          if (touchCap > 650 )  {
            pres = 25;
          }       
        packet.pressure = pres;
        packet.statusByte = 0xD0 | channel;
        wireless.sendData();
      }
      if (!(cap2.touched() & (1 << i))) {
        packet.statusByte = 0x80 | channel;
        packet.note = (notes2[i] + octaves[octave]);
        packet.velocity = 0x00;
        busy = true;
        wireless.sendData();
        busy = false;
        noteOn2[i] = 0 ;
      }
    }
  } // end C
  //begin poll of card D
  if (cap3.touched() & (1 << 0)) {
    if (noteOn3 == 0 ) {
      packet.statusByte = 0x90 | channel;
      packet.note = (notes3 + octaves[octave]);
      if (scale > 0) {
        int touchCapD = cap3.filteredData(0);
        if (touchCapD > 300 && touchCapD < 500) {
          VelocityD = 127;
        }
        if (touchCapD > 500 && touchCapD < 550) {
          VelocityD = 119;
        }
        if (touchCapD > 550 && touchCapD < 600) {
          VelocityD = 90;
        }
        if (touchCapD > 600 && touchCapD < 650)  {
          VelocityD = 40;
        }
        if (touchCapD > 650 )  {
          VelocityD = 25;
        }
        packet.velocity = VelocityD;
      }
      if ( scale > 0 ) {
        packet.velocity = Velocity;
      }
      busy = true;
      wireless.sendData();
      busy = false;
      noteOn3 = 1;
    }
  }
  if (noteOn3 == 1) {
    if (chPress == true) {
     int touchCap = cap3.filteredData(0);
          if (touchCap > 300 && touchCap < 500) {
            pres = 127;
          }
          if (touchCap > 500 && touchCap < 550) {
            pres = 119;
          }
          if (touchCap > 550 && touchCap < 600) {
            pres = 90;
          }
          if (touchCap > 600 && touchCap < 650)  {
            pres = 40;
          }
          if (touchCap > 650 )  {
            pres = 25;
          }       
      packet.pressure = pres;
      packet.statusByte = 0xD0 | channel;
      wireless.sendData();
    }
    if (!(cap3.touched() & (1 << 0))) {
      packet.statusByte = 0x80 | channel;
      packet.note = (notes3 + octaves[octave]);
      packet.velocity = 0x00;
      busy = true;
      wireless.sendData();
      busy = false;
      noteOn3 = 0 ;
    }
  } // end D
}

void readAnalog() {
  adc0 = ads.readADC_SingleEnded(0);
  adc3 = ads.readADC_SingleEnded(3);
  int upbend = adc0;
  int downbend = adc3;
  if (upbend < 13000 && downbend < 13000) {
    pitchValue = center;
    if (pitchOff == true) {
      if (busy == false) {
        packet.statusByte = 0xE0 | channel;
        packet.MSB = (pitchValue & 0x7F);
        packet.LSB = (pitchValue >> 7) & 0x7F;
        wireless.sendData();
        pitchOff = false;
      }
    }
  }
  if ( downbend > 13000) {
    if (busy == false) {
      pitchValue = map(downbend, 12000, 25750, 8192, 0);
      pitchValue = constrain(pitchValue, 0, 8192);
      packet.statusByte = 0xE0 | channel;
      packet.MSB = (pitchValue & 0x7F);
      packet.LSB = (pitchValue >> 7) & 0x7F;
      wireless.sendData();
      pitchOff = true;
    }
  }
  if (  upbend > 13000) {
    if (busy == false) {
      pitchValue = map(upbend, 12000, 25750, 8192, 16383);
      pitchValue = constrain(pitchValue, 8192, 16383);
      packet.statusByte = 0xE0 | channel;
      packet.MSB = (pitchValue & 0x7F);
      packet.LSB = (pitchValue >> 7) & 0x7F;
      wireless.sendData();
      pitchOff = true;
    }
  }
}

void redrawChannel () {
  tft.fillRect(155, 91, 30, 27, BLACK);
  tft.setCursor(156, 94);
  tft.setTextColor(GREEN);
  tft.print(channel + 1);
}



void redrawOctave() {
  tft.fillRect(155,  121, 40, 27, BLACK);
  tft.setCursor ( 156, 125);
  tft.setTextColor(YELLOWORANGE);
  tft.print(octave);
}

void redrawVelocity() {
  tft.fillRect(0, 185, 240, 40, BLACK);
  tft.setTextColor(YELLOWGREEN);
  tft.setCursor(0, 186);
  tft.print("Velocity = ");
  tft.print(Velocity);
}

void keyReset() {
  cap0.writeRegister(MPR121_SOFTRESET, 0x63);
  cap0.setThresholds(upper, lower);
  cap0.writeRegister(MPR121_ECR, 0x00);
  cap0.writeRegister(MPR121_MHDR, 0x01);
  cap0.writeRegister(MPR121_NHDR, 0x01);
  cap0.writeRegister(MPR121_NCLR, 0x0E);
  cap0.writeRegister(MPR121_FDLR, 0x00);
  cap0.writeRegister(MPR121_MHDF, 0x01);
  cap0.writeRegister(MPR121_NHDF, 0x05);
  cap0.writeRegister(MPR121_NCLF, 0x01);
  cap0.writeRegister(MPR121_FDLF, 0x00);
  cap0.writeRegister(MPR121_NHDT, 0x00);
  cap0.writeRegister(MPR121_NCLT, 0x00);
  cap0.writeRegister(MPR121_FDLT, 0x00);
  cap0.writeRegister(MPR121_DEBOUNCE, 0x01);
  cap0.writeRegister(MPR121_CONFIG1, 0x20); // default, 16uA charge current
  cap0.writeRegister(MPR121_CONFIG2, 0x20); // 0.5uS encoding, 1ms period
  cap0.writeRegister(MPR121_AUTOCONFIG0, 0x1B);
  cap0.writeRegister(MPR121_UPLIMIT, 205);     // ((Vdd - 0.7)/Vdd) * 256
  cap0.writeRegister(MPR121_TARGETLIMIT, 190); // UPLIMIT * 0.9
  cap0.writeRegister(MPR121_LOWLIMIT, 125);    // UPLIMIT * 0.65
  byte ECR_SETTING = B11001100 ;
  cap0.writeRegister(MPR121_ECR, ECR_SETTING);
  cap1.writeRegister(MPR121_SOFTRESET, 0x63);
  cap1.setThresholds(upper, lower);
  cap1.writeRegister(MPR121_ECR, 0x00);
  cap1.writeRegister(MPR121_MHDR, 0x01);
  cap1.writeRegister(MPR121_NHDR, 0x01);
  cap1.writeRegister(MPR121_NCLR, 0x0E);
  cap1.writeRegister(MPR121_FDLR, 0x00);
  cap1.writeRegister(MPR121_MHDF, 0x01);
  cap1.writeRegister(MPR121_NHDF, 0x05);
  cap1.writeRegister(MPR121_NCLF, 0x01);
  cap1.writeRegister(MPR121_FDLF, 0x00);
  cap1.writeRegister(MPR121_NHDT, 0x00);
  cap1.writeRegister(MPR121_NCLT, 0x00);
  cap1.writeRegister(MPR121_FDLT, 0x00);
  cap1.writeRegister(MPR121_DEBOUNCE, 0x01);
  cap1.writeRegister(MPR121_CONFIG1, 0x20); // default, 16uA charge current
  cap1.writeRegister(MPR121_CONFIG2, 0x20); // 0.5uS encoding, 1ms period
  cap1.writeRegister(MPR121_AUTOCONFIG0, 0x1B);
  cap1.writeRegister(MPR121_UPLIMIT, 205);     // ((Vdd - 0.7)/Vdd) * 256
  cap1.writeRegister(MPR121_TARGETLIMIT, 190); // UPLIMIT * 0.9
  cap1.writeRegister(MPR121_LOWLIMIT, 125);    // UPLIMIT * 0.65
  cap1.writeRegister(MPR121_ECR, ECR_SETTING);
  cap2.writeRegister(MPR121_SOFTRESET, 0x63);
  cap2.setThresholds(upper, lower);
  cap2.writeRegister(MPR121_ECR, 0x00);
  cap2.writeRegister(MPR121_MHDR, 0x01);
  cap2.writeRegister(MPR121_NHDR, 0x01);
  cap2.writeRegister(MPR121_NCLR, 0x0E);
  cap2.writeRegister(MPR121_FDLR, 0x00);
  cap2.writeRegister(MPR121_MHDF, 0x01);
  cap2.writeRegister(MPR121_NHDF, 0x05);
  cap2.writeRegister(MPR121_NCLF, 0x01);
  cap2.writeRegister(MPR121_FDLF, 0x00);
  cap2.writeRegister(MPR121_NHDT, 0x00);
  cap2.writeRegister(MPR121_NCLT, 0x00);
  cap2.writeRegister(MPR121_FDLT, 0x00);
  cap2.writeRegister(MPR121_DEBOUNCE, 0x01);
  cap2.writeRegister(MPR121_CONFIG1, 0x20); // default, 16uA charge current
  cap2.writeRegister(MPR121_CONFIG2, 0x20); // 0.5uS encoding, 1ms period
  cap2.writeRegister(MPR121_AUTOCONFIG0, 0x1B);
  cap2.writeRegister(MPR121_UPLIMIT, 205);     // ((Vdd - 0.7)/Vdd) * 256
  cap2.writeRegister(MPR121_TARGETLIMIT, 190); // UPLIMIT * 0.9
  cap2.writeRegister(MPR121_LOWLIMIT, 125);    // UPLIMIT * 0.65
  cap2.writeRegister(MPR121_ECR, ECR_SETTING);
  cap3.writeRegister(MPR121_SOFTRESET, 0x63);
  cap3.setThresholds(upper, lower);
  cap3.writeRegister(MPR121_ECR, 0x00);
  cap3.writeRegister(MPR121_MHDR, 0x01);
  cap3.writeRegister(MPR121_NHDR, 0x01);
  cap3.writeRegister(MPR121_NCLR, 0x0E);
  cap3.writeRegister(MPR121_FDLR, 0x00);
  cap3.writeRegister(MPR121_MHDF, 0x01);
  cap3.writeRegister(MPR121_NHDF, 0x05);
  cap3.writeRegister(MPR121_NCLF, 0x01);
  cap3.writeRegister(MPR121_FDLF, 0x00);
  cap3.writeRegister(MPR121_NHDT, 0x00);
  cap3.writeRegister(MPR121_NCLT, 0x00);
  cap3.writeRegister(MPR121_FDLT, 0x00);
  cap3.writeRegister(MPR121_DEBOUNCE, 0x01);
  cap3.writeRegister(MPR121_CONFIG1, 0x20); // default, 16uA charge current
  cap3.writeRegister(MPR121_CONFIG2, 0x20); // 0.5uS encoding, 1ms period
  cap3.writeRegister(MPR121_AUTOCONFIG0, 0x1B);
  cap3.writeRegister(MPR121_UPLIMIT, 205);     // ((Vdd - 0.7)/Vdd) * 256
  cap3.writeRegister(MPR121_TARGETLIMIT, 190); // UPLIMIT * 0.9
  cap3.writeRegister(MPR121_LOWLIMIT, 125);    // UPLIMIT * 0.65
  cap3.writeRegister(MPR121_ECR, ECR_SETTING);
}

void panic() {
  if (digitalRead(15) == LOW) {
    packet.statusByte = 0xB0 | channel ;
    packet.controller = 0x7B;
    packet.controlValue = 0x00;
    wireless.sendData();
  }
}

void systemScreen() {
  tft.fillScreen(BLACK);
  tft.setCursor(0, 20);
  tft.setTextColor(HOTPINK);
  tft.print("System Settings");
  tft.fillRect(10, 40, 30, 30, RED);
  tft.setCursor(42, 48);
  tft.setTextColor(RED);
  tft.print("(TOP) ");
  tft.setTextColor(WHITE);
  tft.print("Key Reset");
  tft.fillRect(10, 80, 30, 30, BLUE);
  tft.setCursor(55, 88);
  tft.setTextColor(WHITE);
  tft.print("Vel/Press Scale ");
  tft.fillRect(10, 120, 30, 30, YELLOW);
  tft.setCursor(55, 128);
  tft.setTextColor(WHITE);
  tft.print("Fixed Velocity");
  tft.setCursor(0, 158);
  tft.setTextColor(GRASS);
  tft.print("Touch Vel = ");
 
  if (scale == 1) {
    tft.print("On");
  }
  if (scale == 0) {
    tft.setTextColor(RED);
    tft.print("Off");
  }  
    tft.setTextColor(YELLOWGREEN);
    tft.setCursor(0, 186);
    tft.print("Velocity = ");
    tft.print(Velocity);
}
