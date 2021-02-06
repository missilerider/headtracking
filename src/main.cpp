#include <Arduino.h>
#include <Wire.h>
#include "Joystick.h"

const int MPU_addr = 0x68; // I2C address of the MPU-6050
const int HMC_addr = 0x1E; // I2C address of the HMC5883L

#define RESETCALIB_PIN  4

int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
int32_t tAX, tAY, tAZ, tGX, tGY, tGZ;
int16_t MX, MY, MZ, lMX, lMY, lMZ; // Mag XYZ, last Mag XYZ
int16_t mdX, mdY, mdZ; // Mag delta XYZ

signed long rX, rY, rZ;

int16_t xAxis, yAxis, zAxis, rxAxis, ryAxis, rzAxis;

signed long calibAccum = 0, calibMillis = -1000000, calibMin = 0, calibMax = 0, calibCalls = 0;

#define AXIS_MAX  32767
#define AXISR_MAX 359

// Calib
#include "constMk1.h"

Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  2, // JOYSTICK_DEFAULT_BUTTON_COUNT
  JOYSTICK_DEFAULT_HATSWITCH_COUNT,
  true, // xAxis
  true, // yAxis
  true, // zAxis
  false, // rxAxis
  false, // ryAxis
  false, // rzAxis
  false, // Rudder
  false, // Throttle
  false, // Accel
  false, // Brake
  false); // Steering

void printGy() {
    Serial1.print("Gy:\t");
    Serial1.print(tGX);
    Serial1.print("\t");
    Serial1.print(tGY);
    Serial1.print("\t");
    Serial1.println(tGZ);
}

void printMag() {
    Serial1.print("Mag:\t");
    Serial1.print(MX);
    Serial1.print("\t");
    Serial1.print(MY);
    Serial1.print("\t");
    Serial1.println(MZ);
}

void readMPU() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(true); // era false
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
  AcX = Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  //Tmp=Wire.read()<<8|Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  Wire.read(); Wire.read(); // Tmp > /dev/null
  GyX = Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

bool readQMC() {
  Wire.beginTransmission(HMC_addr);
  Wire.write(0x03); // Data Output X MSB Reg
  Wire.endTransmission(true);

  Wire.requestFrom(HMC_addr, 0x06, true);
  MX = Wire.read()<<8|Wire.read();
  MY = Wire.read()<<8|Wire.read();
  MZ = Wire.read()<<8|Wire.read();

  if((MX != lMX) || (MY != lMY) || (MZ != lMZ)) {
    lMX = MX;
    lMY = MY;
    lMZ = MZ;
    return true;
  }
  return false;
}

void calcRotation(signed long *pos, int16_t *axis, int32_t val, int32_t dz, signed long range) {
  if(val > dz || val < -dz) {
    *pos += val;

    signed long newAxis = *pos / (float)range * (AXIS_MAX / 2);

    if(newAxis > AXIS_MAX) {
      newAxis = AXIS_MAX;
      //printMag();
    } else {
      if(newAxis < -AXIS_MAX) {
        newAxis = -AXIS_MAX;
        //printMag();
      }
    }

    *axis = (int16_t)newAxis;
  }
}

bool resetButtonPressed() {
  return digitalRead(RESETCALIB_PIN) == LOW;
}

bool printCalibration(int32_t *measure) {
  calibAccum += *measure;
  calibCalls++;

  if(*measure > calibMax) calibMax = *measure;
  if(*measure < calibMin) calibMin = *measure;

  if(millis() - calibMillis < 10000) {
    return false;
  }

  Serial1.print("Polls: ");
  Serial1.println(calibCalls);
  Serial1.print("Range: [");
  Serial1.print((int)calibMin);
  Serial1.print(", ");
  Serial1.print((int)calibMax);
  Serial1.print("] / DZ: ");
  Serial1.println((int)(calibMax - calibMin));

  Serial1.print("Avg: ");
  Serial1.println((int)(calibAccum / (float)calibCalls));
  Serial1.print("Mid: ");
  Serial1.println((int)((calibMax - calibMin) / 2 + calibMin));

  calibMillis = millis();
  calibCalls = 0;

  calibMin = *measure;
  calibMax = *measure;

  return true;
}

void setup() {
  Serial1.begin(9600);
  Serial1.println("Serial up");

  pinMode(RESETCALIB_PIN, INPUT_PULLUP);

  Wire.begin();

  delay(100); // I2C startup
  Wire.setClock(400000L);

  Serial1.println("Clock OK");

  // Init MPU
  Wire.beginTransmission(MPU_addr);
  Serial1.println("tx ok");
  Wire.write(0x6B); // PWR_MGMT_1 register
  Serial1.println("addr ok");
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Serial1.println("cfg ok");
  Wire.endTransmission(true);

  Serial1.println("MPU6050 OK");
/*
  // Init HMC
  Wire.beginTransmission(HMC_addr);
  Wire.write(0x00); // Conf Reg A
  Wire.write(B00011100); // A: 
  Wire.write(B00000000); // B: 
  Wire.write(B00000000); // Mode

//  Wire.write(0x02); // Mode Register
  Wire.write(0x00); // Continuous
  Wire.endTransmission(true);
  */

  Serial1.println("QMC5883 OK");

  rX = 0;
  rY = 0;
  rZ = 0;

  MX = MY = MZ = lMX = lMY = lMZ = 0;
  mdX = mdY = mdZ = 0;

  xAxis = 0;
  yAxis = 0;
  zAxis = 0;

  Joystick.setXAxisRange(-AXIS_MAX, AXIS_MAX);
  Joystick.setYAxisRange(-AXIS_MAX, AXIS_MAX);
  Joystick.setZAxisRange(-AXIS_MAX, AXIS_MAX);

/*  Joystick.setRxAxisRange(-AXISR_MAX, AXISR_MAX);
  Joystick.setRyAxisRange(-AXISR_MAX, AXISR_MAX);
  Joystick.setRzAxisRange(-AXISR_MAX, AXISR_MAX);*/

  Joystick.begin(false); // autoSendMode = false
}

unsigned long lastT = 0;

void loop() {
  readMPU();

  tAX += AcX;
  tAY += AcY;
  tAZ += AcZ;
  tGX += GyX;
  tGY += GyY;
  tGZ += GyZ;

  if(millis() - lastT < 50) { // 50ms
    return;
  }

  lastT = millis();

  //printCalibration(&tGX);


  if(resetButtonPressed()) {

    printGy();
    printMag();
    Serial1.println();

    rX = 0;
    rY = 0;
    rZ = 0;

    mdX = MX;
    mdY = MY;
    mdZ = MZ;
  }

/*  if(readQMC()) {
    MX -= mdX;
    MY -= mdY;
    MZ -= mdZ;

    printMag();

    Joystick.setRxAxis(MZ);
    Joystick.setRyAxis(MX);
    Joystick.setRzAxis(MY);
  }*/

  calcRotation(&rX, &xAxis, tGX - GYX0, GYXDZ, GYXR);
  calcRotation(&rY, &yAxis, tGY - GYY0, GYYDZ, GYYR);
  calcRotation(&rZ, &zAxis, tGZ - GYZ0, GYZDZ, GYZR);

  tAX = tAY = tAZ = tGX = tGY = tGZ = 0; // Reset para siguiente bucle temporizado

  Joystick.setXAxis(-zAxis);
  Joystick.setYAxis(-yAxis);
  Joystick.setZAxis(-xAxis);

  Joystick.sendState();
}