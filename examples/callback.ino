#define SDATA 6
#define SCLOCK 7
#define ENABLE 8

#include <LiquidCrystal.h>

// d4-d7 on shift register pins 0-3, rs on shift register pin 4
void shift_send(uint8_t value, uint8_t mode, uint8_t nbits) {
  value |= mode << 4;
  // set the pins
  shiftOut(SDATA, SCLOCK, MSBFIRST, value);
  // pulse ENABLE
  digitalWrite(ENABLE, HIGH);
  // the pulse must be at least 450ns high
  delayMicroseconds(1);
  digitalWrite(ENABLE, LOW);
  // the library handles any necessary delays after the pulse
}

// set to callback function in 4-bit mode
LiquidCrystal lcd(shift_send, 4);

void setup() {
  lcd.begin(16, 2);
  lcd.print("Hello, World!");
}

void loop() {
  lcd.setCursor(0, 1);
  lcd.print(millis()/1000);
}
