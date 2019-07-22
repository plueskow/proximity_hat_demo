#include <Servo.h>
#include <Math.h>

#define SERVO_NUM 6
#define LEVEL_NUM 9

int servoDefault[SERVO_NUM] = {71, 67, 92, 77, 62, 69};
int servosPos[SERVO_NUM];
float offset[SERVO_NUM] = {1.0, 1.25, 1.0, 0.8, 0.9, 0.8};
Servo servos[SERVO_NUM];

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean newLine = false;

int currentServo = 0;

void setup() {
  Serial.begin(115200);

  // attach servos
  for(int i=0; i< SERVO_NUM; i++) {
    servos[i].attach(i+2);
  } 

  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop() {
  for (int i = 0; i < SERVO_NUM; i++){
    while (inputString.equals(""))
      inputString = Serial.readStringUntil(';');
    
    int level = inputString.toInt();
    float input = ((float)level)/9.0 * 50.0;
    
    servosPos[i] = (int)(servoDefault[i] + (50 * input) * offset[i]);
    
    // clear the string:
    inputString = "";
  }
  oneSensorCycle();
}

void oneSensorCycle() { // sensor ping cycle complete
  for(int i = 0; i < SERVO_NUM; i++) {
    servos[i].write(servosPos[i]);
    Serial.print(i);
    Serial.print(" ");
    Serial.print(servosPos[i]);
    Serial.print(" ");
  }
  Serial.println("");
}























