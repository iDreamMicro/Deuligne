/***
 *
 *
 * Deuligne library
 *
 * copyleft 2011 snootlab
 * free software, have fun !
 *
 **/
#include "Deuligne.h"
#include <Wire.h>

extern "C" {
  #include <stdio.h>  //not needed yet
  #include <string.h> //needed for strlen()
  #include <inttypes.h>
  #include "WConstants.h"  //all things wiring / arduino
}

//command bytes for LCD
#define CMD_CLR 0x01
#define CMD_RIGHT 0x1C
#define CMD_LEFT 0x18
#define CMD_HOME 0x02

#define NUM_KEYS 5

//stuff the library user might call---------------------------------

//constructor.  num_lines must be 1 or 2, currently.

static byte dataPlusMask = 0x20; // TODO!!!

static const int adc_key_val[NUM_KEYS] = { 50, 190, 400, 540, 770 };

Deuligne::Deuligne( uint8_t devI2CAddress, uint8_t num_lines, uint8_t lcdwidth, uint8_t bufferwidth)  {
  myNumLines = num_lines;
  myWidth = lcdwidth;
  myAddress = devI2CAddress;
  myBufferwidth= bufferwidth;
}

void SetMCPReg( byte deviceAddr, byte reg, byte val ) {
  Wire.beginTransmission(deviceAddr);
  Wire.send(reg);
  Wire.send(val);
  Wire.endTransmission();
}

void SendToLCD( byte deviceAddr, byte data ) {
  data |= dataPlusMask;
  SetMCPReg(deviceAddr,0x0A,data);
  data ^= 0x10; // E
  delayMicroseconds(1);
  SetMCPReg(deviceAddr,0x0A,data);
  data ^= 0x10; // E
  delayMicroseconds(1);
  SetMCPReg(deviceAddr,0x0A,data);
  delay(1);
}

void WriteLCDByte( byte deviceAddr, byte bdata ) {
  SendToLCD(deviceAddr,bdata >> 4);
  SendToLCD(deviceAddr,bdata & 0x0F);
}

void Deuligne::init( void ) {
  TWBR = ((CPU_FREQ / TWI_FREQ_MCP23008) - 16) / 2;
  dataPlusMask = 0; // initial: 0
  SetMCPReg(myAddress,0x05,0x0C); // set CONFREG (0x05) to 0
  SetMCPReg(myAddress,0x00,0x00); // set IOREG (0x00) to 0
  //
  delay(50);
  SendToLCD(myAddress,0x03); 
  delay(5);
  SendToLCD(myAddress,0x03);
  delayMicroseconds(100);
  SendToLCD(myAddress,0x03);
  delay(5);
  SendToLCD(myAddress,0x02);
  WriteLCDByte(myAddress,0x28);
  WriteLCDByte(myAddress,0x08);
  WriteLCDByte(myAddress,0x0C); // turn on, cursor off, no blinking
  delayMicroseconds(60);
  WriteLCDByte(myAddress,LCD_CLEARDISPLAY); // clear display
  delay(3);  
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);
  backLight(1);
  display();
}

void Deuligne::backLight( bool turnOn ) {
  dataPlusMask = turnOn ? (dataPlusMask | 0x80) : (dataPlusMask & ~0x80);
  SetMCPReg(myAddress,0x0A,dataPlusMask);  
}


void Deuligne::write( uint8_t value ) {
  dataPlusMask |= 0x40; // RS
  WriteLCDByte(myAddress,(byte)value);
  dataPlusMask ^= 0x40; // RS
}
/*
void Deuligne::printIn( char value[] ) {
  for ( char *p = value; *p != 0; p++ ) 
    print(*p);
}

*/
void Deuligne::clear() {
  command(CMD_CLR);
}

void Deuligne::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {

}

void Deuligne::setCursor(uint8_t col, uint8_t row) {
  uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row > myNumLines ) {
    row = myNumLines-1;    // we count rows starting w/0
  }
  
  command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

/*
void Deuligne::cursorTo(int line_num, int x) {
  command(CMD_HOME);
  int targetPos = x + line_num * myBufferwidth;
  for ( int i = 0; i < targetPos; i++)
    command(0x14);
}*/

void Deuligne::command( uint8_t command ) {
  // RS - leave low
  WriteLCDByte(myAddress,command);
  delay(1);
}


/**
 *  From LiquidCrystal (official arduino) Library
 **/

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void Deuligne::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (uint8_t i=0; i<8; i++) {
    write(charmap[i]);
  }
}



// Turn the display on/off (quickly)
void Deuligne::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Deuligne::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void Deuligne::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Deuligne::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void Deuligne::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Deuligne::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void Deuligne::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void Deuligne::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void Deuligne::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void Deuligne::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void Deuligne::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void Deuligne::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

void Deuligne::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}


// Get Joystick value
// Convert ADC value to key number : 
// 0: Right Key
// 1: Up Key
// 2: Down Key
// 3: Left Key
// 4: Select Key

int8_t Deuligne::get_key(){
  int adc_key_in = analogRead(0);    // read the value from the sensor  

  int8_t k = 0;

  //   determine which key is pressed
  for (k = 0; k < NUM_KEYS; k++)
  {
    if (adc_key_in < adc_key_val[k])
    {

      return k;
    }
  }

  if (k >= NUM_KEYS)
    k = -1;     // No valid key pressed

  return k;
}
