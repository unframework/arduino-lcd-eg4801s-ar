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

  pinMode(13, OUTPUT);
}

void loop() {
  int frame = 0;

  while(1) {
    frame += 1;

    digitalWrite(13, frame % 2 ? HIGH : LOW);

    // run the Y-lines
    for (int line = 0; line < 64; line += 1) {
      // move up YSCL
      digitalWrite(pinYSCL, HIGH);

      // at start of frame, also move up DIN
      if (line == 0) {
        digitalWrite(pinDIN, HIGH);
      }

      // drop YSCL after a wait
      delayMicroseconds(4);
      digitalWrite(pinYSCL, LOW);

      // if DIN was up due to start of frame, lower it after a wait
      if (line == 0) {
        delayMicroseconds(4);
        digitalWrite(pinDIN, LOW);
      }

      // always set up the first XECL of dot block series
      digitalWrite(pinXECL, HIGH);
      delayMicroseconds(4);

      // set up latch
      digitalWrite(pinLP, HIGH);

      // drop XECL midway while latch is up
      delayMicroseconds(4);
      digitalWrite(pinXECL, LOW);
      delayMicroseconds(4);

      // finally drop latch
      digitalWrite(pinLP, LOW);

      // on first line, toggle frame signal around the same time as latch drop
      if (line == 0) {
        digitalWrite(pinFR, frame & 1 ? LOW : HIGH);
      }

      // run the X-axis
      for (int dot = 0; dot < 256; dot += 1) {
        // toggle XECL (except the first one that is aligned to latch timing)
        if (dot != 0 && dot % 16 == 0) {
          digitalWrite(pinXECL, HIGH);
          delayMicroseconds(4);
          digitalWrite(pinXECL, LOW);
          delayMicroseconds(4);
        }

        // send data
        int val1 = line & 1 ? HIGH : LOW;
        int val2 = line & 1 ? LOW : HIGH;
        digitalWrite(pinD0, val1);
        digitalWrite(pinD1, val2);
        digitalWrite(pinD2, val1);
        digitalWrite(pinD3, val2);

        // toggle XSCL up and down
        digitalWrite(pinXSCL, HIGH);
        delayMicroseconds(4);
        digitalWrite(pinXSCL, LOW);
        delayMicroseconds(4);
      }
    }
  }
}
