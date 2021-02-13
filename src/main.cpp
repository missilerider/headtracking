#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

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

uint8_t calibData;

void setup(void)
{
  Serial1.begin(115200);
  if (!bno.begin())
  {
    Serial1.print("No BNO055 detected");
    while (1);
  }

  bno.setMode(Adafruit_BNO055::OPERATION_MODE_NDOF);

  delay(1000);
}

void loop(void)
{
  //
  unsigned long tStart = micros();
  //sensors_event_t orientationData , linearAccelData;
  //bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  //  bno.getEvent(&angVelData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  //bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
  
  uint8_t sys, gyro, accel, mag = 0;
  bno.getCalibration(&sys, &gyro, &accel, &mag);

  imu::Quaternion q = bno.getQuat();
  

  if (printCount * BNO055_SAMPLERATE_DELAY_MS >= PRINT_DELAY_MS) {
    //enough iterations have passed that we can print the latest data
    Serial1.print("Q: ");
    Serial1.print(q.x());
    Serial1.print(", ");
    Serial1.print(q.y());
    Serial1.print(", ");
    Serial1.print(q.z());
    Serial1.print(", ");
    Serial1.println(q.w());
    Serial1.print("Cal:\t");
    Serial1.print(sys, DEC);
    Serial1.print("\t");
    Serial1.print(gyro, DEC);
    Serial1.print("\t");
    Serial1.print(accel, DEC);
    Serial1.print("\t");
    Serial1.print(mag, DEC);

    if(sys + gyro + accel + mag == 3 * 4) {
        Serial1.print("\tCALIB OK!");

        bno.getSensorOffsets(&calibData);



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
