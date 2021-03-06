/*#include "MCP23S17_LCD.h"

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
// MCP23S17_LCD constructor is called).


// Configuration for MCP23S17_LCD
// rst  = MCP23S17 RST pin
// cs   = MCP23S17 CS pin
// port = MCP23S17 PORT for LCD
MCP23S17_LCD::MCP23S17_LCD(uint8_t rst, uint8_t cs, uint8_t PORT)
{
   init (rst, cs, PORT);
}

void MCP23S17_LCD::init(uint8_t rst, uint8_t cs, uint8_t PORT)
{
  // NOTE: 4 bits only 
  // Display is connected as folows
  // D7 - GPx7
  // D6 - GPx6
  // D5 - GPx5
  // D4 - GPx4
  // EN - GPx3
  // RS - GPx2
  
  // Where GPx can be GPA or GPB, as per MCP23S17 datasheet
  
  // Private variables 
  _rst = rst;
  _cs = cs;
  _port = PORT;
  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  
  //begin(16, 1);  
}

void MCP23S17_LCD::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  expander_setup();	
	
  if (lines > 1) {
   _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);  

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
     _displayfunction |= LCD_5x10DOTS;
  }

  // No RW pin
 
  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
  delayMicroseconds(50000); 
  // Now we pull both RS and R/W low to begin commands
  
  //put the LCD into 4 bit or 8 bit mode
  // this is according to the hitachi HD44780 datasheet
  // figure 24, pg 46

  // we start in 8bit mode, try to set 4 bit mode
  write4bits(0x03, 0);
  delayMicroseconds(4500); // wait min 4.1ms

  // second try
  write4bits(0x03, 0);
  delayMicroseconds(4500); // wait min 4.1ms
    
  // third go!
  write4bits(0x03, 0); 
  delayMicroseconds(150);

  // finally, set to 4-bit interface
  write4bits(0x02, 0); 
 
  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);
  
}

void MCP23S17_LCD::setRowOffsets(int row0, int row1, int row2, int row3)
{
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
/*
void MCP23S17_LCD::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void MCP23S17_LCD::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void MCP23S17_LCD::setCursor(uint8_t col, uint8_t row)
{
  const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
  if ( row >= max_lines ) {
    row = max_lines - 1;    // we count rows starting w/0
  }
  if ( row >= _numlines ) {
    row = _numlines - 1;    // we count rows starting w/0
  }
  
  command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
void MCP23S17_LCD::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void MCP23S17_LCD::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void MCP23S17_LCD::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void MCP23S17_LCD::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void MCP23S17_LCD::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void MCP23S17_LCD::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void MCP23S17_LCD::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void MCP23S17_LCD::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void MCP23S17_LCD::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void MCP23S17_LCD::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void MCP23S17_LCD::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void MCP23S17_LCD::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void MCP23S17_LCD::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */
/*
inline void MCP23S17_LCD::command(uint8_t value) {
  send(value, LOW);
}

inline size_t MCP23S17_LCD::write(uint8_t value) {
  send(value, HIGH);
  return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data
// low-level function to match other Arduino libraries
/*
void MCP23S17_LCD::send(uint8_t value, uint8_t mode) {	
   write8bits(value, (bool) mode);
}

void MCP23S17_LCD::write4bits(uint8_t value, bool RSbit) {
    uint8_t packet = (value << 4) | (RSbit << 2); //check wiring map*******
    // EN = 0
    expander_setOutput(packet);
	delayMicroseconds(5);
    // EN = 1
    expander_setOutput(packet | (1<<3));
    delayMicroseconds(5);
    // EN = 0
    expander_setOutput(packet);
    delayMicroseconds(40);
}
  
void MCP23S17_LCD::write8bits(uint8_t value, bool RSbit) {
    uint8_t nibbleHigh = value >> 4;
    uint8_t nibbleLow = value & 0xF;
    uint8_t packetHigh = (nibbleHigh << 4) | (RSbit << 2);
    uint8_t packetLow = (nibbleLow << 4) | (RSbit << 2);
    // EN = 0
    expander_setOutput(packetHigh);
    delayMicroseconds(10);
    // EN = 1
    expander_setOutput(packetHigh | (1<<3));
    delayMicroseconds(10);
    // EN = 0
    expander_setOutput(packetHigh);
    delayMicroseconds(10);
    // EN = 0
    expander_setOutput(packetLow);
    delayMicroseconds(10);
    // EN = 1
    expander_setOutput(packetLow | (1<<3));
    delayMicroseconds(10);
    // EN = 0
    expander_setOutput(packetLow);
    delayMicroseconds(40);
}

// This function confugires the MCP23S17 port expander
void MCP23S17_LCD::expander_setup(void){
	// Select the correct IOCON register depending on the indicated port
	if (_port == PORTA){
	  _iodir = IODIRA;
    }
    if (_port == PORTB){
	  _iodir = IODIRB;
    }
    // Now, configure the expander	
	// 1. Configure the MCP23S17 control pins
	pinMode(_rst, OUTPUT);
    pinMode(_cs, OUTPUT);
	digitalWrite(_rst, 1);
    digitalWrite(_cs, 1);
	// 2. Start SPI
	SPI.begin();
	// 3. briefly flash the reset pin
    digitalWrite(_rst, 0);
    delayMicroseconds(100);
    digitalWrite(_rst, 1);
    delayMicroseconds(100); 	
	// 4. enable hardware addressing
    expander_sendByte(IOCON, 0b00001000);
    delayMicroseconds(50);
	// configure LCD port direction as output
    expander_sendByte(_iodir, 0);
	// set LCD port as 0
    expander_sendByte(_port, 0);	
	delayMicroseconds(50);
	
	// 5. Debugging only
	// This code will briefly turn on pin 0 of LCD port
	// That pin is used to trigger my Saleae logic level analyzer
	//expander_sendByte(_port, 1);	
	//delayMicroseconds(50);
	//expander_sendByte(_port, 0);	
	//delayMicroseconds(50);
}


// Writes to MCP23S17
void MCP23S17_LCD::expander_sendByte(uint8_t addr, uint8_t tbyte){
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, 0);
    SPI.transfer(OPCODEW);
    SPI.transfer(addr);
    SPI.transfer(tbyte);
    digitalWrite(_cs, 1);
    SPI.endTransaction(); 
}

// Updates the status of MCP23S17 port
void MCP23S17_LCD::expander_setOutput(uint8_t output){
    expander_sendByte(_port, output);
}
*/
//---------------------------------------------------------------------------

#include "MCP23S17_LCD.h"

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
// MCP23S17_LCD constructor is called).


// Configuration for MCP23S17_LCD
// rst  = MCP23S17 RST pin
// cs   = MCP23S17 CS pin
// port = MCP23S17 PORT for LCD
MCP23S17_LCD::MCP23S17_LCD(uint8_t rst, uint8_t cs, uint8_t PORT)
{
   init (rst, cs, PORT);
}

void MCP23S17_LCD::init(uint8_t rst, uint8_t cs, uint8_t PORT)
{
  // NOTE: 4 bits only 
  // Display is connected as folows
  // D7 - GPx7
  // D6 - GPx6
  // D5 - GPx5
  // D4 - GPx4
  // EN - GPx3
  // RS - GPx2
  
  // Where GPx can be GPA or GPB, as per MCP23S17 datasheet
  
  // Private variables 
  _rst = rst;
  _cs = cs;
  _port = PORT;
  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  
  //begin(16, 1); 
}

void MCP23S17_LCD::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, 1);
  delay(100);
  
  expander_setup();	
  lcd_setup();
  //writeByte(IOCONA, 0b00100000);
	_gpioDirection = 0xFFFF;//all in
	_gpioState = 0x0000;//all low 
	
  if (lines > 1) {
   _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);  

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
     _displayfunction |= LCD_5x10DOTS;
  }

  // No RW pin
 
  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
  delayMicroseconds(50000); 
  // Now we pull both RS and R/W low to begin commands
  
  //put the LCD into 4 bit or 8 bit mode
  // this is according to the hitachi HD44780 datasheet
  // figure 24, pg 46

  // we start in 8bit mode, try to set 4 bit mode
  write4bits(0x03, 0);
  delayMicroseconds(4500); // wait min 4.1ms

  // second try
  write4bits(0x03, 0);
  delayMicroseconds(4500); // wait min 4.1ms
    
  // third go!
  write4bits(0x03, 0); 
  delayMicroseconds(150);

  // finally, set to 4-bit interface
  write4bits(0x02, 0); 
 
  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);
  
}

void MCP23S17_LCD::setRowOffsets(int row0, int row1, int row2, int row3)
{
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
void MCP23S17_LCD::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void MCP23S17_LCD::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void MCP23S17_LCD::setCursor(uint8_t col, uint8_t row)
{
  const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
  if ( row >= max_lines ) {
    row = max_lines - 1;    // we count rows starting w/0
  }
  if ( row >= _numlines ) {
    row = _numlines - 1;    // we count rows starting w/0
  }
  
  command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
void MCP23S17_LCD::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void MCP23S17_LCD::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void MCP23S17_LCD::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void MCP23S17_LCD::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void MCP23S17_LCD::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void MCP23S17_LCD::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void MCP23S17_LCD::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void MCP23S17_LCD::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void MCP23S17_LCD::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void MCP23S17_LCD::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void MCP23S17_LCD::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void MCP23S17_LCD::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void MCP23S17_LCD::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

inline void MCP23S17_LCD::command(uint8_t value) {
  send(value, LOW);
}

inline size_t MCP23S17_LCD::write(uint8_t value) {
  send(value, HIGH);
  return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data
// low-level function to match other Arduino libraries
void MCP23S17_LCD::send(uint8_t value, uint8_t mode) {	
   write8bits(value, (bool) mode);
}

void MCP23S17_LCD::write4bits(uint8_t value, bool RSbit) {
    uint8_t packet = (value << 4) | (RSbit << 2); //check wiring map*******
    // EN = 0
    expander_setOutput(packet);
	delayMicroseconds(5);
    // EN = 1
    expander_setOutput(packet | (1<<3));
    delayMicroseconds(5);
    // EN = 0
    expander_setOutput(packet);
    delayMicroseconds(40);
}
  
void MCP23S17_LCD::write8bits(uint8_t value, bool RSbit) {
    uint8_t nibbleHigh = value >> 4;
    uint8_t nibbleLow = value & 0xF;
    uint8_t packetHigh = (nibbleHigh << 4) | (RSbit << 2);
    uint8_t packetLow = (nibbleLow << 4) | (RSbit << 2);
    // EN = 0
    expander_setOutput(packetHigh);
    delayMicroseconds(10);
    // EN = 1
    expander_setOutput(packetHigh | (1<<3));
    delayMicroseconds(10);
    // EN = 0
    expander_setOutput(packetHigh);
    delayMicroseconds(10);
    // EN = 0
    expander_setOutput(packetLow);
    delayMicroseconds(10);
    // EN = 1
    expander_setOutput(packetLow | (1<<3));
    delayMicroseconds(10);
    // EN = 0
    expander_setOutput(packetLow);
    delayMicroseconds(40);
}

// This function confugires the MCP23S17 port expander
void MCP23S17_LCD::expander_setup(void){
	// Select the correct IOCON register depending on the indicated port
	if (_port == PORTA){
	  _iodir = IODIRA;
    }
    if (_port == PORTB){
	  _iodir = IODIRB;
    }
    // Now, configure the expander	
	// 1. Configure the MCP23S17 control pins
	pinMode(_rst, OUTPUT);
    pinMode(_cs, OUTPUT);
	digitalWrite(_rst, 1);
    digitalWrite(_cs, 1);
	// 2. Start SPI
	SPI.begin();
	// 3. briefly flash the reset pin
    digitalWrite(_rst, 0);
    delayMicroseconds(100);
    digitalWrite(_rst, 1);
    delayMicroseconds(100); 	
	// 4. enable hardware addressing
    //writeByte(IOCONA, 0b00100000); //gpio---- 
	expander_sendByte(IOCONB, 0b00001000);
    delayMicroseconds(50);
	//gpio
	//_gpioDirection = 0xFFFF; //all in
	//_gpioState = 0x0000; //all low
	
	// configure LCD port direction as output
    expander_sendByte(_iodir, 0);
	//writeWord(IODIRA, 0);
	
	// set LCD port as 0
    expander_sendByte(_port, 0);
	//writeWord(PORTA, 0);
	delayMicroseconds(50);
	
	// 5. Debugging only
	// This code will briefly turn on pin 0 of LCD port
	// That pin is used to trigger my Saleae logic level analyzer
	//expander_sendByte(_port, 1);	
	//delayMicroseconds(50);
	//expander_sendByte(_port, 0);	
	//delayMicroseconds(50);
}


// Writes to MCP23S17
void MCP23S17_LCD::expander_sendByte(uint8_t addr, uint8_t tbyte){
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, 0);
    SPI.transfer(OPCODEW);
    SPI.transfer(addr);
    SPI.transfer(tbyte);
    digitalWrite(_cs, 1);
    SPI.endTransaction(); 
}

// Updates the status of MCP23S17 port
void MCP23S17_LCD::expander_setOutput(uint8_t output){
    expander_sendByte(_port, output);
}

//function GPIO INPUT/OUTPUT
//set lcd new 2/5/64
void MCP23S17_LCD::lcd_setup(void){
		
	pinMode(_rst, OUTPUT);
    pinMode(_cs, OUTPUT);
	digitalWrite(_rst, 1);
    digitalWrite(_cs, 1);
	// 2. Start SPI
	SPI.begin();
	// 3. briefly flash the reset pin
    digitalWrite(_rst, 0);
    delayMicroseconds(100);
    digitalWrite(_rst, 1);
    delayMicroseconds(100); 	
	// 4. enable hardware addressing
    writeByte(IOCONA, 0b00100000); //gpio---- 
	//expander_sendByte(IOCONB, 0b00001000);
    delayMicroseconds(50);
	//gpio
	//_gpioDirection = 0xFFFF; //all in
	//_gpioState = 0x0000; //all low
	
	// configure LCD port direction as output
    expander_sendByte(IODIRA, 0b00000001);
	//writeWord(IODIRA, 0);
	
	// set LCD port as 0
    //expander_sendByte(PORTA, 0);
	//writeWord(PORTA, 1);
	
	delayMicroseconds(50);
}
//end set

void MCP23S17_LCD::gpioPinMode(uint16_t mode){
	if (mode == INPUT){
		_gpioDirection = 0xFFFF;
	} else if (mode == OUTPUT){	
		_gpioDirection = 0x0000;
		_gpioState = 0x0000;
	} else {
		_gpioDirection = mode;
	}
	//expander_sendByte(IODIRA,_gpioDirection);
	writeWord(IODIRA,_gpioDirection);
}

void MCP23S17_LCD::gpioPinMode(uint8_t pin, bool mode){
	if (pin < 16){//0...7
		mode == INPUT ? _gpioDirection |= (1 << pin) :_gpioDirection &= ~(1 << pin);
		//expander_sendByte(IODIRA,_gpioDirection);
		//writeWord(IODIRA,_gpioDirection);
		writeWord(IODIRA,_gpioDirection);
	}
}

void MCP23S17_LCD::gpioPort(uint16_t value){
	if (value == HIGH){
		_gpioState = 0xFFFF;
	} else if (value == LOW){	
		_gpioState = 0x0000;
	} else {
		_gpioState = value;
	}
	//expander_sendByte(GPIO,_gpioState);
	//writeWord(GPIOA,_gpioState);
	writeWord(PORTA,_gpioState);
}

void MCP23S17_LCD::gpioPort(byte lowByte, byte highByte){
	_gpioState = highByte | (lowByte << 8);
	//expander_sendByte(GPIO,_gpioState);
	//writeWord(GPIOA,_gpioState);
	writeWord(PORTA,_gpioState);
}

void MCP23S17_LCD::gpioDigitalWrite(uint8_t pin, bool value){
	if (pin < 16){//0...15
		value == HIGH ? _gpioState |= (1 << pin) : _gpioState &= ~(1 << pin);
		expander_sendByte(IODIRA,_gpioState);
		//lcd_setup(GPIOA,_gpioState);
		//writeByte(IODIRA,_gpioState);
	}
}

void MCP23S17_LCD::writeByte(uint8_t addr, uint8_t data){
	SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, 0);
    SPI.transfer(addr);
    SPI.transfer(data);
    digitalWrite(_cs, 1);
    SPI.endTransaction(); 
}

void MCP23S17_LCD::writeWord(uint8_t addr, uint16_t data){
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, 0);
    SPI.transfer(addr);
    SPI.transfer(data << 8);
	SPI.transfer(data & 0xFF);
    digitalWrite(_cs, 1);
    SPI.endTransaction(); 
}