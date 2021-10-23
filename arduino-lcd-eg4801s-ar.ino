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

const PROGMEM uint8_t dotBlockData[] = {
  PORTD_DATA_PINS(0b1100),
  PORTD_DATA_PINS(0b1101),
  PORTD_DATA_PINS(0b0101),
  PORTD_DATA_PINS(0b1100)
};
#define DOT_BLOCK_COUNT 4

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
    frame++;

    if (frame == 1) {
      // enable LCD negative voltage drop once some signal comes through
      pinMode(pinVLCD, OUTPUT);
      digitalWrite(pinVLCD, LOW);
    }

    // run the Y-lines
    int dotBlock = 0;
    for (int line = 0; line < 64; line++) {
      // go through X-axis banks
      for (int bank = 0; ; bank++) {
        // unwrapped loop to set pattern from memory and then toggle XSCL up/down
        #define DOTTICK() { \
          uint8_t portXSCLDown = pgm_read_byte(dotBlockData + dotBlock); \
          uint8_t portXSCLUp = portXSCLDown | PORTD_XSCL; \
          PORTD = portXSCLDown; \
          dotBlock++; \
          dotBlock = dotBlock & (DOT_BLOCK_COUNT - 1); \
          NOP(); \
          PORTD = portXSCLUp; \
          NOP(); \
          PORTD = portXSCLDown; \
        }

        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();
        DOTTICK();

        // toggle XECL after a delay (except for the last one which is aligned to latch timing)
        if (bank >= 15) {
          break;
        }

        SET(PORTD, PORTD_XECL);
        NOP();
        CLR(PORTD, PORTD_XECL);
      }

      if (line > 0) {
        // move up YSCL (cannot do this too early after previous latch)
        SET(PORTB, PORTB_PIN_YSCL);
        NOP();

        // drop YSCL after a wait (can run latch immediately after)
        CLR(PORTB, PORTB_PIN_YSCL);
      } else {
        // at start of frame, also move up DIN before YSCL
        SET(PORTB, PORTB_PIN_YSCL);
        NOP();
        SET(PORTB, PORTB_PIN_DIN);
        NOP();

        CLR(PORTB, PORTB_PIN_YSCL);
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
