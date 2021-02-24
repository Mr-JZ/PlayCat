#include <Servo.h>

Servo servoA;
Servo servoB;

int state = 0;

int servoAPos = 90;
int servoBPos = 90;
int nextServoAPos = 0;
int nextServoBPos = 0;

void setup() {
  servoA.attach(9);
  servoB.attach(10);
  Serial.begin(9600);
}

void loop() {
  while(Serial.available() > 0){
    char data = Serial.read();
    switch (state) {
    case 0:
        switch (data) {
        case 's':
            state++;
            break;
        default:
            state = 0;
            break;
        }
        break;
    case 1:
        switch (data) {
        case 'e':
            state++;
            break;
        default:
            state = 0;
            break;
        }
        break;
    case 2:
        nextServoAPos = ((int)data) + 128;
        state++;
        break;
    case 3:
        switch (data) {
        case 'r':
            state++;
            break;
        default:
            state = 0;
            break;
        }
        break;
    case 4:
        nextServoBPos = ((int)data) + 128;
        state++;
        break;
    case 5:
        switch (data) {
        case 'v':
            state++;
            break;
        default:
            state = 0;
            break;
        }
        break;
    case 6:
        switch (data) {
        case 'o':
            state = 0;
            servoAPos = capServo(nextServoAPos);
            servoBPos = capServo(nextServoBPos);
            
            break;
        default:
            state = 0;
            break;
        }
        break;
    }
  }
  
  servoA.write(servoAPos);
  servoB.write(servoBPos);
  delay(20);
}

int capServo(int pos){
  if(pos < 5){
    return 5;
  }
  if(pos > 175){
    return 175;
  }
  return pos;
}
