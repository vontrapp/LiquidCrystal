#ifndef LiquidCrystal_h
#define LiquidCrystal_h

#include <inttypes.h>
#include "Print.h"

// commands
#define LCD_CLEARDISPLAY 0
#define LCD_RETURNHOME 1
#define LCD_ENTRYMODESET 2
#define LCD_DISPLAYCONTROL 3
#define LCD_CURSORSHIFT 4
#define LCD_FUNCTIONSET 5
#define LCD_SETCGRAMADDR 6
#define LCD_SETDDRAMADDR 7

// flags for display entry mode
#define LCD_ENTRYLEFT 1
#define LCD_ENTRYSHIFTINCREMENT 0

// flags for display on/off control
#define LCD_DISPLAY 2
#define LCD_CURSOR 1
#define LCD_BLINK 0

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 3
#define LCD_MOVERIGHT 2

// flags for function set
#define LCD_8BITMODE 4
#define LCD_2LINE 3
#define LCD_5x10DOTS 2

#define LCDCTL_SHIFT 0
#define LCDCTL_SHIFTENABLE 1
#define LCDCTL_SHIFTLATCH 2
#define LCDCTL_CUSTOM 3

class LiquidCrystal : public Print {
public:
  LiquidCrystal(uint8_t rs, uint8_t enable,
                uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
  LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
  LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
  LiquidCrystal(uint8_t rs, uint8_t enable,
                uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);

  // shift register pins as follows
  // 0 - enable
  // 1 - rs
  // 2 - rw
  // 3 - d0
  // 4 - d1
  // 5 - d2
  // 6 - d3
  // shift registers
  LiquidCrystal(uint8_t shiftdata, uint8_t shiftclock, uint8_t shiftlatch);
  // latch can be 255 (-1) to indicate a non-latching register when a separate enable pin is given
  LiquidCrystal(uint8_t enable, uint8_t shiftdata, uint8_t shiftclock, uint8_t shiftlatch);

  // custom callback function
  // nbits must be 4 or 8
  // the callback must do the following
  // * set the data pins (4 or 8, according to nbits)
  // * set the rs pin to mode
  // * r/w must be low
  // * pulse the enable pin, >450ns high then low again
  LiquidCrystal(void (*func)(uint8_t value, uint8_t mode, uint8_t nbits), uint8_t nbits);

  void init(uint8_t ctl/*LCDCTL_*/, uint8_t rs, uint8_t rw, uint8_t enable,
            uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
            uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
            uint8_t shiftdata, uint8_t shiftclock, uint8_t shiftlatch);

  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = !LCD_5x10DOTS);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t); 
  virtual size_t write(uint8_t);
  void command(uint8_t);

  using Print::write;
private:
  void send(uint8_t, uint8_t, uint8_t);
  void write4bits(uint8_t);
  void write8bits(uint8_t);
  void pulseEnable();

  uint8_t _rs_pin; // LOW: command.  HIGH: character.
  uint8_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
  uint8_t _enable_pin; // activated by a HIGH pulse.
  uint8_t _data_pins[8];
  uint8_t _shiftdata, _shiftclock, _shiftlatch;
  uint8_t _ctlmode;
  uint8_t (*_write_callback)(uint8_t value, uint8_t mode, uint8_t nbits);

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _initialized;

  uint8_t _numlines,_currline;
};

#endif
