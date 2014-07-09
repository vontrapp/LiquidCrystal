#include "LiquidCrystal.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                             uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  init(_BV(LCD_8BITMODE), rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7, 255, 255, 255);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                             uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  init(_BV(LCD_8BITMODE), rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7, 255, 255, 255);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  init(0, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0, 255, 255, 255);
}

LiquidCrystal::LiquidCrystal(uint8_t rs,  uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  init(0, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0, 255, 255, 255);
}

LiquidCrystal::LiquidCrystal(uint8_t shiftdata, uint8_t shiftclock, uint8_t shiftlatch)
{
  uint8_t ctl = _BV(LCDCTL_SHIFT);
  ctl |= _BV(LCDCTL_SHIFTENABLE);
  ctl |= _BV(LCDCTL_SHIFTLATCH);
  init(ctl, 1, 2, 0, 3, 4, 5, 6, 7, 8, 9, 10, shiftdata, shiftclock, shiftlatch);
}

LiquidCrystal::LiquidCrystal(uint8_t enable, uint8_t shiftdata, uint8_t shiftclock, uint8_t shiftlatch)
{
  uint8_t ctl = _BV(LCDCTL_SHIFT);
  if (shiftlatch != 255) {
    ctl |= _BV(LCDCTL_SHIFTLATCH);
  }
  init(ctl, 1, 2, enable, 3, 4, 5, 6, 7, 8, 9, 10, shiftdata, shiftclock, shiftlatch);
}

LiquidCrystal::LiquidCrystal(uint8_t (*func)(uint8_t, uint8_t, uint8_t), uint8_t nbits)
{
  uint8_t ctl = _BV(LCDCTL_CUSTOM);
  if (nbits == 8) {
    ctl |= _BV(LCD_8BITMODE);
  }

  _write_callback = func;

  init(ctl, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255);
}

void LiquidCrystal::init(uint8_t ctl, uint8_t rs, uint8_t rw, uint8_t enable,
                         uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                         uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
                         uint8_t shiftdata, uint8_t shiftclock, uint8_t shiftlatch)
{
  _rs_pin = rs;
  _rw_pin = rw;
  _enable_pin = enable;

  _data_pins[0] = d0;
  _data_pins[1] = d1;
  _data_pins[2] = d2;
  _data_pins[3] = d3;
  _data_pins[4] = d4;
  _data_pins[5] = d5;
  _data_pins[6] = d6;
  _data_pins[7] = d7;

  _shiftdata = shiftdata;
  _shiftclock = shiftclock;
  _shiftlatch = shiftlatch;

  _ctlmode = ctl;

  if (_ctlmode & _BV(LCDCTL_SHIFT)) {
    // shift mode
    pinMode(_shiftdata, OUTPUT);
    pinMode(_shiftclock, OUTPUT);
    if (_ctlmode & _BV(LCDCTL_SHIFTLATCH)) {
      pinMode(_shiftlatch, OUTPUT);
    } else {
      pinMode(_enable_pin, OUTPUT);
    }
  } else if (~_ctlmode & _BV(LCDCTL_CUSTOM)) {
    // ^^ don't set any pin modes if custom control mode
    pinMode(_rs_pin, OUTPUT);
    // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
    if (_rw_pin != 255) { 
      pinMode(_rw_pin, OUTPUT);
    }
    pinMode(_enable_pin, OUTPUT);
  }

  if (_ctlmode & _BV(LCD_8BITMODE))
    _displayfunction = _BV(LCD_8BITMODE);
  else
    _displayfunction = 0;

  begin(16, 1);
}

void LiquidCrystal::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  if (lines > 1) {
    _displayfunction |= _BV(LCD_2LINE);
  }
  _numlines = lines;
  _currline = 0;

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != 0) && (lines == 1)) {
    _displayfunction |= _BV(LCD_5x10DOTS);
  }

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
  delayMicroseconds(50000);

  //put the LCD into 4 bit or 8 bit mode
  if (! (_displayfunction & _BV(LCD_8BITMODE))) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    send(0x03, LOW, 4);
    delayMicroseconds(4500); // wait min 4.1ms

    // second try
    send(0x03, LOW, 4);
    delayMicroseconds(4500); // wait min 4.1ms

    // third go!
    send(0x03, LOW, 4);
    delayMicroseconds(150);

    // finally, set to 4-bit interface
    send(0x02, LOW, 4);
  } else {
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    command(_BV(LCD_FUNCTIONSET) | _displayfunction);
    delayMicroseconds(4500);  // wait more than 4.1ms

    // second try
    command(_BV(LCD_FUNCTIONSET) | _displayfunction);
    delayMicroseconds(150);

    // third go
    command(_BV(LCD_FUNCTIONSET) | _displayfunction);
  }

  // finally, set # lines, font size, etc.
  command(_BV(LCD_FUNCTIONSET) | _displayfunction);

  // turn the display on with no cursor or blinking default
  _displaycontrol = _BV(LCD_DISPLAY);
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = _BV(LCD_ENTRYLEFT);
  // set the entry mode
  command(_BV(LCD_ENTRYMODESET) | _displaymode);

}

/********** high level commands, for the user! */
void LiquidCrystal::clear()
{
  command(_BV(LCD_CLEARDISPLAY));  // clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void LiquidCrystal::home()
{
  command(_BV(LCD_RETURNHOME));  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void LiquidCrystal::setCursor(uint8_t col, uint8_t row)
{
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row >= _numlines ) {
    row = _numlines-1;    // we count rows starting w/0
  }
  
  command(_BV(LCD_SETDDRAMADDR) | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LiquidCrystal::noDisplay() {
  _displaycontrol &= ~_BV(LCD_DISPLAY);
  command(_BV(LCD_DISPLAYCONTROL) | _displaycontrol);
}
void LiquidCrystal::display() {
  _displaycontrol |= _BV(LCD_DISPLAY);
  command(_BV(LCD_DISPLAYCONTROL) | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal::noCursor() {
  _displaycontrol &= ~_BV(LCD_CURSOR);
  command(_BV(LCD_DISPLAYCONTROL) | _displaycontrol);
}
void LiquidCrystal::cursor() {
  _displaycontrol |= _BV(LCD_CURSOR);
  command(_BV(LCD_DISPLAYCONTROL) | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal::noBlink() {
  _displaycontrol &= ~_BV(LCD_BLINK);
  command(_BV(LCD_DISPLAYCONTROL) | _displaycontrol);
}
void LiquidCrystal::blink() {
  _displaycontrol |= _BV(LCD_BLINK);
  command(_BV(LCD_DISPLAYCONTROL) | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal::scrollDisplayLeft(void) {
  command(_BV(LCD_CURSORSHIFT) | _BV(LCD_DISPLAYMOVE));
}
void LiquidCrystal::scrollDisplayRight(void) {
  command(_BV(LCD_CURSORSHIFT) | _BV(LCD_DISPLAYMOVE) | _BV(LCD_MOVERIGHT));
}

// This is for text that flows Left to Right
void LiquidCrystal::leftToRight(void) {
  _displaymode |= _BV(LCD_ENTRYLEFT);
  command(_BV(LCD_ENTRYMODESET) | _displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystal::rightToLeft(void) {
  _displaymode &= ~_BV(LCD_ENTRYLEFT);
  command(_BV(LCD_ENTRYMODESET) | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystal::autoscroll(void) {
  _displaymode |= _BV(LCD_ENTRYSHIFTINCREMENT);
  command(_BV(LCD_ENTRYMODESET) | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystal::noAutoscroll(void) {
  _displaymode &= ~_BV(LCD_ENTRYSHIFTINCREMENT);
  command(_BV(LCD_ENTRYMODESET) | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(_BV(LCD_SETCGRAMADDR) | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

inline void LiquidCrystal::command(uint8_t value) {
  if (_displayfunction & _BV(LCD_8BITMODE)) {
    send(value, LOW, 8);
  } else {
    send(value>>4, LOW, 4);
    send(value, LOW, 4);
  }
}

inline size_t LiquidCrystal::write(uint8_t value) {
  if (_displayfunction & _BV(LCD_8BITMODE)) {
    send(value, HIGH, 8);
  } else {
    send(value>>4, HIGH, 4);
    send(value, HIGH, 4);
  }
  return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void LiquidCrystal::send(uint8_t value, uint8_t mode, uint8_t nbits) {
  if (_ctlmode & _BV(LCDCTL_CUSTOM)) {
    // custom callback mode
    _write_callback(value, mode, nbits);
    delayMicroseconds(100);
  } else if (_ctlmode & _BV(LCDCTL_SHIFT)) {
    // shift register mode
    int sreg = 0;
    if (mode)
      sreg |= _BV(_rs_pin);
    if (_ctlmode & _BV(LCDCTL_SHIFTENABLE))
      sreg |= _BV(_enable_pin);
    if (_rw_pin != 255)
      sreg |= _BV(_rw_pin);
    for (int i = 0; i < nbits; i++) {
      sreg |= (1 & (value >> i)) << _data_pins[i];
    }

    shiftOut(_shiftdata, _shiftclock, MSBFIRST, sreg);
    if (_ctlmode & _BV(LCDCTL_SHIFTLATCH)) {
      digitalWrite(_shiftlatch, LOW);
      delayMicroseconds(1);
      digitalWrite(_shiftlatch, HIGH);
    }

    if (_ctlmode & _BV(LCDCTL_SHIFTENABLE)) {
      // have to pulse the enable using shifting
      delayMicroseconds(1); // ensure the pulse is high long enough
      sreg &= ~_BV(_enable_pin);
      shiftOut(_shiftdata, _shiftclock, MSBFIRST, sreg);
      if (_ctlmode & _BV(LCDCTL_SHIFTLATCH)) {
        digitalWrite(_shiftlatch, LOW);
        delayMicroseconds(1);
        digitalWrite(_shiftlatch, HIGH);
      }
    } else {
      // pulse the enable pin as a normal pin
      digitalWrite(_enable_pin, HIGH);
      delayMicroseconds(1);
      digitalWrite(_enable_pin, LOW);
    }
    delayMicroseconds(100);
  } else {
    // normal control mode
    digitalWrite(_rs_pin, mode);

    // if there is a RW pin indicated, set it low to Write
    if (_rw_pin != 255) { 
      digitalWrite(_rw_pin, LOW);
    }

    if (nbits == 8) {
      write8bits(value); 
    } else {
      write4bits(value);
    }
  }
}

void LiquidCrystal::pulseEnable(void) {
  digitalWrite(_enable_pin, LOW);
  delayMicroseconds(1);    
  digitalWrite(_enable_pin, HIGH);
  delayMicroseconds(1);    // enable pulse must be >450ns
  digitalWrite(_enable_pin, LOW);
  delayMicroseconds(100);   // commands need > 37us to settle
}

void LiquidCrystal::write4bits(uint8_t value) {
  for (int i = 0; i < 4; i++) {
    pinMode(_data_pins[i], OUTPUT);
    digitalWrite(_data_pins[i], (value >> i) & 0x01);
  }

  pulseEnable();
}

void LiquidCrystal::write8bits(uint8_t value) {
  for (int i = 0; i < 8; i++) {
    pinMode(_data_pins[i], OUTPUT);
    digitalWrite(_data_pins[i], (value >> i) & 0x01);
  }
  
  pulseEnable();
}
