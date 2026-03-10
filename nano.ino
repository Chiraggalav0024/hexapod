#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Servo.h>

#define TOTAL_SERVOS 22
#define NUM_DIRECT_SERVOS 6

#define SERVOMIN 110
#define SERVOMAX 490

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);
Servo directServos[NUM_DIRECT_SERVOS];

int directPins[NUM_DIRECT_SERVOS] = {2,3,4,5,8,9};
int currentAngle[TOTAL_SERVOS];


// ================= POSES =================

int standneutral[TOTAL_SERVOS] = {
90,96,90,118,96,90,119,80,90,114,94,90,
77,120,90,90,129,90,49,98,56,90
};

int halfstand[TOTAL_SERVOS] = {
124,79,110,74,125,107,63,131,101,63,135,90,
143,53,90,173,47,81,74,93,96,90
};

int fullstand[TOTAL_SERVOS] = {
20,158,110,169,39,98,160,40,102,167,51,90,
40,154,93,61,146,81,74,93,96,90
};

int fullsit[TOTAL_SERVOS] = {
156,95,63,41,106,90,50,96,155,53,93,131,
139,128,90,173,109,39,74,93,96,90
};


// ================= SERVO CORE =================

uint16_t angleToPulse(int angle){
  return map(angle,0,180,SERVOMIN,SERVOMAX);
}

void writeServo(int id,int angle){

  angle = constrain(angle,0,180);

  if(id < 16)
    pca.setPWM(id,0,angleToPulse(angle));
  else
    directServos[id-16].write(angle);
}


// ================= SMOOTH MOVE =================

void moveSim(int ids[], int targets[], int count){

  bool moving = true;

  while(moving){

    moving = false;

    for(int i=0;i<count;i++){

      int id = ids[i];

      if(currentAngle[id] < targets[i]){
        currentAngle[id]++;
        moving = true;
      }
      else if(currentAngle[id] > targets[i]){
        currentAngle[id]--;
        moving = true;
      }

      writeServo(id,currentAngle[id]);
    }

    delay(8);
  }
}


// ================= MOVE TO POSE =================

void moveToPose(int pose[]){

  int ids[TOTAL_SERVOS];
  int targets[TOTAL_SERVOS];

  for(int i=0;i<TOTAL_SERVOS;i++){
    ids[i] = i;
    targets[i] = pose[i];
  }

  moveSim(ids,targets,TOTAL_SERVOS);
}


// =================================================
// WALK ENGINE
// =================================================

void walkCycle(int pose[], bool backward, bool finalCycle){

  int dir = backward ? -1 : 1;

  int s1[]={1,7,13};
  int t1[]={pose[1]-40,pose[7]+40,pose[13]-40};
  moveSim(s1,t1,3);

  int s2[]={2,8,14};
  int t2[]={pose[2]-30*dir,pose[8]-30*dir,pose[14]+30*dir};
  moveSim(s2,t2,3);

  int s3[]={1,7,13};
  int t3[]={pose[1],pose[7],pose[13]};
  moveSim(s3,t3,3);

  int s4[]={2,8,14,4,10,16};
  int t4[]={pose[2],pose[8],pose[14],
            pose[4]+40,pose[10]+40,pose[16]-40};
  moveSim(s4,t4,6);

  int s5[]={5,11,17};
  int t5[]={pose[5]-30*dir,pose[11]+30*dir,pose[17]+30*dir};
  moveSim(s5,t5,3);

  int s6[]={4,10,16};
  int t6[]={pose[4],pose[10],pose[16]};
  moveSim(s6,t6,3);

  if(!finalCycle){

    int s7[]={5,11,17,1,7,13};
    int t7[]={pose[5],pose[11],pose[17],
              pose[1]-40,pose[7]+40,pose[13]-40};

    moveSim(s7,t7,6);
  }
  else{

    int s7[]={5,11,17};
    int t7[]={pose[5],pose[11],pose[17]};

    moveSim(s7,t7,3);
  }

  delay(200);
}


void forwardWalk(int pose[]){

  moveToPose(pose);

  walkCycle(pose,false,false);
  walkCycle(pose,false,false);
  walkCycle(pose,false,true);

  moveToPose(pose);
}

void backwardWalk(int pose[]){

  moveToPose(pose);

  walkCycle(pose,true,false);
  walkCycle(pose,true,false);
  walkCycle(pose,true,true);

  moveToPose(pose);
}


// =================================================
// ROTATION
// =================================================

void superPhaseA(int pose[], int dir){

  int s1[]={1,7,13};
  int t1[]={pose[1]-40,pose[7]+40,pose[13]-40};
  moveSim(s1,t1,3);

  int s2[]={2,8,14};
  int t2[]={pose[2]-30*dir,pose[8]-30*dir,pose[14]-30*dir};
  moveSim(s2,t2,3);

  int s3[]={1,7,13};
  int t3[]={pose[1],pose[7],pose[13]};
  moveSim(s3,t3,3);

  int s4[]={2,8,14,4,10,16};
  int t4[]={pose[2],pose[8],pose[14],
            pose[4]+40,pose[10]+40,pose[16]-40};

  moveSim(s4,t4,6);
}

void superPhaseB(int pose[], int dir){

  int s5[]={5,11,17};
  int t5[]={pose[5]+30*dir,pose[11]+30*dir,pose[17]+30*dir};
  moveSim(s5,t5,3);

  int s6[]={4,10,16};
  int t6[]={pose[4],pose[10],pose[16]};
  moveSim(s6,t6,3);

  int s7[]={5,11,17,1,7,13};
  int t7[]={pose[5],pose[11],pose[17],
            pose[1]-40,pose[7]+40,pose[13]-40};
  moveSim(s7,t7,6);
}

void doRotation(int pose[], int dir){

  moveToPose(pose);

  for(int i=0;i<3;i++){

    superPhaseA(pose,dir);
    superPhaseB(pose,-dir);

  }

  moveToPose(pose);
}


// =================================================
// SMOVE
// =================================================

void doSMove(int pose[]){

  moveToPose(pose);

  for(int cycle=0;cycle<3;cycle++){

    superPhaseA(pose,1);
    superPhaseB(pose,1);

  }

  moveToPose(pose);
}


// =================================================
// HI GESTURE
// =================================================

void hiMotion(){

  moveToPose(standneutral);

  for(int a=currentAngle[1];a>=30;a--){
    writeServo(1,a);
    delay(15);
  }

  for(int i=0;i<2;i++){

    for(int a=60;a<=120;a++){
      writeServo(0,a);
      delay(10);
    }

    for(int a=120;a>=60;a--){
      writeServo(0,a);
      delay(10);
    }
  }

  moveToPose(standneutral);
}


// =================================================
// SETUP
// =================================================

void setup(){

  Serial.begin(115200);

  pca.begin();
  pca.setPWMFreq(50);

  for(int i=0;i<NUM_DIRECT_SERVOS;i++)
    directServos[i].attach(directPins[i]);

  for(int i=0;i<TOTAL_SERVOS;i++){
    currentAngle[i]=90;
    writeServo(i,90);
  }

  delay(1000);

  moveToPose(standneutral);

  Serial.println("Hexapod Unified Firmware Ready");
}


// =================================================
// LOOP
// =================================================

void loop(){

  if(Serial.available()){

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if(cmd=="standneutral") moveToPose(standneutral);
    if(cmd=="halfstand") moveToPose(halfstand);
    if(cmd=="fullstand") moveToPose(fullstand);
    if(cmd=="fullsit") moveToPose(fullsit);

    if(cmd=="fmove") forwardWalk(standneutral);
    if(cmd=="bmove") backwardWalk(standneutral);

    if(cmd=="fmove0") forwardWalk(halfstand);
    if(cmd=="bmove0") backwardWalk(halfstand);

    if(cmd=="fmove1") forwardWalk(fullstand);
    if(cmd=="bmove1") backwardWalk(fullstand);

    if(cmd=="rmove") doRotation(standneutral,1);
    if(cmd=="lmove") doRotation(standneutral,-1);

    if(cmd=="rmove0") doRotation(halfstand,1);
    if(cmd=="lmove0") doRotation(halfstand,-1);

    if(cmd=="rmove1") doRotation(fullstand,1);
    if(cmd=="lmove1") doRotation(fullstand,-1);

    if(cmd=="smove") doSMove(standneutral);
    if(cmd=="smove0") doSMove(halfstand);
    if(cmd=="smove1") doSMove(fullstand);

    if(cmd=="hi") hiMotion();
  }
}
