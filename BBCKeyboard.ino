//#define DEBUG

#include "BBCKeyboard.h"

// Pin numbers

const int led1Pin = A0;
const int led2Pin = A1;
const int led3Pin = A2;

// Column
const int pa0Pin = 8;
const int pa1Pin = 9;
const int pa2Pin = 10;
const int pa3Pin = 11;

// Row
const int pa4Pin = 4;
const int pa5Pin = 5;
const int pa6Pin = 6;

const int ca2Pin = 12;
const int pa7Pin = 7;
const int breakPin = 13;

const int clkPin =  2;

// Keyboard state
char KeyState[MAXCOL][MAXROW];
char BreakState;

// Current USB report
unsigned char UsbReport[8];


void setup()
{
  // Initialise serial
  Serial.begin(9600);

  // Set up the pins
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);

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
  digitalWrite(led1Pin, 1);
  digitalWrite(led2Pin, 1);
  digitalWrite(led3Pin, 1);

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
  InitCheck();
#endif
}

int ctr = 0;

void loop()
{
  unsigned int Row;
  unsigned int Col;

  // Cycle LEDs
  digitalWrite(led1Pin, (ctr & 0x100) ? 0 : 1);
  digitalWrite(led2Pin, (ctr & 0x200) ? 0 : 1);
  digitalWrite(led3Pin, (ctr & 0x400) ? 0 : 1);
  ++ctr;

  for(Col = 0; Col < 10; Col++){
    SetRowCol(0, Col);
    if(digitalRead(ca2Pin)){
      // There is a key down in this column
      for(Row = 1; Row < 8; Row++){
        SetRowCol(Row, Col);
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
}

void SetRowCol(unsigned int Row, unsigned int Col)
{
  digitalWrite(clkPin, LOW);

  SetRow(Row);
  SetCol(Col);

  digitalWrite(clkPin, HIGH);
}

void SetRow(unsigned int Row)
{
  digitalWrite(pa4Pin, (Row & 0x1 ? HIGH : LOW));
  digitalWrite(pa5Pin, (Row & 0x2 ? HIGH : LOW));
  digitalWrite(pa6Pin, (Row & 0x4 ? HIGH : LOW));
}

void SetCol(unsigned int Col)
{
  digitalWrite(pa0Pin, (Col & 0x1 ? HIGH : LOW));
  digitalWrite(pa1Pin, (Col & 0x2 ? HIGH : LOW));
  digitalWrite(pa2Pin, (Col & 0x4 ? HIGH : LOW));
  digitalWrite(pa3Pin, (Col & 0x8 ? HIGH : LOW));
}

void SetKeyState(unsigned int Col, unsigned int Row, char State)
{
  KeyDet *Key;

  if(KeyState[Col][Row] != State){
    // Key state changed
    KeyState[Col][Row] = State;

    // Get key details
    Key = &KeyDetails[Row][Col];

    // Update the report
    UpdateReport(Key, State);

#ifdef DEBUG
    Serial.write(KeyDetails[Row][Col].Desc);
    Serial.write(" (Row "); 
    Serial.print(Row); 
    Serial.write(" Col "); 
    Serial.print(Col);
    Serial.print(") ");
    switch(State){
    case 'D':
      Serial.write("down");
      break;
    case 'U':
      Serial.write("up");
      break;
    }      
    Serial.print("\n");
#endif

  }
}

void SetBreakState(char State)
{
  KeyDet *Key;

  if(BreakState != State){
    // Change state
    BreakState = State;

    // Get key details
    Key = &BreakDetails;

    // Update the report
    UpdateReport(Key, State);

#ifdef DEBUG
    Serial.write("Break"); 
    switch(State){
    case 'D':
      Serial.write(" down\n");
      break;
    case 'U':
      Serial.write(" up\n");
      break;
    }      
#endif

  }
}

void UpdateReport(KeyDet *Key, unsigned char State)
{
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
#ifdef DEBUG
      Serial.print("REPORT: ");
      for(int Loop = 0; Loop < 8; Loop++){
        Serial.print(UsbReport[Loop], HEX);
        Serial.print(" ");
      }
      Serial.print("\n");
#else
      Serial.write(UsbReport, 8);
      // TODO GET LED STATE BACK?
#endif
    }
  }
}

int AddReport(unsigned int Report)
{
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

#ifdef DEBUG

// Sanity checks

void InitCheck()
{
  unsigned int Col1, Row1;
  unsigned int Col2, Row2;
  unsigned int Elem;
  unsigned int Report;
  unsigned char Used[256];

  for(Elem = 0; Elem < 256; Elem++) Used[Elem] = 0;

  for(Row1 = 0; Row1 < MAXROW; Row1++){
    for(Col1 = 0; Col1 < MAXCOL; Col1++){
      Report = KeyDetails[Row1][Col1].Report;

      if(Report < 256) Used[Report] = 1;

      for(Row2 = 0; Row2 < MAXROW; Row2++){
        for(Col2 = 0; Col2 < MAXCOL; Col2++){
          if(KeyDetails[Row2][Col2].Report == Report && 
            Row1 != Row2 && Col1 != Col2) {
            Serial.write("Duplicated report ");
            Serial.print(Report, HEX);
            Serial.write("\n");
          }
        }
      }    
    }
  }

  for(Elem = 0; Elem < 256; Elem++){
    if((Elem % 16) == 0) Serial.write("\n");
    if(Used[Elem]) Serial.write("X");
    else Serial.write(".");
  }
  Serial.write("\n");
}

#endif


