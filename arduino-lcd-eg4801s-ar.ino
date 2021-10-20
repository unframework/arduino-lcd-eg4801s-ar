const int pinVLCD = 13;

// X-driver: pins 2-7
#define PORTD_XSCL (1 << 2)
#define PORTD_XECL (1 << 3)
#define PORTD_D0   (1 << 4)
#define PORTD_D1   (1 << 5)
#define PORTD_D2   (1 << 6)
#define PORTD_D3   (1 << 7)

#define PORTD_DATA_PINS(value4bit) (value4bit << 4)

// Y-scan, frame: pins 8-11
#define PORTB_PIN_YSCL (1 << 0)
#define PORTB_PIN_DIN  (1 << 1)
#define PORTB_PIN_LP   (1 << 2)
#define PORTB_PIN_FR   (1 << 3)

#define SET(port, pinset) (port |= pinset)
#define CLR(port, pinset) (port &= (~pinset))

#define SET_MASK(port, mask, pinset) (port = (port & (~mask)) | pinset)

#define NOP() __asm__("nop\n\t")

void setup() {
  SET(DDRD, (
    PORTD_XSCL |
    PORTD_XECL |
    PORTD_D0 |
    PORTD_D1 |
    PORTD_D2 |
    PORTD_D3
  ));

  SET(DDRB, (
    PORTB_PIN_YSCL |
    PORTB_PIN_DIN |
    PORTB_PIN_LP |
    PORTB_PIN_FR
  ));

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
        SET_MASK(
          PORTD,
          PORTD_DATA_PINS(0b1111),
          PORTD_DATA_PINS(myval)
        );

        // toggle XSCL up and down
        SET(PORTD, PORTD_XSCL);
        NOP();
        CLR(PORTD, PORTD_XSCL);

        // toggle XECL after a delay (except for the last one which is aligned to latch timing)
        if (dot != 255 && (dot & 15) == 15) {
          SET(PORTD, PORTD_XECL);
          NOP();
          CLR(PORTD, PORTD_XECL);
        }
      }

      // move up YSCL (cannot do this too early after previous latch)
      SET(PORTB, PORTB_PIN_YSCL);
      NOP();

      // at start of frame, also move up DIN
      int useDIN = line < 2;
      if (useDIN) {
        SET(PORTB, PORTB_PIN_DIN);
        NOP();
      }

      // drop YSCL after a wait (can run latch immediately after)
      CLR(PORTB, PORTB_PIN_YSCL);

      // if DIN was up due to start of frame, lower it after a wait
      if (useDIN) {
        NOP();
        CLR(PORTB, PORTB_PIN_DIN);
      }


      // set up the last XECL of dot block series
      SET(PORTD, PORTD_XECL);
      NOP(); // hold a bit more just in case

      // set up latch
      SET(PORTB, PORTB_PIN_LP);

      // drop XECL midway while latch is up
      NOP(); // hold a bit more just in case
      CLR(PORTD, PORTD_XECL);
      NOP(); // hold a bit more just in case

      if (line == 63) {
        // on last line, toggle frame signal around the same time as latch drop
        SET_MASK(
          PORTB,
          (PORTB_PIN_LP | PORTB_PIN_FR), // affect these two pins
          frame & 1 ? 0 : PORTB_PIN_FR // always zero for LP + toggle FR
        );
      } else {
        // normally just drop latch
        CLR(PORTB, PORTB_PIN_LP);
      }
    }
  }
}
