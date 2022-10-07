//Problemas atuais, so está girando para um lado
#include <Stepper.h>

const int stepsPerRevolution = 1024;  // change this to fit the number of steps per revolution
int angle = 0;
// stepsPerRevolution = 180 graus
// ULN2003 Motor Driver Pins
#define IN1 12
#define IN2 13
#define IN3 15
#define IN4 14

// initialize the stepper library
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

void setup() {
  myStepper.setSpeed(4);
  Serial.begin(115200);
}

void loop() {

}

void moveTo(int value){
  int step = (moveTo-angle)*int(stepsPerRevolution/180);
  myStepper.step(step);
}