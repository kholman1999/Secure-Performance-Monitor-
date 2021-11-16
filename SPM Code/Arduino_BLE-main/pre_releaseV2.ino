//TIMER INIT START

// These define's must be placed at the beginning before #include "STM32TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
// Don't define TIMER_INTERRUPT_DEBUG > 2. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

#include "STM32TimerInterrupt.h"
#include "STM32TimerInterrupt.h"
#include "STM32_ISR_Timer.h"
#include <BigNumber.h>

#define TIMER_INTERVAL_MS         100
#define HW_TIMER_INTERVAL_MS      50

// Init STM32 timer TIM1
STM32Timer ITimer(TIM1);

// Init STM32_ISR_Timer
// Each STM32_ISR_Timer can service 16 different ISR-based timers
STM32_ISR_Timer ISR_Timer;

//interval lengths
#define TIMER_INTERVAL_10S             2000L
#define TIMER_INTERVAL_5S            5000L

void TimerHandler()
{
  ISR_Timer.run();
}

//TIMER INIT DONE

//////general includes//////
#include <SPI.h>
#include <SD.h>
#include "TinyGPS++.h"

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <Adafruit_BNO08x.h>



//////defines//////
#define SEALEVELPRESSURE_HPA (1013.25)
#define BNO08X_RESET -1
//////consts//////
const int chipSelect = 8; //FOR SD CARD

//////global variables//////
int flag = 0;
int initbuffer = 0;
int mode = 0;
char packet[67];
char buf[12];
float GPS_AR[4] = {};
float BME_AR[3] = {};
float IMU_AR[3] = {};

int  pk1 = 129; // public key one (exponent) can be adjusted

char pk2[] = "10283216039871810935867070308763590033267924706279480036805479708407577958672010439491502023522562278580887361154108790868660131671345775095268853731990497";
BigNumber mod = BigNumber(pk2); //converts number in string form to BigNumber data type


Adafruit_BME680 bme; // I2C
Adafruit_BNO08x  bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;
HardwareSerial ble(D52, D53); // RX, TX
HardwareSerial serial_connection(D0, D1); //RX=pin 0, TX=pin 1
TinyGPSPlus gps;

//////FUNCTIONS//////
void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  }
}
void(* resetFunc) (void) = 0;
void removeSpaces(char* s) {
  *std::remove(s, s + strlen(s), ' ') = 0;
}

void doingSomething1()
{
  char buf[33];
  File dataFile = SD.open("test.txt", FILE_WRITE);

  if (mode == 0 && dataFile ) {
    //print date//
    char sz[32];
    sprintf(sz, "%04d/%02d/%02d ", gps.date.year(), gps.date.month(), gps.date.day());
    Serial.println("date:");
    Serial.println(sz);

    //print time//
    Serial.println("time");
    sprintf(sz, "%02d:%02d:%02d ", gps.time.hour(), gps.time.minute(), gps.time.second());
    Serial.println(sz);
    Serial.println("");

    //print lat//
    Serial.println("Latitude:");
    Serial.println(GPS_AR[0], 6);
    dtostrf(GPS_AR[0], -6, 3, buf);

    //append lat to pre encyypt string//
    strcpy(packet, "A");
    strcat(packet, buf);

    //print long//
    Serial.println("Longitude:");
    Serial.println(GPS_AR[1], 6);
    dtostrf(GPS_AR[1], -6, 3, buf);

    //print long to pre encrypt string//
    strcat(packet, "O");
    strcat(packet, buf);

    //print altitude//
    Serial.println("Altitude Feet:");
    Serial.println(GPS_AR[2]);
    dtostrf(GPS_AR[2], -5, 3, buf);

    //append altitude to pre encrypt string//
    //do you see the pattern?//
    strcat(packet, "H");
    strcat(packet, buf);

    //format speed//
    Serial.println("Speed:");
    Serial.println(GPS_AR[3]);
    dtostrf(GPS_AR[3], -2, 1, buf);

    strcat(packet, "S");
    strcat(packet, buf);

    //format temp//
    Serial.println("temperature:");
    Serial.println(BME_AR[0]);
    dtostrf(BME_AR[0], -3, 1, buf);

    strcat(packet, "T");
    strcat(packet, buf);

    //format pressure//
    Serial.println("pressure:");
    Serial.println(BME_AR[1]);
    dtostrf(BME_AR[1], -6, 3, buf);

    strcat(packet, "P");
    strcat(packet, buf);

    //format humidity//
    Serial.println("humidity:");
    Serial.println(BME_AR[2]);
    dtostrf(BME_AR[2], -4, 3, buf);

    strcat(packet, "W");
    strcat(packet, buf);

    //format x gforce//
    Serial.println("X G:");
    Serial.println(IMU_AR[0]);
    dtostrf(IMU_AR[0], 10, 10, buf);

    strcat(packet, "X");
    strcat(packet, buf);

    //format y gforce//
    Serial.println("Y G:");
    Serial.println(IMU_AR[1]);
    dtostrf(IMU_AR[1], 10, 10, buf);

    strcat(packet, "Y");
    strcat(packet, buf);

    //format z gforce//
    Serial.println("Z G:");
    Serial.println(IMU_AR[2]);

    dtostrf(IMU_AR[2], 10, 10, buf);
    strcat(packet, "Z");
    strcat(packet, buf);

    //heart rate//
    float HardRate = random(1, 5) + 60;
    Serial.println("HR: ");
    Serial.println(HardRate);
    dtostrf(HardRate, -2, 1, buf);
    strcat(packet, "R");
    strcat(packet, buf);


    //prep pre encrypt string//
    removeSpaces(packet);
    int lin = sizeof(packet);
    BigNumber dataNum = inp2BigInt(packet, lin);
    Serial.println(dataNum);
    Serial.println(lin);

    //encrypt pre encrypt string//
    BigNumber encrypted = RSAencrypt(dataNum, pk1, mod);
    String outputStr = encrypted.toString();
    //convert to char array
    const char *outP = outputStr.c_str();

    //push encrypted string
    ble.write("B");
    ble.write(outP);
    Serial.println(packet);
    Serial.println(outP);
    ble.write("E");

    //save encrypted string to SD card
    dataFile.print(outP);
    dataFile.print("\r\n");

    //flag for data averaging
    flag = 0;

    //close the file
    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
  }

}

BigNumber inp2BigInt(char * input, int len) // converts line of text to number
{
  char table[49] = "==========1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ. ";
  int i = 0;
  int j;
  BigNumber num = "0";
  BigNumber hund = "100";
  char com;
  for (i = (len - 2); i >= 0; i--) {
    j = 10;
    while (table[j] != input[i]) {
      j++;
    }
    num += BigNumber(j);
    num *= hund;
  }
  num /= 100;
  return num;
}





BigNumber RSAencrypt(BigNumber dataNum, int power, BigNumber mod)
{
  BigNumber prod = BigNumber(1);


  for (int i = 1; i <= power; i++)
  {
    prod *= dataNum;
    prod %= mod;
    //Serial.println(prod);

  }
  //Serial.println(prod);
  delay(100);
  return prod;
}


//void doingSomething2()
//{
//  char buf[33];
//  File dataFile = SD.open("cool.txt", FILE_WRITE);
//  if (mode == 1 && dataFile ) {
//    Serial.println("Latitude:");
//    //Serial.println(GPS_AR[0], 6);
//    dataFile.print(GPS_AR[0], 6);
//    dataFile.print(", ");
//    //dtostrf(GPS_AR[0], 8, 3, buf);
//    //ble.write(buf);
//    //ble.write("\r\n");
//
//    Serial.println("Longitude:");
//    //Serial.println(GPS_AR[1], 6);
//    dataFile.print(GPS_AR[1], 6);
//    dataFile.print(", ");
//    //dtostrf(GPS_AR[1], 8, 3, buf);
//    //ble.write(buf);
//    //ble.write("\r\n");
//
//    Serial.println("Altitude Feet:");
//    //Serial.println(GPS_AR[2]);
//    dataFile.print(GPS_AR[2], 6);
//    dataFile.print(", ");
//    //dtostrf(GPS_AR[2], 8, 3, buf);
//    //ble.write(buf);
//    //ble.write("\r\n");
//
//    Serial.println("temperature:");
//    //Serial.println(BME_AR[0]);
//    dataFile.print(BME_AR[0], 6);
//    dataFile.print(", ");
//
//    Serial.println("pressure:");
//    //Serial.println(BME_AR[1]);
//    dataFile.print(BME_AR[1], 6);
//    dataFile.print(", ");
//
//    Serial.println("humidity:");
//    Serial.println(BME_AR[2]);
//    dataFile.print(BME_AR[2], 6);
//    dataFile.println("");
//
//
//
//
//
//    flag = 0;
//    dataFile.close();
//  }
//
//}

double dataVerify(float newData, float oldData) {
  double dif = ((newData - oldData) / (oldData)) * 100;
  if (abs(dif) > 15) {
    return oldData;
  }
  else {
    return newData;
  }
}






void setup()
{
  //serial and GPS
  Serial.begin(9600);
  serial_connection.begin(9600);//This opens up communications to the GPS
  Serial.println("GPS online.");

  // Set interval in microsecs
  if (ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS * 1000, TimerHandler))
  {
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));

  //init SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("SD card online.");


  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);

  }
  Serial.println("BME online.");

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms






  BigNumber::begin();
  BigNumber mod = BigNumber(pk2); //converts number in string form to BigNumber data type
  Serial.println("Crypto Start");



  ble.begin(9600);//start ble
  Serial.println("BLE start");//verify


  // init bno0
  if (!bno08x.begin_I2C()) {
    Serial.println("Failed to find BNO08x chip");


    while (1) {
      delay(1000);
      //ble.write("RESET");
    }
  }
  setReports();
  Serial.println("Reading events");
  Serial.println("BNO08x online.");


  Serial.println("all systems nominal.");

  //allow gps to "warm up"
  delay(5000);

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_Timer
  ISR_Timer.setInterval(TIMER_INTERVAL_10S,    doingSomething1);
  //ISR_Timer.setInterval(TIMER_INTERVAL_2r5S,    doingSomething2);


}


void loop()
{


  while (serial_connection.available()) //wait for data
  {
    gps.encode(serial_connection.read());//feed the serial NMEA data
  }
  if (gps.location.isUpdated()) //full package only
  {
    if (flag == 0) {
      GPS_AR[0] = gps.location.lat();
      GPS_AR[1] = gps.location.lng();
      GPS_AR[2] = gps.altitude.feet();
      GPS_AR[3] = gps.speed.mph();
    }
    else {
      GPS_AR[0] = (dataVerify(gps.location.lat(), GPS_AR[0]) + GPS_AR[0]) / 2;
      GPS_AR[1] = (dataVerify(gps.location.lng(), GPS_AR[1]) + GPS_AR[1]) / 2;
      GPS_AR[2] = (dataVerify(gps.altitude.feet(), GPS_AR[2]) + GPS_AR[2]) / 2;
      GPS_AR[3] = (dataVerify(gps.speed.mph(), GPS_AR[3]) + GPS_AR[3]) / 2;
    }
  }

  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  if (flag == 0) {
    BME_AR[0] = bme.temperature;
    BME_AR[1] = bme.pressure / 100.0;
    BME_AR[2] = bme.humidity;
  }
  else {
    BME_AR[0] = (dataVerify(bme.temperature, BME_AR[0]) + BME_AR[0]) / 2;
    BME_AR[1] = (dataVerify((bme.pressure / 100.0), BME_AR[1]) + BME_AR[1]) / 2;
    BME_AR[2] = (dataVerify(bme.humidity, BME_AR[2]) + BME_AR[2]) / 2;
  }

  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  switch (sensorValue.sensorId) {
    case SH2_ACCELEROMETER:
      int tempx = sensorValue.un.accelerometer.x;
      int tempy = sensorValue.un.accelerometer.y;
      int tempz = sensorValue.un.accelerometer.z;
      if (flag == 0) {
        if (abs(tempx) < .01) {
          IMU_AR[0] = 0;
        }
        else {
          IMU_AR[0] = sensorValue.un.accelerometer.x / 9.8;
        }
        if (abs(tempy) < .01) {
          IMU_AR[1] = 0;
        }
        else {
          IMU_AR[1] = sensorValue.un.accelerometer.y / 9.8;
        }
        if (abs(tempz) < .01) {
          IMU_AR[2] = 0;
        }
        else {
          IMU_AR[2] = sensorValue.un.accelerometer.z / 9.8;
        }
      }

      else {
        if (abs(tempx) < .01) {
          IMU_AR[0] = 0;
        }
        else {
          IMU_AR[0] = tempx/9.8;
        }
        if (abs(tempy) < .01) {
          IMU_AR[1] = 0;
        }
        else {
          IMU_AR[1] = tempy/9.8;
        }
        if (abs(tempz) < .01) {
          IMU_AR[2] = 0;
        }
        else {
          IMU_AR[2] = tempz/9.8;
        }
        //        if (abs(tempx) < .01) {
        //          if (dataVerify(0, IMU_AR[0]) == 0) {
        //            IMU_AR[0] = 0;
        //          }
        //          else {
        //            IMU_AR[0] = (IMU_AR[0] + dataVerify(0, IMU_AR[0])) / 2;
        //          }
        //        }
        //        else {
        //          IMU_AR[0] = (IMU_AR[0] + dataVerify(tempx, IMU_AR[0])) / 2;
        //        }
        //        if (abs(tempy) < .01) {
        //          if (dataVerify(0, IMU_AR[1]) == 0) {
        //            IMU_AR[1] = 0;
        //          }
        //          else {
        //            IMU_AR[1] = (IMU_AR[1] + dataVerify(0, IMU_AR[1])) / 2;
        //          }
        //        }
        //        else {
        //          IMU_AR[1] = (IMU_AR[1] + dataVerify(tempy, IMU_AR[1])) / 2;
        //        }
        //        if (abs(tempz) < .01) {
        //          if (dataVerify(0, IMU_AR[2]) == 0) {
        //            IMU_AR[2] = 0;
        //          }
        //          else {
        //            IMU_AR[2] = (IMU_AR[2] + dataVerify(0, IMU_AR[2])) / 2;
        //          }
        //        }
        //        else {
        //          IMU_AR[2] = (IMU_AR[2] + dataVerify(tempz, IMU_AR[2])) / 2;
        //        }
      }
  }


  flag = 1;


}
