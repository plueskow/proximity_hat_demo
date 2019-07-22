#include <NewPingMod.h>
#include <Servo.h>
#include <Math.h>
#include <RunningMedian.h>

#define SONAR_NUM 6
#define SERVO_NUM 6
#define MEDIAN_COUNT 16
#define MAX_DISTANCE 250 
#define PING_INTERVAL 100 // time inbetween pigs
#define RELAX 13 // 

RunningMedian samples[6] = {
  RunningMedian(MEDIAN_COUNT), 
  RunningMedian(MEDIAN_COUNT), 
  RunningMedian(MEDIAN_COUNT), 
  RunningMedian(MEDIAN_COUNT),
  RunningMedian(MEDIAN_COUNT), 
  RunningMedian(MEDIAN_COUNT)
};
unsigned long pingTimer;
unsigned int distance[SONAR_NUM];

int servoDefault[SERVO_NUM] = {71, 67, 92, 77, 62, 69};
float offset[SERVO_NUM] = {1.0, 1.25, 1.0, 0.8, 0.9, 0.8};
Servo servos[SERVO_NUM];
float servosPos[SERVO_NUM];
boolean echoFlag[SONAR_NUM]; // flag if no echo is received

NewPing sonar[6] = {
  NewPing(A0, 8, MAX_DISTANCE), // trigger pin, echo pin, max distance
  NewPing(A1, 9, MAX_DISTANCE),
  NewPing(A2, 10, MAX_DISTANCE),
  NewPing(A3, 11, MAX_DISTANCE),
  NewPing(A4, 12, MAX_DISTANCE),
  NewPing(A5, 13, MAX_DISTANCE),
};

void setup() {
  Serial.begin(115200);
  // pinging will start after a short period of time
  pingTimer = millis() + 250; 
  // attach servos
  for(int i=0; i< SERVO_NUM; i++) {
    servos[i].attach(i+2);
  } 

  // initialise median
  for (int i = 0; i < SONAR_NUM; i++) {
    for (int j = 0; j < MEDIAN_COUNT; j++) {
      samples[i].add(MAX_DISTANCE);
    }
  }
}

void loop() {
  if (millis() >= pingTimer) {
    sonar[0].timer_stop(); // cancel previous timer just in case

    oneSensorCycle();
    pingTimer += PING_INTERVAL;  // next time this sensor will be pinged

    sonar[0].ping_timer_all(echoCheck); // start ping. interrupt will call echoCheck
    //boolean started = sonar[0].ping_trigger_all(); // start ping. interrupt will call echoCheck
    //Serial.println(started);

    for (uint8_t i = 0; i < SONAR_NUM; i++) {
      echoFlag[i] = false;
      sonar[i]._max_time = sonar[0]._max_time; // sync sensors
    }
  }
}

void echoCheck() { // If ping received, store the distance
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // check all sensors
//  for (uint8_t i = 0; i < 1; i++) { // check all sensors
    if (!echoFlag[i] && sonar[i].check_timer_all()) {
      echoFlag[i] = true;   
    }
  }
}

void updateServo(int currentServo, float distance) {
  distance = max(30.0, distance);
  float logScaled = fscale(30.0, 251.0, 0.0, 1.0, distance, 2.5); // logarithmic mapping of distance to pressure
  servosPos[currentServo] = servoDefault[currentServo] - RELAX + 50 * offset[currentServo] - (50 * logScaled);
  //  servosPos[currentServo] = servoDefault[currentServo] - RELAX + 50 * offset[currentServo] - (50 * logScaled * offset[currentServo]); // test this
  servos[currentServo].write((int) servosPos[currentServo]); 
}

void oneSensorCycle() { // sensor ping cycle complete
  for(int i = 0; i < SONAR_NUM; i++) {
    // read values
    if(echoFlag[i]) {
      float newDistance =  sonar[i].ping_result / US_ROUNDTRIP_CM;
      samples[i].add(newDistance);
    } else {
      samples[i].add((float) MAX_DISTANCE);
    }
   
    float median = samples[i].getMedian();
    updateServo(i, median);
    Serial.print(i);
    Serial.print(" ");
    Serial.print(median);
    Serial.print(" ");
  }
  Serial.println("");
}

float fscale( float originalMin, float originalMax, float newBegin, float
newEnd, float inputValue, float curve){

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;

  // condition curve parameter
  // limit range

  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;

  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero Refference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin){
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax ) {
    return 0;
  }

  if (invFlag == 0){
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else     // invert the ranges
  {  
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}























