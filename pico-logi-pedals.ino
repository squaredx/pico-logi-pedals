#include <PicoGamepad.h>

#define THROTTLE 0
#define BRAKE 1
#define CLUTCH 2

const int THROTTLE_PIN = 28;
const int BRAKE_PIN = 27;
const int CLUTCH_PIN = 26;

const int INPUT_MAX = 1023;

const int NUM_PEDALS = 3;
const int SMOOTHING_READINGS = 5;

PicoGamepad gamepad;

int rawValues[NUM_PEDALS];
int previousValues[NUM_PEDALS][SMOOTHING_READINGS];
int indices[NUM_PEDALS];
int totals[NUM_PEDALS];
int averages[NUM_PEDALS];

int pedalMax[NUM_PEDALS];
int pedalMin[NUM_PEDALS];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // board pin 34
  pinMode(THROTTLE_PIN, INPUT);
  // board pin 32
  pinMode(BRAKE_PIN, INPUT);
  // board pin 31
  pinMode(CLUTCH_PIN, INPUT);

  for (int pedalIndex = 0; pedalIndex < NUM_PEDALS; pedalIndex++) {
    rawValues[pedalIndex] = 0;
    indices[pedalIndex] = 0;
    totals[pedalIndex] = 0;
    averages[pedalIndex] = 0;
    pedalMax[pedalIndex] = 0;
    pedalMin[pedalIndex] = INPUT_MAX;
    
    for (int readingIndex = 0; readingIndex < SMOOTHING_READINGS; readingIndex++) {
      previousValues[pedalIndex][readingIndex] = 0;
    }
  }
}

void loop() {
  for(int pedalIndex = 0; pedalIndex < NUM_PEDALS; pedalIndex++) {
    totals[pedalIndex] = totals[pedalIndex] - previousValues[pedalIndex][indices[pedalIndex]];
    previousValues[pedalIndex][indices[pedalIndex]] = getMappedPedalPosition(pedalIndex, analogRead(getPedalPin(pedalIndex)));
    totals[pedalIndex] = totals[pedalIndex] + previousValues[pedalIndex][indices[pedalIndex]];
    indices[pedalIndex] = indices[pedalIndex] + 1;
  
    if (indices[pedalIndex] >= SMOOTHING_READINGS) {
      indices[pedalIndex] = 0;
    }
  
    averages[pedalIndex] = totals[pedalIndex] / SMOOTHING_READINGS;
  }
// Update Gamepad
  gamepad.SetRx(map((averages[CLUTCH] - averages[THROTTLE])+INPUT_MAX, 0, INPUT_MAX*2, -32767, 32767));
  gamepad.SetRy(map(averages[BRAKE], 0, INPUT_MAX, -32767, 32767));
 
  gamepad.send_update();

  delay(50);
}


// FROM: https://github.com/mohragk/G27_Pedal_USB/blob/master/g27_pedals/g27_pedals.ino
int getMappedPedalPosition(int pedal, int rawPosition) {
    // store (new) extremes of the pedal
    pedalMin[pedal] = min(rawPosition, pedalMin[pedal]);
    pedalMax[pedal] = max(rawPosition, pedalMax[pedal]);

    // remap the position to a 0 - 1023 range
    int mappedPostion = map(rawPosition, pedalMin[pedal], pedalMax[pedal], INPUT_MAX, 0);
    return mappedPostion;
}

int getPedalPin(int pedal) {
  switch(pedal) {
    case THROTTLE:
      return THROTTLE_PIN;
    case BRAKE:
      return BRAKE_PIN;
    case CLUTCH:
      return CLUTCH_PIN;
    default:
      return -1;
  }
}
