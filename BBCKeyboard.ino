// Debug flag
//#define DEBUG

#include "BBCKeyboard.h"

// Output types
#define OT_SERIAL 1
#define OT_KEYBOARD 2

#if defined(__AVR_ATmega32U4__)
// Arduino leonardo / micro - USB built in - use keyboard library to report keys
# define OUTPUTTYPE OT_KEYBOARD

#elif defined(__AVR_ATmega328p__)
// Arduino Uno - USB not built in. Output is always serial - an 8 byte report if DEBUG not set for the keyboard firmware
# define OUTPUTTYPE OT_SERIAL

#else
# error "Unrecognised mcu type"

#endif

// BBC connector pinout and arduino equivalent
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// | 0v |RST |1mhz|KBEN|PA4 |PA5 |PA6 |PA0 |PA1 |PA2 |PA3 |PA7 |LED3|CA2 | 5v |LED1|LED2|
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |GND |D2  |D3  |GND |D4  |D5  |D6  |D7  |D8  |D9  |D10 |D11 |A2  |D12 | 5V |A1  |A0  |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+

// Pin numbers

const int ledShiftLockPin = A1;
const int ledCapsLockPin = A0;
const int ledCassPin = A2;

// Column
const int pa0Pin = 7;
const int pa1Pin = 8;
const int pa2Pin = 9;
const int pa3Pin = 10;

// Row
const int pa4Pin = 4;
const int pa5Pin = 5;
const int pa6Pin = 6;

const int pa7Pin = 11;
const int ca2Pin = 12;
const int breakPin = 2;

const int clkPin =  3;

// Keyboard state
char KeyState[MAXCOL][MAXROW];
char BreakState;

// Current USB report
unsigned char UsbReport[8];

// LED status
#define LEDMODE_REPORT (-1)
#define LEDMODE_SCROLL (-2)

int LedMode = LEDMODE_REPORT;
uint8_t LastLedStatus = 0;

int LedCtr1 = 0;
int LedCtr2 = 0;
uint8_t LedScroll[] = {
  1, 3, 7, 6, 4, 0}; // xxO xOO OOO OOx Oxx xxx

void setup()
{
  // Set up Serial
#if OUTPUTTYPE == OT_SERIAL
  // Initialise serial for report / debug output
  Serial.begin(9600);

#elif OUTPUTTYPE == OT_KEYBOARD
  // Initialise the keyboard output
  Keyboard.begin();
  Serial.begin(9600);

# ifdef DEBUG
  // Initialise serial and wait if debugging active
  while(!Serial);

  Serial.print("KEYBOARD output\n");
# endif

#else
  // unknown output type
# error "UNKNOWN OUTPUTTYPE!"

#endif

  // Set up the pins
  pinMode(ledShiftLockPin, OUTPUT);
  pinMode(ledCapsLockPin, OUTPUT);
  pinMode(ledCassPin, OUTPUT);

  pinMode(pa0Pin, OUTPUT);      
  pinMode(pa1Pin, OUTPUT);      
  pinMode(pa2Pin, OUTPUT);      
  pinMode(pa3Pin, OUTPUT);      

  pinMode(pa4Pin, OUTPUT);      
  pinMode(pa5Pin, OUTPUT);      
  pinMode(pa6Pin, OUTPUT);      

  pinMode(ca2Pin, INPUT);
  pinMode(pa7Pin, INPUT);      
  pinMode(breakPin, INPUT);
  digitalWrite(breakPin, HIGH); // Pull up resistor on break pin

  pinMode(clkPin, OUTPUT);

  // Turn LEDs off
  digitalWrite(ledShiftLockPin, 1);
  digitalWrite(ledCapsLockPin, 1);
  digitalWrite(ledCassPin, 1);

  // Initialise key state
  for(int Col = 0; Col < MAXCOL; Col++){    
    for(int Row = 0; Row < MAXCOL; Row++){
      KeyState[Col][Row] = 'U';
    }
  }
  BreakState = 'U';

  // Initialise the USB report
  for(int Loop = 0; Loop < 8; Loop++){
    UsbReport[Loop] = 0;
  }

#ifdef DEBUG
  // Debug initialisation checks
  InitCheck();
#endif
}

void loop()
{
  unsigned int Row;
  unsigned int Col;
  uint8_t LedStatus;
  unsigned char SerChar;

  // Check each column in turn
  for(Col = 0; Col < 10; Col++){
    SetRowCol(0, Col);

    // If CA2 pin raised then a key on this column is pressed
    if(digitalRead(ca2Pin)){
      // There is a key down in this column
      for(Row = 1; Row < 8; Row++){
        // Check each key in this column
        SetRowCol(Row, Col);

        // If PA7 pin raised then this key is down
        if(digitalRead(pa7Pin)){
          // This key is down
          SetKeyState(Col, Row, 'D');
        } 
        else {
          // This key is up
          SetKeyState(Col, Row, 'U');
        }
      }
    }
    else{
      // No keys down in this column
      for(Row = 1; Row < 8; Row++){
        SetKeyState(Col, Row, 'U');
      }        
    }
  }

  // Check Row 0
  for(Col = 0; Col < 10; Col++){
    SetRowCol(0, Col);

    // If PA7 pin raised then this key is down
    if(digitalRead(pa7Pin)){
      // This key is down
      SetKeyState(Col, 0, 'D');
    } 
    else {
      // This key is up
      SetKeyState(Col, 0, 'U');
    }
  }

  // Check break key
  if(digitalRead(breakPin)) {
    SetBreakState('U');
  }
  else{
    SetBreakState('D');
  }

#if OUTPUTTYPE == OT_KEYBOARD
  // Got any data on the serial channel?
  while(Serial.available() > 0){
    SerChar = Serial.read();

#ifdef DEBUG
    Serial.write("Read char ");
    Serial.print(SerChar, HEX);
    Serial.write(" from Serial\n");
#endif

    switch(SerChar){
    case 'R':
    case 'r':
      LedMode = LEDMODE_REPORT;
      break;
    case 'S':
    case 's':
      LedMode = LEDMODE_SCROLL;
      break;
    default:
      if(SerChar >= '0' && SerChar <= '7'){
        LedMode = SerChar - '0';
      }
    }
  }
#endif

  // Update LED state
  switch(LedMode){
  case LEDMODE_SCROLL:
    // Scroll LEDs
    if(++LedCtr1 >= 80){
      LedCtr1 = 0;
      if(++LedCtr2 == 6) LedCtr2 = 0;
      SetLedStatus(LedScroll[LedCtr2]);
    }
    break;

  case LEDMODE_REPORT:
    // Get LED state from HID / Serial
#if OUTPUTTYPE == OT_KEYBOARD
    // Update LED state from HID
    LedStatus = Keyboard.getLedStatus();
#elif OUTPUTTYPE == OT_SERIAL
    // Get LED state from serial
    while(Serial.available() > 0){
      LedStatus = Serial.read();
    }
#endif

    if(LedStatus != LastLedStatus){
#ifdef DEBUG
      Serial.write("LED status changed from ");
      Serial.print(LastLedStatus, HEX);
      Serial.write(" to ");
      Serial.print(LedStatus, HEX);
      Serial.write("\n");
#endif

      SetLedStatus(LedStatus);
    }
    break;

  default:
    // Forced LED mode
    SetLedStatus(LedMode);
    break;

  }
}

void SetRowCol(unsigned int Row, unsigned int Col)
{
  // Clock the keyboard array row and column on

  digitalWrite(clkPin, LOW);

  SetRow(Row);
  SetCol(Col);

  digitalWrite(clkPin, HIGH);
}

void SetRow(unsigned int Row)
{
  // Set current keyboard array row

  digitalWrite(pa4Pin, (Row & 0x1 ? HIGH : LOW));
  digitalWrite(pa5Pin, (Row & 0x2 ? HIGH : LOW));
  digitalWrite(pa6Pin, (Row & 0x4 ? HIGH : LOW));
}

void SetCol(unsigned int Col)
{
  // Set current keyboard array column

  digitalWrite(pa0Pin, (Col & 0x1 ? HIGH : LOW));
  digitalWrite(pa1Pin, (Col & 0x2 ? HIGH : LOW));
  digitalWrite(pa2Pin, (Col & 0x4 ? HIGH : LOW));
  digitalWrite(pa3Pin, (Col & 0x8 ? HIGH : LOW));
}

void SetKeyState(unsigned int Col, unsigned int Row, char State)
{
  // Set a key to a given state

  KeyDet *Key;

  if(KeyState[Col][Row] != State){

    // Key state changed
    KeyState[Col][Row] = State;

    // Get key details
    Key = &KeyDetails[Row][Col];

#ifdef DEBUG
    Serial.write("Key ");
    Serial.write(Key->Desc);
    Serial.write(" (Col ");
    Serial.print(Col);
    Serial.write(" Row ");
    Serial.print(Row);
    Serial.write(") ");
    switch(State){
    case 'D':
      Serial.write("down");
      break;
    case 'U':
      Serial.write("up");
      break;
    }      
    Serial.write("\n");
#endif

    // Update key
    UpdateReport(Key, State);
  }
}

void SetBreakState(char State)
{
  // Set the BREAK key state

  KeyDet *Key;

  if(BreakState != State){
    // Change state
    BreakState = State;

    // Get key details
    Key = &BreakDetails;

#ifdef DEBUG
    Serial.write("Break ");
    switch(State){
    case 'D':
      Serial.write("down");
      break;
    case 'U':
      Serial.write("up");
      break;
    }      
    Serial.write("\n");
#endif

    // Update key
    UpdateReport(Key, State);
  }
}

void UpdateReport(KeyDet *Key, unsigned char State)
{
  // Update the report with a given key and state

  unsigned int Report;
  int SendReport;

  // Get report key
  Report = Key->Report;

  if(Report != 0){
    // Update the report    
    if(State == 'D') SendReport = AddReport(Report);
    else SendReport = RemoveReport(Report);

    // Send if required
    if(SendReport){

#if OUTPUTTYPE == OT_SERIAL
      // Use serial output
      Serial.write(UsbReport, 8);
#elif OUTPUTTYPE == OT_KEYBOARD
      // Use keyboard HID output
      HID_SendReport(2, UsbReport, 8);
#endif

#ifdef DEBUG
      // Dump the report
      DumpReport();
#endif
    }
  }
}

int AddReport(unsigned int Report)
{
  // Add a key to the report

  int Changed = 0;
  unsigned char Byte;

  if(Report & MODKEY){
    // Add modifier key
    Byte = UsbReport[0] | (Report & 0xff);
    if(UsbReport[0] != Byte){
      UsbReport[0] = Byte;
      Changed = 1;
    }
  }
  else{
    // Add normal key
    for(int Loop = 2; Loop < 8; Loop++){
      if(UsbReport[Loop] == 0x00){
        UsbReport[Loop] = Report;
        Changed = 1;
        break;
      }
    }
  }

  return Changed;
}

int RemoveReport(unsigned int Report)
{
  // Remove a key from the report

  int Changed = 0;
  unsigned char Byte;

  if(Report & MODKEY){
    // Remove modifier key
    Byte = UsbReport[0] & ~((unsigned char) (Report & 0xff));
    if(UsbReport[0] != Byte){
      UsbReport[0] = Byte;
      Changed = 1;
    }
  }
  else{
    // Remove normal key
    for(int Loop = 2; Loop < 8; Loop++){
      if(UsbReport[Loop] == Report){
        UsbReport[Loop] = 0;
        Changed = 1;
        break;
      }
    }
  }

  return Changed;
}

void SetLedStatus(uint8_t LedStatus)
{
  digitalWrite(ledShiftLockPin, (LedStatus & 0x1) ? 0 : 1);
  digitalWrite(ledCapsLockPin, (LedStatus & 0x2) ? 0 : 1);
  digitalWrite(ledCassPin, (LedStatus & 0x4) ? 0 : 1);

  LastLedStatus = LedStatus;
}

#ifdef DEBUG

void DumpReport()
{
  // Dump current key report

  Serial.print("REPORT: ");
  for(int Loop = 0; Loop < 8; Loop++){
    Serial.print(UsbReport[Loop], HEX);
    Serial.print(" ");
  }
  Serial.print("\n");
}

void InitCheck()
{
  // Sanity checks

  unsigned int Col, Row;
  unsigned int Elem;
  unsigned int Report;
  unsigned char Used[256];
  unsigned char ModUsed;

  for(Elem = 0; Elem < 256; Elem++) Used[Elem] = 0;
  ModUsed = 0;

  for(Row = 0; Row < MAXROW; Row++){
    for(Col = 0; Col < MAXCOL; Col++){
      Report = KeyDetails[Row][Col].Report;
      CheckReport(Report, Used, &ModUsed);
    }
  }

  Report = BreakDetails.Report;
  CheckReport(Report, Used, &ModUsed);

  for(Elem = 0; Elem < 256; Elem++){
    if((Elem % 16) == 0){
      if(Elem == 0) Serial.write("    0123456789abcdef");
      Serial.write("\n");
      Serial.print((Elem / 16) & 0xf0, HEX);
      Serial.print((Elem / 16) & 0x0f, HEX);
      Serial.write(": ");
    }
    if(Used[Elem]) Serial.write("X");
    else Serial.write(".");
  }
  Serial.write("\n");
}

void CheckReport(unsigned int Report, unsigned char *Used, unsigned char *ModUsed)
{
  if(Report == 0) return;

  if(Report >= 256){
    if(Report & MODKEY && (Report & ~MODKEY) < 256){
      Report &= 0x0f;
      if(*ModUsed & Report){
        Serial.write("Duplicated modifier ");
        Serial.print(Report, HEX);
        Serial.write("\n");            
      }
      *ModUsed |= Report;
    }
    else{
      Serial.write("Unrecognised report ");
      Serial.print(Report, HEX);
      Serial.write("\n");            
    }
  }
  else{
    if(Used[Report]){
      Serial.write("Duplicated report ");
      Serial.print(Report, HEX);
      Serial.write("\n");            
    }
    Used[Report] = 1;
  }
}

#endif










