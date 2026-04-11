#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Servo.h>

#define TOTAL_SERVOS 22
#define NUM_DIRECT_SERVOS 6

// ======================
// PCA9685 Setup
// ======================
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);

#define SERVOMIN 110
#define SERVOMAX 490

// ======================
// Direct Servo Setup
// ======================
Servo directServos[NUM_DIRECT_SERVOS];

// 🔥 YOUR PINS
int directPins[NUM_DIRECT_SERVOS] = {2, 3, 4, 5, 8, 9};

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(10);

  // Start PCA
  pca.begin();
  pca.setPWMFreq(50);

  // Attach direct servos
  for (int i = 0; i < NUM_DIRECT_SERVOS; i++) {
    directServos[i].attach(directPins[i]);
  }

  // Center all servos
  for (int i = 0; i < TOTAL_SERVOS; i++) {
    setServoAngle(i, 90);
  }
}

uint16_t angleToPulse(int angle) {
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

void setServoAngle(int id, int angle) {

  angle = constrain(angle, 0, 180);

  if (id < 16) {
    // PCA9685 servos
    uint16_t pulse = angleToPulse(angle);
    pca.setPWM(id, 0, pulse);
  }
  else if (id < 22) {
    // Direct Nano servos
    directServos[id - 16].write(angle);
  }
}

void loop() {

  if (Serial.available()) {

    String input = Serial.readStringUntil('\n');
    int comma = input.indexOf(',');

    if (comma > 0) {

      int id = input.substring(0, comma).toInt();
      int angle = input.substring(comma + 1).toInt();

      if (id >= 0 && id < TOTAL_SERVOS) {
        setServoAngle(id, angle);
      }
    }
  }
}
