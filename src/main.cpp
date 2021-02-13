#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <EEPROM.h>

#define PIN_BUTTON 10

double xPos = 0, yPos = 0, headingVel = 0;
uint16_t BNO055_SAMPLERATE_DELAY_MS = 10; //how often to read data from the board
uint16_t PRINT_DELAY_MS = 500; // how often to print the data
uint16_t printCount = 0; //counter to avoid printing every 10MS sample

//velocity = accel*dt (dt in seconds)
//position = 0.5*accel*dt^2
double ACCEL_VEL_TRANSITION =  (double)(BNO055_SAMPLERATE_DELAY_MS) / 1000.0;
double ACCEL_POS_TRANSITION = 0.5 * ACCEL_VEL_TRANSITION * ACCEL_VEL_TRANSITION;
double DEG_2_RAD = 0.01745329251; //trig functions require radians, BNO055 outputs degrees

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);

uint8_t calibData[22], lastCalibData[22];
imu::Quaternion q, qDelta, qTmp;
imu::Vector<3> refY, refZ;
imu::Vector<3> headY, headZ;

int n;

void calcHeadPos() {
  qTmp = q * qDelta;

  headY = qTmp.rotateVector(refY);
  headZ = qTmp.rotateVector(refZ);
}

void setDelta() {
  qDelta = q.conjugate();
}

void saveCalibData() {
  for (n = 0; n < 22; n++)
    EEPROM.write(n, calibData[n]);

  EEPROM.write(23, 123);
}

void saveDouble(int addr, double data) {
  unsigned char* p = (unsigned char*)&data;
  EEPROM.write(addr++, *p++);
  EEPROM.write(addr++, *p++);
  EEPROM.write(addr++, *p++);
  EEPROM.write(addr++, *p++);
  EEPROM.write(addr++, *p++);
  EEPROM.write(addr++, *p++);
  EEPROM.write(addr++, *p++);
  EEPROM.write(addr++, *p++);
}

double loadDouble(int addr) {
  double ret;
  unsigned char *p = (unsigned char*)&ret;
  *p++ = EEPROM.read(addr++);
  *p++ = EEPROM.read(addr++);
  *p++ = EEPROM.read(addr++);
  *p++ = EEPROM.read(addr++);
  *p++ = EEPROM.read(addr++);
  *p++ = EEPROM.read(addr++);
  *p++ = EEPROM.read(addr++);
  *p++ = EEPROM.read(addr++);

  return ret;
}

void saveDeltaData() {
  EEPROM.write(24, 123);

  saveDouble(25, qDelta.w());
  saveDouble(25+8, qDelta.x());
  saveDouble(25+16, qDelta.y());
  saveDouble(25+24, qDelta.z());
}

void loadCalibData() {
  if(EEPROM.read(23) != 123) {
    Serial1.println("READ CHUNGO!!!!!!!!!!!!!!!!!");
    return;
  }

  Serial1.println("CALIB DATA OK");

  for (n = 0; n < 22; n++) {
    calibData[n] = EEPROM.read(n);
    lastCalibData[n] = calibData[n];
  }
}

void loadDeltaData() {
  if(EEPROM.read(24) != 123) {
    Serial1.println("READ DELTA CHUNGO!!!!!!!!!!!!!!!!!");
    return;
  }

  Serial1.println("DELTA DATA OK");

  qDelta = imu::Quaternion(
    loadDouble(25), 
    loadDouble(25+8), 
    loadDouble(25+16), 
    loadDouble(25+24)
    );
}


void setup(void)
{
  Serial1.begin(115200);
  if (!bno.begin())
  {
    Serial1.print("No BNO055 detected");
    while (1);
  }

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  bno.setMode(Adafruit_BNO055::OPERATION_MODE_NDOF);

  memset(calibData, 0, 22);
  memset(lastCalibData, 0, 22);

  loadCalibData();

  bno.setSensorOffsets(calibData);

  // Cabeza en cualquier sitio
  refY = imu::Vector<3>(0.0, 512.0, 0.0);
  refZ = imu::Vector<3>(0.0, 0.0, 512.0);

  qDelta = imu::Quaternion();

  loadDeltaData();

  delay(1000);
}

void loop(void)
{
  //
  unsigned long tStart = micros();
  
  uint8_t sys, gyro, accel, mag = 0;
  bno.getCalibration(&sys, &gyro, &accel, &mag);

  q = bno.getQuat();

  if(!digitalRead(PIN_BUTTON)) {
    setDelta();
    Serial1.print("*");
    saveDeltaData();
  }

  if (printCount * BNO055_SAMPLERATE_DELAY_MS >= PRINT_DELAY_MS) {
    //setDelta();

    calcHeadPos();
    
    Serial1.print("X: ");
    Serial1.print(headZ.x(), DEC);
    Serial1.print("\tY: ");
    Serial1.print(headY.x(), DEC);
    Serial1.print("\tZ: ");
    Serial1.print(headZ.y(), DEC);
    Serial1.println();

    Serial1.print("Cal:\tS: ");
    Serial1.print(sys, DEC);
    Serial1.print("\tG: ");
    Serial1.print(gyro, DEC);
    Serial1.print("\tA: ");
    Serial1.print(accel, DEC);
    Serial1.print("\tM: ");
    Serial1.print(mag, DEC);

    if(sys + gyro + accel + mag == 3 * 4) {
        Serial1.print("\t[CALIB]");

        bno.getSensorOffsets(calibData);

        if(memcmp(lastCalibData, calibData, 22) != 0) {

          memcpy(lastCalibData, calibData, 22);

          saveCalibData();

          Serial1.print("\t[SAVE]");
        }

    }

    Serial1.println();
    Serial1.println("-------");

    printCount = 0;
  }
  else {
    printCount = printCount + 1;
  }

  while ((micros() - tStart) < (BNO055_SAMPLERATE_DELAY_MS * 1000))
  {
    //poll until the next sample is ready
  }
}
