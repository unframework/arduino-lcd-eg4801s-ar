const int pinLP = 2;
const int pinFR = 3;
const int pinYSCL = 4;
const int pinDIN = 5;
const int pinXSCL = 6;
const int pinXECL = 7;
const int pinD0 = 8;
const int pinD1 = 9;
const int pinD2 = 10;
const int pinD3 = 11;
const int pinVLCD = 13;

#define PORT_A_PIN_2  0b00000100
#define PORT_A_PIN_3  0b00001000
#define PORT_A_PIN_4  0b00010000
#define PORT_A_PIN_5  0b00100000
#define PORT_A_PIN_6  0b01000000
#define PORT_A_PIN_7  0b10000000

#define PORT_B_PIN_8  0b00000001
#define PORT_B_PIN_9  0b00000010
#define PORT_B_PIN_10 0b00000100
#define PORT_B_PIN_11 0b00001000
#define PORT_B_PIN_12 0b00010000
#define PORT_B_PIN_13 0b00100000

#define SET(port, pinset) (port |= pinset)
#define CLR(port, pinset) (port &= (~pinset))

// pins:

// on PORTD:
// XSCL = 2
// XECL = 3
// D0 = 4
// D1 = 5
// D2 = 6
// D3 = 7

// on PORTB:
// YSCL = 8
// DIN = 9
// LP = 10
// FR = 11

// @todo set up dedicated macro for PORTD to set D0-D3 at once, etc

void setup() {
  pinMode(pinLP, OUTPUT);
  pinMode(pinFR, OUTPUT);
  pinMode(pinYSCL, OUTPUT);
  pinMode(pinDIN, OUTPUT);
  pinMode(pinXSCL, OUTPUT);
  pinMode(pinXECL, OUTPUT);
  pinMode(pinD0, OUTPUT);
  pinMode(pinD1, OUTPUT);
  pinMode(pinD2, OUTPUT);
  pinMode(pinD3, OUTPUT);

  pinMode(pinVLCD, INPUT_PULLUP);
}

void loop() {
  int frame = 0;

  while(1) {
    frame += 1;

    if (frame == 1) {
      // enable LCD negative voltage drop once some signal comes through
      pinMode(pinVLCD, OUTPUT);
      digitalWrite(pinVLCD, LOW);
    }

    // run the Y-lines
    for (int line = 0; line < 64; line += 1) {
      // run the X-axis
      for (int dot = 0; dot < 256; dot += 1) {
        // send data
        // int val1 = line & 1 ? HIGH : LOW;
        // int val2 = line & 1 ? LOW : HIGH;
        // digitalWrite(pinD0, val1);
        // digitalWrite(pinD1, val1);
        // digitalWrite(pinD2, val2);
        // digitalWrite(pinD3, val2);

        // diagonal stripe pattern
        int myval = ((line + dot) & 15) < 8 ? 15 : 0;
        digitalWrite(pinD0, (myval & 1));
        digitalWrite(pinD1, (myval & 2) >> 1);
        digitalWrite(pinD2, (myval & 4) >> 2);
        digitalWrite(pinD3, (myval & 8) >> 3);

        // toggle XSCL up and down
        digitalWrite(pinXSCL, HIGH);
        digitalWrite(pinXSCL, LOW);

        // toggle XECL after a delay (except for the last one which is aligned to latch timing)
        if (dot != 255 && (dot & 15) == 15) {
          digitalWrite(pinXECL, HIGH);
          digitalWrite(pinXECL, LOW);
        }
      }

      // move up YSCL (cannot do this too early after previous latch)
      digitalWrite(pinYSCL, HIGH);

      // at start of frame, also move up DIN
      int useDIN = line < 2;
      if (useDIN) {
        digitalWrite(pinDIN, HIGH);
      }

      // drop YSCL after a wait (can run latch immediately after)
      digitalWrite(pinYSCL, LOW);

      // if DIN was up due to start of frame, lower it after a wait
      if (useDIN) {
        digitalWrite(pinDIN, LOW);
      }


      // set up the last XECL of dot block series
      digitalWrite(pinXECL, HIGH);

      // set up latch
      digitalWrite(pinLP, HIGH);

      // drop XECL midway while latch is up
      digitalWrite(pinXECL, LOW);

      if (line == 63) {
        // on last line, toggle frame signal around the same time as latch drop
        digitalWrite(pinFR, frame & 1 ? LOW : HIGH);
      }

      // finally drop latch
      digitalWrite(pinLP, LOW);
    }
  }
}
