//Problemas atuais, so está girando para um lado
#include <Stepper.h>

const int stepsPerRevolution = 1024;  // change this to fit the number of steps per revolution
int angle = 0;
const float adjustAngle = float(stepsPerRevolution)/180.0;
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
    moveTo(175);
    delay(5000);
    moveTo(-175);
}

void moveTo(int value){
  value = constrain(value,-175,175);
  Serial.printf("move to %d and now is %d with adjustAngle equal to %f\n",value,angle,adjustAngle);
  // int step = ((value * adjustAngle)-angle)*(stepsPerRevolution/180);
  int step = (value-angle)*(adjustAngle);
  myStepper.step(step);
  angle=value;
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

}