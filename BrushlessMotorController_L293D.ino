/*
 * MIT License
Copyright (c) 2020 jayanthyk192
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */


/* Tested with Arduino Uno.
 * 
 * This runs the motor in full voltage and does not have a PWM voltage control.
 * 
 * Intended to be used with 12V hard drive motors. DO-NOT use this code/circuit 
 * with an RC Brushless motor.
 * 
 * Make sure you power all the ciruits with an SMPS which shuts down when there is
 * a current surge. DO-NOT run this with a LiPo battery. If there are shot-circuits, 
 * you'll end up with a lot of damage.
 * 
 * DO_NOT run the motor without the 100uF capacitor shown in the schematic, 
 * you'll kill the L293Ds very soon!
 */
 
#define En_A_pin 9
#define En_B_pin 10
#define En_C_pin 11

#define Ph_A_pin 2
#define Ph_B_pin 3
#define Ph_C_pin 4

#define Ph_A 0
#define Ph_B 1
#define Ph_C 2

char disableMosfets = 0; // Set this to 1 to disable all the mosfets

void setup() {
  // Set these pins as output
  pinMode(En_A_pin, OUTPUT);
  pinMode(En_B_pin, OUTPUT);
  pinMode(En_C_pin, OUTPUT);
  
  pinMode(Ph_A_pin, OUTPUT);
  pinMode(Ph_B_pin, OUTPUT);
  pinMode(Ph_C_pin, OUTPUT);

  Serial.begin(115200);
}

void waitBEMF(int BEMF_n) {
  ADMUX = BEMF_n; //Set the mux to the required.
  digitalRead(9); //Dummy read to spend 2
  digitalRead(9); //Dummy read to spend 2
  while (!(ACSR & (1 << ACO))); //Wait till the comparator changes from 0 to 1                                
}

void setPhases(int Ph_high, int Ph_low) {
  //Set all the Enable pins on L293D to low
  digitalWrite(En_A_pin, LOW);
  digitalWrite(En_B_pin, LOW);
  digitalWrite(En_C_pin, LOW);

  //Set all the Phase control pins on L293D to low
  digitalWrite(Ph_A_pin, LOW);
  digitalWrite(Ph_B_pin, LOW);
  digitalWrite(Ph_C_pin, LOW);

  if (!disableMosfets) {
    switch (Ph_high) {
      case Ph_A:
        digitalWrite(En_A_pin, HIGH);
        digitalWrite(Ph_A_pin, HIGH);
        break;
      case Ph_B:
        digitalWrite(En_B_pin, HIGH);
        digitalWrite(Ph_B_pin, HIGH);
        break;
      case Ph_C:
        digitalWrite(En_C_pin, HIGH);
        digitalWrite(Ph_C_pin, HIGH);
        break;
    }
    switch (Ph_low) {
      case Ph_A:
        digitalWrite(En_A_pin, HIGH);
        digitalWrite(Ph_A_pin, LOW);
        break;
      case Ph_B:
        digitalWrite(En_B_pin, HIGH);
        digitalWrite(Ph_B_pin, LOW);
        break;
      case Ph_C:
        digitalWrite(En_C_pin, HIGH);
        digitalWrite(Ph_C_pin, LOW);
        break;
    }
  }
}

void mydelay(long del) {
  for (int i = 0; i < del / 16000; i++) {
    delayMicroseconds(16000);
  }
  delayMicroseconds(del % 16000);
}

void runMotor() {
  /* Run the motor in half step mode. Switching sequence:
      Positive    Negative    Open
         A           B          C
         B           C          A
         C           A          B   
  */
  while (1) {
    setPhases(Ph_A, Ph_B);
    waitBEMF(Ph_C);
    setPhases(Ph_B, Ph_C);
    waitBEMF(Ph_A);
    setPhases(Ph_C, Ph_A);
    waitBEMF(Ph_B);   
    Serial.println('b');
  }
}

void setupComparator() {
  // Clear the ADC settings and Enable Comparator
  ADCSRB = 0b01000000;
  ADCSRA = 0;
}

void test_phases() {
  setPhases(Ph_B, Ph_C);
  while (1) {
    ADMUX = Ph_A;
    digitalRead(9);
    digitalRead(9);
    Serial.println((ACSR & (1 << ACO)));
  }
}

void startMotor(){
  long del = 120000; //Set this depending on the motor.
  while((del-=2000) > 50000){
    setPhases(Ph_A, Ph_B);
    mydelay(del);
    setPhases(Ph_B, Ph_C);
    mydelay(del);
    setPhases(Ph_C, Ph_A);
    mydelay(del);
  }
}

void loop() {
  setupComparator();  
  //test_phases();

  startMotor();
  runMotor();
}
