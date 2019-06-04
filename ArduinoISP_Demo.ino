//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Supervisors: Prof. Danny Hughes, Dr. Fan Yang
// Author: Sarah Joseph, Bachelorproef Informatica, Arduino Support for Smart LEDs
//
// This file has been changed for direct ISP communication between two atmega328 
// microcontrollers. The Programmer board is a Funduino with a Nano Atmega328
// bootloader, the target board is a Arduino Nano with a Nano Atmega328 bootloader.
// The file derives many methods from the ArduinoISP example (Arduino IDE 1.0) 
// created by David A. Mellis and Randall Bohn.
//
// The serial byte array defined at the start is a 'virtual' or artificial serial port 
// that contains hexadecimal data to connect to, identify, and upload a program via 
// I(C)SP. The case 'd' sections within this array are used to program application 
// flash memory.  
//
// The technology is applicable for optical communication using VLC with red LEDs.
// Where the programmer Arduino Nano receives and decodes a VLC signal in reverse bias. 
//
// Currently, the sketch file contains code to upload the Blink program via I(C)SP. The wires 
// to connect for I(C)SP programming are the following.
//
// Programmer       Target
//
// 5V ------------> 2-+Vcc
// GND -----------> 6-GND
// 13 ------------> 3-SCK
// 12 ------------> 1-MISO
// 11 ------------> 4-MOSI
// 10 ------------> 5-RESET
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "pins_arduino.h"  // defines SS,MOSI,MISO,SCK
#include <avr/pgmspace.h>

#define RESET SS

#define LED_HB 9
#define LED_ERR 8
#define LED_PMODE 7

#define HWVER 2
#define SWMAJ 1
#define SWMIN 18

// STK Definitions
#define STK_OK 0x10
#define STK_FAILED 0x11
#define STK_UNKNOWN 0x12
#define STK_INSYNC 0x14
#define STK_NOSYNC 0x15
#define CRC_EOP 0x20 //ok it is a space...

void pulse(int pin, int times);

void setup() {
  Serial.begin(19200);
  pinMode(7, OUTPUT);
  pulse(7, 2);
  pinMode(8, OUTPUT);
  pulse(8, 2);
  pinMode(9, OUTPUT);
  pulse(9, 2);
}

int error=0;
int pmode=0;

// address for reading and writing, set by 'U' command
int here;
int i = 0;
int j = 0;

const uint8_t serial[] PROGMEM = {0x30,0x20,0x30,0x20,0x30,0x20,0x41,0x80,0x20,0x41,0x81,0x20,0x41,0x82,0x20,0x41,0x98,0x20,0x41,0x84,0x20,0x41,0x85,0x20,0x41,0x86,0x20,0x41,0x87,0x20,
0x41,0x89,0x20,0x41,0x81,0x20,0x41,0x82,0x20,
// case 0 and A
0x42,0x86,0x00,0x00,0x01,0x01,0x01,0x01,0x03,0xff,0xff,0xff,0xff,0x00,0x80,0x04,0x00,0x00,0x00,0x80,0x00,0x20,
// case B
0x45,0x05,0x04,0xd7,0xc2,0x00,0x20,
// case E
0x50,0x20,
// case P
0x56,0x30,0x00,0x00,0x00,0x20,0x56,0x30,0x00,0x01,0x00,0x20,0x56,0x30,0x00,0x02,0x00,0x20,0x56,0xa0,0x03,0xfc,0x00,0x20,0x56,0xa0,0x03,0xfd,0x00,0x20,0x56,0xa0,0x03,0xfe,0x00,
0x20,0x56,0xa0,0x03,0xff,0x00,0x20,0x56,0xac,0x80,0x00,0x00,0x20,
// case V
0x41,0x81,0x20,0x41,0x82,0x20,
// case A
0x42,0x86,0x00,0x00,0x01,0x01,0x01,0x01,0x03,0xff,0xff,0xff,0xff,0x00,0x80,0x04,0x00,0x00,0x00,0x80,0x00,0x20,
// case B
0x45,0x05,0x04,0xd7,0xc2,0x00,0x20,
// case E
0x50,0x20,
// case P
0x55,0x00,0x00,0x20,
// case U
0x64,0x00,0x80,0x46,0x0c,0x94,0x61,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,
0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,
0x9a,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,0x94,0x7e,0x00,0x0c,
0x94,0x7e,0x00,0x00,0x00,0x00,0x00,0x24,0x00,0x27,0x00,0x2a,0x00,0x00,0x00,0x00,0x00,0x25,0x00,0x28,0x00,0x2b,0x00,0x00,0x00,0x00,0x00,0x20,
// case d (1)
0x55,0x40,0x00,0x20,
// case U
0x64,0x00,0x80,0x46,0x23,0x00,0x26,0x00,0x29,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x02,0x02,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x01,0x02,0x04,0x08,0x10,
0x20,0x40,0x80,0x01,0x02,0x04,0x08,0x10,0x20,0x01,0x02,0x04,0x08,0x10,0x20,0x00,0x00,0x00,0x07,0x00,0x02,0x01,0x00,0x00,0x03,0x04,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x11,0x24,0x1f,0xbe,0xcf,0xef,0xd8,0xe0,0xde,0xbf,0xcd,0xbf,0x11,0xe0,0xa0,0xe0,0xb1,0xe0,0xe2,0xe0,0xf4,0xe0,0x02,0xc0,0x05,0x90,0x0d,0x92,0xa0,0x30,0xb1,0x07,0xd9,0xf7,0x11,
0xe0,0xa0,0xe0,0xb1,0xe0,0x01,0xc0,0x1d,0x92,
0xa9,0x30,0xb1,0x07,0xe1,0xf7,0x0e,0x94,0xf0,0x01,0x0c,0x94,0xff,0x01,0x0c,0x94,0x00,0x00,0x20,
// case d (2)
0x55,0x80,0x00,0x20,
// case U
0x64,0x00,0x80,0x46,0x8d,0xe0,0x61,0xe0,0x0e,0x94,0x9c,0x01,0x68,0xee,0x73,0xe0,0x80,0xe0,0x90,0xe0,0x0e,0x94,0xe2,0x00,0x8d,0xe0,0x60,0xe0,0x0e,0x94,0x9c,0x01,0x68,0xee,0x73,
0xe0,0x80,0xe0,0x90,0xe0,0x0e,0x94,0xe2,0x00,0x08,0x95,0x8d,0xe0,0x61,0xe0,0x0e,0x94,0x76,0x01,0x08,0x95,0x1f,0x92,0x0f,0x92,0x0f,0xb6,0x0f,0x92,0x11,0x24,0x2f,0x93,0x3f,0x93,
0x8f,0x93,0x9f,0x93,0xaf,0x93,0xbf,0x93,0x80,0x91,0x04,0x01,0x90,0x91,0x05,0x01,0xa0,0x91,0x06,0x01,0xb0,0x91,0x07,0x01,0x30,0x91,0x08,0x01,0x01,0x96,0xa1,0x1d,0xb1,0x1d,0x23,
0x2f,0x2d,0x5f,0x2d,0x37,0x20,0xf0,0x2d,0x57,0x01,0x96,0xa1,0x1d,0xb1,0x1d,0x20,0x93,0x08,0x01,0x80,0x93,0x04,0x01,0x90,0x93,0x05,0x01,0x20,
// case d (3)
0x55,0xc0,0x00,0x20,
// case U
0x64,0x00,0x80,0x46,0xa0,0x93,0x06,0x01,0xb0,0x93,0x07,0x01,0x80,0x91,0x00,0x01,0x90,0x91,0x01,0x01,0xa0,0x91,0x02,0x01,0xb0,0x91,0x03,0x01,0x01,0x96,0xa1,0x1d,0xb1,0x1d,0x80,
0x93,0x00,0x01,0x90,0x93,0x01,0x01,0xa0,0x93,0x02,0x01,0xb0,0x93,0x03,0x01,0xbf,0x91,0xaf,0x91,0x9f,0x91,0x8f,0x91,0x3f,0x91,0x2f,0x91,0x0f,0x90,0x0f,0xbe,0x0f,0x90,0x1f,0x90,
0x18,0x95,0x9b,0x01,0xac,0x01,0x7f,0xb7,0xf8,0x94,0x80,0x91,0x00,0x01,0x90,0x91,0x01,0x01,0xa0,0x91,0x02,0x01,0xb0,0x91,0x03,0x01,0x66,0xb5,0xa8,0x9b,0x05,0xc0,0x6f,0x3f,0x19,
0xf0,0x01,0x96,0xa1,0x1d,0xb1,0x1d,0x7f,0xbf,0xba,0x2f,0xa9,0x2f,0x98,0x2f,0x88,0x27,0x86,0x0f,0x91,0x1d,0xa1,0x1d,0xb1,0x1d,0x62,0xe0,0x20,
// case d (4)
0x55,0x00,0x01,0x20,
// case U
0x64,0x00,0x80,0x46,0x88,0x0f,0x99,0x1f,0xaa,0x1f,0xbb,0x1f,0x6a,0x95,0xd1,0xf7,0xbc,0x01,0x2d,0xc0,0xff,0xb7,0xf8,0x94,0x80,0x91,0x00,0x01,0x90,0x91,0x01,0x01,0xa0,0x91,0x02,
0x01,0xb0,0x91,0x03,0x01,0xe6,0xb5,0xa8,0x9b,0x05,0xc0,0xef,0x3f,0x19,0xf0,0x01,0x96,0xa1,0x1d,0xb1,0x1d,0xff,0xbf,0xba,0x2f,0xa9,0x2f,0x98,0x2f,0x88,0x27,0x8e,0x0f,0x91,0x1d,
0xa1,0x1d,0xb1,0x1d,0xe2,0xe0,0x88,0x0f,0x99,0x1f,0xaa,0x1f,0xbb,0x1f,0xea,0x95,0xd1,0xf7,0x86,0x1b,0x97,0x0b,0x88,0x5e,0x93,0x40,0xc8,0xf2,0x21,0x50,0x30,0x40,0x40,0x40,0x50,
0x40,0x68,0x51,0x7c,0x4f,0x21,0x15,0x31,0x05,0x41,0x05,0x51,0x05,0x71,0xf6,0x08,0x95,0x78,0x94,0x84,0xb5,0x82,0x60,0x84,0xbd,0x84,0xb5,0x20,
// case d (5)
0x55,0x40,0x01,0x20,
// case U
0x64,0x00,0x80,0x46,0x81,0x60,0x84,0xbd,0x85,0xb5,0x82,0x60,0x85,0xbd,0x85,0xb5,0x81,0x60,0x85,0xbd,0xee,0xe6,0xf0,0xe0,0x80,0x81,0x81,0x60,0x80,0x83,0xe1,0xe8,0xf0,0xe0,0x10,
0x82,0x80,0x81,0x82,0x60,0x80,0x83,0x80,0x81,0x81,0x60,0x80,0x83,0xe0,0xe8,0xf0,0xe0,0x80,0x81,0x81,0x60,0x80,0x83,0xe1,0xeb,0xf0,0xe0,0x80,0x81,0x84,0x60,0x80,0x83,0xe0,0xeb,
0xf0,0xe0,0x80,0x81,0x81,0x60,0x80,0x83,0xea,0xe7,0xf0,0xe0,0x80,0x81,0x84,0x60,0x80,0x83,0x80,0x81,0x82,0x60,0x80,0x83,0x80,0x81,0x81,0x60,0x80,0x83,0x80,0x81,0x80,0x68,0x80,
0x83,0x10,0x92,0xc1,0x00,0x08,0x95,0x48,0x2f,0x50,0xe0,0xca,0x01,0x86,0x56,0x9f,0x4f,0xfc,0x01,0x24,0x91,0x4a,0x57,0x5f,0x4f,0xfa,0x01,0x20,
// case d (6)
0x55,0x80,0x01,0x20,
// case U
0x64,0x00,0x80,0x46,0x84,0x91,0x88,0x23,0xc1,0xf0,0xe8,0x2f,0xf0,0xe0,0xee,0x0f,0xff,0x1f,0xe8,0x59,0xff,0x4f,0xa5,0x91,0xb4,0x91,0x66,0x23,0x41,0xf4,0x9f,0xb7,0xf8,0x94,0x8c,
0x91,0x20,0x95,0x82,0x23,0x8c,0x93,0x9f,0xbf,0x08,0x95,0x9f,0xb7,0xf8,0x94,0x8c,0x91,0x82,0x2b,0x8c,0x93,0x9f,0xbf,0x08,0x95,0x48,0x2f,0x50,0xe0,0xca,0x01,0x82,0x55,0x9f,0x4f,
0xfc,0x01,0x24,0x91,0xca,0x01,0x86,0x56,0x9f,0x4f,0xfc,0x01,0x94,0x91,0x4a,0x57,0x5f,0x4f,0xfa,0x01,0x34,0x91,0x33,0x23,0x09,0xf4,0x40,0xc0,0x22,0x23,0x51,0xf1,0x23,0x30,0x71,
0xf0,0x24,0x30,0x28,0xf4,0x21,0x30,0xa1,0xf0,0x22,0x30,0x11,0xf5,0x14,0xc0,0x26,0x30,0xb1,0xf0,0x27,0x30,0xc1,0xf0,0x24,0x30,0xd9,0xf4,0x20,
// case d (7)
0x55,0xc0,0x01,0x20,
// case U
0x64,0x00,0x80,0x46,0x04,0xc0,0x80,0x91,0x80,0x00,0x8f,0x77,0x03,0xc0,0x80,0x91,0x80,0x00,0x8f,0x7d,0x80,0x93,0x80,0x00,0x10,0xc0,0x84,0xb5,0x8f,0x77,0x02,0xc0,0x84,0xb5,0x8f,
0x7d,0x84,0xbd,0x09,0xc0,0x80,0x91,0xb0,0x00,0x8f,0x77,0x03,0xc0,0x80,0x91,0xb0,0x00,0x8f,0x7d,0x80,0x93,0xb0,0x00,0xe3,0x2f,0xf0,0xe0,0xee,0x0f,0xff,0x1f,0xee,0x58,0xff,0x4f,
0xa5,0x91,0xb4,0x91,0x2f,0xb7,0xf8,0x94,0x66,0x23,0x21,0xf4,0x8c,0x91,0x90,0x95,0x89,0x23,0x02,0xc0,0x8c,0x91,0x89,0x2b,0x8c,0x93,0x2f,0xbf,0x08,0x95,0xcf,0x93,0xdf,0x93,0x0e,
0x94,0x3b,0x01,0x0e,0x94,0x95,0x00,0xc0,0xe0,0xd0,0xe0,0x0e,0x94,0x80,0x00,0x20,0x97,0xe1,0xf3,0x0e,0x94,0x00,0x00,0xf9,0xcf,0xf8,0x94,0x20,
// case d (8)
0x55,0x00,0x02,0x20,
// case U
0x64,0x00,0x02,0x46,0xff,0xcf,0x20,
// case d (9)
0x55,0x00,0x00,0x20,
// case U
0x74,0x00,0x80,0x46,0x20,
// case t (1)
0x55,0x40,0x00,0x20,
// case U
0x74,0x00,0x80,0x46,0x20,
// case t (2)
0x55,0x80,0x00,0x20,
// case U
0x74,0x00,0x80,0x46,0x20,
// case t (3)
0x55,0xc0,0x00,0x20,
// case U
0x74,0x00,0x80,0x46,0x20,
// case t (4)
0x55,0x00,0x01,0x20,
// case U
0x74,0x00,0x80,0x46,0x20,
// case t (5)
0x55,0x40,0x01,0x20,
// case U
0x74,0x00,0x80,0x46,0x20,
// case t (6)
0x55,0x80,0x01,0x20,
// case U
0x74,0x00,0x80,0x46,0x20,
// case t (7)
0x55,0xc0,0x01,0x20,
// case U
0x74,0x00,0x80,0x46,0x20,
// case t (8)
0x55,0x00,0x02,0x20,
// case U
0x74,0x00,0x02,0x46,0x20,
// case t (9)
0x51,0x20};
// case Q

uint8_t buff[256]; // global block storage

#define beget16(addr) (*addr * 256 + *(addr+1) )
typedef struct param {
  
  uint8_t devicecode;
  uint8_t revision;
  
  
  uint8_t progtype;
  uint8_t parmode;
  uint8_t polling;
  uint8_t selftimed;
  uint8_t lockbytes;
  uint8_t fusebytes;
  int flashpoll;
  int eeprompoll;
  int pagesize;
  int eepromsize;
  int flashsize;
} 
parameter;

parameter param;

// this provides a heartbeat on pin 9, so you can tell the software is running.
uint8_t hbval=128;
int8_t hbdelta=8;
void heartbeat() {
  if (hbval > 192) hbdelta = -hbdelta;
  if (hbval < 32) hbdelta = -hbdelta;
  hbval += hbdelta;
  analogWrite(LED_HB, hbval);
  delay(40);
}
  

void loop(void) {
  // is pmode active?
  if (pmode) digitalWrite(LED_PMODE, HIGH); 
  else digitalWrite(LED_PMODE, LOW);
  // is there an error?
  if (error) digitalWrite(LED_ERR, HIGH); 
  else digitalWrite(LED_ERR, LOW);
  
  // light the heartbeat LED
  heartbeat();
  avrisp();
}

uint8_t getch() {
  return serial_();
  
}

uint8_t serial_() {
    uint8_t t = pgm_read_byte_near(serial + i);
    i = i+1;
    return t;
//  if(i<159){
//    uint8_t t = pgm_read_byte_near(serial + i);
//    i = i+1;
//    return t;
//  }
//  else{
//    return vlc_buffer[j++];
//  }
}

void readbytes(int n) {
  for (int x = 0; x < n; x++) {
    buff[x] = serial_();
  }
}

#define PTIME 30
void pulse(int pin, int times) {
  do {
    digitalWrite(pin, HIGH);
    delay(PTIME);
    digitalWrite(pin, LOW);
    delay(PTIME);
  } 
  while (times--);
}

void spi_init() {
  uint8_t x;
  SPCR = 0x53;
  x=SPSR;
  x=SPDR;
}

void spi_wait() {
  do {
  } 
  while (!(SPSR & (1 << SPIF)));
}

uint8_t spi_send(uint8_t b) {
  uint8_t reply;
  SPDR=b;
  spi_wait();
  reply = SPDR;
  return reply;
}

uint8_t spi_transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  uint8_t n;
  spi_send(a); 
  n=spi_send(b);
  //if (n != a) error = -1;
  n=spi_send(c);
  return spi_send(d);
}

void empty_reply() {
  if (CRC_EOP == getch()) {
    Serial.print((char)STK_INSYNC, HEX);
    Serial.println();
    Serial.print((char)STK_OK, HEX);
    Serial.println();
  } 
  else {
    Serial.print((char)STK_NOSYNC, HEX);
    Serial.println();   
  }
}

void breply(uint8_t b) {
  if (CRC_EOP == getch()) {
    Serial.print((char)STK_INSYNC, HEX);
    Serial.println();    
    Serial.print((char)b, HEX);
    Serial.println();    
    Serial.print((char)STK_OK, HEX);
    Serial.println();    
  } 
  else {
    Serial.print((char)STK_NOSYNC, HEX);
    Serial.println();    
  }
}

void get_version(uint8_t c) {
  switch(c) {
  case 0x80:
    breply(HWVER);
    break;
  case 0x81:
    breply(SWMAJ);
    break;
  case 0x82:
    breply(SWMIN);
    break;
  case 0x93:
    breply('S'); // serial programmer
    break;
  default:
    breply(0);
  }
}

void set_parameters() {
  // call this after reading paramter packet into buff[]
  param.devicecode = buff[0];
  param.revision = buff[1];
  param.progtype = buff[2];
  param.parmode = buff[3];
  param.polling = buff[4];
  param.selftimed = buff[5];
  param.lockbytes = buff[6];
  param.fusebytes = buff[7];
  param.flashpoll = buff[8]; 
  // ignore buff[9] (= buff[8])
  //getch(); // discard second value
  
  // WARNING: not sure about the byte order of the following
  // following are 16 bits (big endian)
  param.eeprompoll = beget16(&buff[10]);
  param.pagesize = beget16(&buff[12]);
  param.eepromsize = beget16(&buff[14]);

  // 32 bits flashsize (big endian)
  param.flashsize = buff[16] * 0x01000000
    + buff[17] * 0x00010000
    + buff[18] * 0x00000100
    + buff[19];

}

void start_pmode() {
  spi_init();
  // following delays may not work on all targets...
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, HIGH);
  pinMode(SCK, OUTPUT);
  digitalWrite(SCK, LOW);
  delay(50);
  digitalWrite(RESET, LOW);
  delay(50);
  pinMode(MISO, INPUT);
  pinMode(MOSI, OUTPUT);
  spi_transaction(0xAC, 0x53, 0x00, 0x00);
  pmode = 1;
}

void end_pmode() {
  pinMode(MISO, INPUT);
  pinMode(MOSI, INPUT);
  pinMode(SCK, INPUT);
  pinMode(RESET, INPUT);
  pmode = 0;
}

void universal() {
  int w;
  uint8_t ch;

  for (w = 0; w < 4; w++) {
    buff[w] = getch();
  }
  ch = spi_transaction(buff[0], buff[1], buff[2], buff[3]);
  breply(ch);
}

void flash(uint8_t hilo, int addr, uint8_t data) {
  spi_transaction(0x40+8*hilo, 
  addr>>8 & 0xFF, 
  addr & 0xFF,
  data);
}
void commit(int addr) {
  spi_transaction(0x4C, (addr >> 8) & 0xFF, addr & 0xFF, 0);
}

//#define _current_page(x) (here & 0xFFFFE0)
int current_page(int addr) {
  if (param.pagesize == 32) return here & 0xFFFFFFF0;
  if (param.pagesize == 64) return here & 0xFFFFFFE0;
  if (param.pagesize == 128) return here & 0xFFFFFFC0;
  if (param.pagesize == 256) return here & 0xFFFFFF80;
  return here;
}
uint8_t write_flash(int length) {
  if (param.pagesize < 1) return STK_FAILED;
  //if (param.pagesize != 64) return STK_FAILED;
  int page = current_page(here);
  int x = 0;
  while (x < length) {
    if (page != current_page(here)) {
      commit(page);
      page = current_page(here);
    }
    flash(LOW, here, buff[x++]);
    flash(HIGH, here, buff[x++]);
    here++;
  }

  commit(page);

  return STK_OK;
}

uint8_t write_eeprom(int length) {
  // here is a word address, so we use here*2
  // this writes byte-by-byte,
  // page writing may be faster (4 bytes at a time)
  for (int x = 0; x < length; x++) {
    spi_transaction(0xC0, 0x00, here*2+x, buff[x]);
    delay(45);
  } 
  return STK_OK;
}

void program_page() {
  char result = (char) STK_FAILED;
  int length = 256 * getch() + getch();
  if (length > 256) {
      Serial.print((char) STK_FAILED);
      Serial.println();    
      return;
  }
  char memtype = getch();
  for (int x = 0; x < length; x++) {
    buff[x] = getch();
  }
  if (CRC_EOP == getch()) {
    Serial.print((char) STK_INSYNC, HEX);
    Serial.println();    
    if (memtype == 'F') result = (char)write_flash(length);
    if (memtype == 'E') result = (char)write_eeprom(length);
    Serial.print(result, HEX);
    Serial.println();    
  } 
  else {
    Serial.print((char) STK_NOSYNC, HEX);
    Serial.println();    
  }
}
uint8_t flash_read(uint8_t hilo, int addr) {
  return spi_transaction(0x20 + hilo * 8,
    (addr >> 8) & 0xFF,
    addr & 0xFF,
    0);
}

char flash_read_page(int length) {
  for (int x = 0; x < length; x+=2) {
    uint8_t low = flash_read(LOW, here);
    Serial.print((char) low, HEX);
    Serial.println();    
    uint8_t high = flash_read(HIGH, here);
    Serial.print((char) high, HEX);
    Serial.println();    
    here++;
  }
  return STK_OK;
}

char eeprom_read_page(int length) {
  // here again we have a word address
  for (int x = 0; x < length; x++) {
    uint8_t ee = spi_transaction(0xA0, 0x00, here*2+x, 0xFF);
    Serial.print((char) ee, HEX);
    Serial.println();    
  }
  return STK_OK;
}

void read_page() {
  char result = (char)STK_FAILED;
  int length = 256 * getch() + getch();
  char memtype = getch();
  if (CRC_EOP != getch()) {
    Serial.print((char) STK_NOSYNC, HEX);
    Serial.println();    
    return;
  }
  Serial.print((char) STK_INSYNC, HEX);
  Serial.println();  
  if (memtype == 'F') result = flash_read_page(length);
  if (memtype == 'E') result = eeprom_read_page(length);
  Serial.print(result, HEX);
  Serial.println();
  return;
}

void read_signature() {
  if (CRC_EOP != getch()) {
    Serial.print((char) STK_NOSYNC, HEX);
    Serial.println();    
    return;
  }
  Serial.print((char) STK_INSYNC, HEX);
  Serial.println();  
  uint8_t high = spi_transaction(0x30, 0x00, 0x00, 0x00);
  Serial.print((char) high, HEX);
  Serial.println();  
  uint8_t middle = spi_transaction(0x30, 0x00, 0x01, 0x00);
  Serial.print((char) middle, HEX);
  Serial.println();  
  uint8_t low = spi_transaction(0x30, 0x00, 0x02, 0x00);
  Serial.print((char) low, HEX);
  Serial.println();  
  Serial.print((char) STK_OK, HEX);
  Serial.println();  
}
//////////////////////////////////////////
//////////////////////////////////////////


////////////////////////////////////
////////////////////////////////////
int avrisp() { 
  uint8_t data, low, high;
  uint8_t ch = getch();
  switch (ch) {
  case '0': // signon
    empty_reply();
    break;
  case '1':
    if (getch() == CRC_EOP) {
      Serial.print((char) STK_INSYNC, HEX);
      Serial.println();  
      Serial.print("AVR ISP");
      Serial.println();  
      Serial.print((char) STK_OK, HEX);
      Serial.println();  
    }
    break;
  case 'A':
    Serial.println("entering case A, getting version");
    get_version(getch());
    
    break;
  case 'B':
    Serial.println("entering case B, setting parameters");
    readbytes(20);
    set_parameters();
    empty_reply();
    break;
  case 'E': // extended parameters - ignore for now
    Serial.println("entering case E, reading bytes");
    readbytes(5);
    empty_reply();
    break;
    


  case 'P':
    Serial.println("entering case P, starting pmode");
    start_pmode();
    empty_reply();
    break;
  case 'U':
    Serial.println("entering case U, writing to flash");
    here = getch() + 256 * getch();
    empty_reply();
    break;

  case 0x60: //STK_PROG_FLASH
    low = getch();
    high = getch();
    empty_reply();
    break;
  case 0x61: //STK_PROG_DATA
    data = getch();
    empty_reply();
    break;

  case 0x64: //STK_PROG_PAGE
    Serial.println("entering case d, programming page");
    program_page();
    break;
    
  case 0x74: //STK_READ_PAGE
    Serial.println("entering case t, reading page");
    read_page();    
    break;

  case 'V':
    Serial.println("entering case V, accepting instructions");
    universal();
    break;
  case 'Q':
    Serial.println("entering case Q, accepting instructions");
    error=0;
    end_pmode();
    empty_reply();
    break;
    
  case 0x75: //STK_READ_SIGN
    read_signature();
    break;

  // expecting a command, not CRC_EOP
  // this is how we can get back in sync
  case CRC_EOP:
    Serial.print((char) STK_NOSYNC, HEX);
    Serial.println();  
    break;
    
  // anything else we will return STK_UNKNOWN
  default:
    if (CRC_EOP == getch()){
      Serial.print((char)STK_UNKNOWN, HEX);
      Serial.println();  
    }
    else{
      Serial.print((char)STK_NOSYNC, HEX);
      Serial.println();
    }
    
    
  }
}
