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

void removeSpaces(char* s) {
  *std::remove(s, s + strlen(s), ' ') = 0;
}

void doingSomething1()
{
  char buf[33];
  File dataFile = SD.open("test.txt", FILE_WRITE);

  if (mode == 0 && dataFile ) {
    char sz[32];
    sprintf(sz, "%04d/%02d/%02d ", gps.date.year(), gps.date.month(), gps.date.day());
    Serial.println("date:");
    Serial.println(sz);
    //    dataFile.print(sz);
    //    dataFile.print(", ");





    Serial.println("time");
    sprintf(sz, "%02d:%02d:%02d ", gps.time.hour(), gps.time.minute(), gps.time.second());
    Serial.println(sz);
    Serial.println("");
    //    dataFile.print(sz);
    //    dataFile.print(", ");




    Serial.println("Latitude:");
    Serial.println(GPS_AR[0], 6);
    //    dataFile.print(GPS_AR[0], 6);
    //    dataFile.print(", ");
    dtostrf(GPS_AR[0], -6, 3, buf);
    //    ble.write("B\r\n");
    //    strcat(buf,"A");
    //    ble.write(buf);
    //    ble.write("\r\n");

    strcpy(packet, "A");
    strcat(packet, buf);

    Serial.println("Longitude:");
    Serial.println(GPS_AR[1], 6);
    //    dataFile.print(GPS_AR[1], 6);
    //    dataFile.print(", ");
    dtostrf(GPS_AR[1], -6, 3, buf);
    //strcat(buf,"O");
    //    ble.write(buf);
    //    ble.write("\r\n");

    strcat(packet, "O");
    strcat(packet, buf);

    Serial.println("Altitude Feet:");
    Serial.println(GPS_AR[2]);
    //    dataFile.print(GPS_AR[2], 6);
    //    dataFile.print(", ");
    dtostrf(GPS_AR[2], -5, 3, buf);
    //strcat(buf,"H");
    //    ble.write(buf);
    //    ble.write("\r\n");

    strcat(packet, "H");
    strcat(packet, buf);

    Serial.println("Speed:");
    Serial.println(GPS_AR[3]);
    //    dataFile.print(GPS_AR[3], 6);
    //    dataFile.print(", ");
    dtostrf(GPS_AR[3], -2, 1, buf);
    //strcat(buf,"S");
    //    ble.write(buf);
    //    ble.write("\r\n");

    strcat(packet, "S");
    strcat(packet, buf);

    Serial.println("temperature:");
    Serial.println(BME_AR[0]);
    //    dataFile.print(BME_AR[0], 6);
    //    dataFile.print(", ");
    dtostrf(BME_AR[0], -3, 1, buf);
    //    strcat(buf,"T");
    //    ble.write(buf);
    //    ble.write("\r\n");

    strcat(packet, "T");
    strcat(packet, buf);

    Serial.println("pressure:");
    Serial.println(BME_AR[1]);
    //    dataFile.print(BME_AR[1], 6);
    //    dataFile.print(", ");
    dtostrf(BME_AR[1], -6, 3, buf);
    //    strcat(buf,"P");
    //    ble.write(buf);
    //    ble.write("\r\n");

    strcat(packet, "P");
    strcat(packet, buf);

    Serial.println("humidity:");
    Serial.println(BME_AR[2]);
    //    dataFile.print(BME_AR[2], 6);
    //    dataFile.println("");
    dtostrf(BME_AR[2], -4, 3, buf);
    //    strcat(buf,"W");
    //    ble.write(buf);
    //    ble.write("\r\n");

    strcat(packet, "W");
    strcat(packet, buf);

    Serial.println("X G:");
    Serial.println(IMU_AR[0]);
    //    dataFile.print(IMU_AR[0], 6);
    //    dataFile.print(", ");
    dtostrf(IMU_AR[0], 10, 10, buf);
    //    strcat(buf,"X");
    //    ble.write(buf);
    //    ble.write("\r\n");

    strcat(packet, "X");
    strcat(packet, buf);

    Serial.println("Y G:");
    Serial.println(IMU_AR[1]);
    //    dataFile.print(IMU_AR[1], 6);
    //    dataFile.print(", ");
    dtostrf(IMU_AR[1], 10, 10, buf);
    //    strcat(buf,"Y");
    //    ble.write(buf);
    //    ble.write("\r\n");

    strcat(packet, "Y");
    strcat(packet, buf);

    Serial.println("Z G:");
    Serial.println(IMU_AR[2]);
    //    dataFile.print(IMU_AR[2], 6);
    //    dataFile.print(", ");
    dtostrf(IMU_AR[2], 10, 10, buf);
    //    strcat(buf,"Z");
    //    ble.write(buf);
    //    ble.write("\r\n");
    strcat(packet, "Z");
    strcat(packet, buf);

    float HardRate = random(1, 5) + 60;
    Serial.println("HR: ");
    Serial.println(HardRate);
    dtostrf(HardRate, -2, 1, buf);
    strcat(packet, "R");
    strcat(packet, buf);



    removeSpaces(packet);
    int lin = sizeof(packet);
    BigNumber dataNum = inp2BigInt(packet, lin);
    Serial.println(dataNum);
    Serial.println(lin);





    BigNumber encrypted = RSAencrypt(dataNum, pk1, mod);
    String outputStr = encrypted.toString();

    const char *outP = outputStr.c_str();

    ble.write("B");
    ble.write(outP);
    Serial.println(packet);
    Serial.println(outP);
    ble.write("E");

    dataFile.print(outP);
    dataFile.print("\r\n");





    flag = 0;
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

double dataVerify(double newData, double oldData) {
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


  // init bno0
  if (!bno08x.begin_I2C()) {
    Serial.println("Failed to find BNO08x chip");
    while (1) {
      delay(10);
    }
  }
  setReports();
  Serial.println("Reading events");
  Serial.println("BNO08x online.");



  BigNumber::begin();
  BigNumber mod = BigNumber(pk2); //converts number in string form to BigNumber data type
  Serial.println("Crypto Start");



  ble.begin(9600);//start ble
  Serial.println("BLE start");//verify

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
      //      GPS_AR[0] = (dataVerify(gps.location.lat(), GPS_AR[0]) + GPS_AR[0]) / 2;
      //      GPS_AR[1] = (dataVerify(gps.location.lng(), GPS_AR[1]) + GPS_AR[1]) / 2;
      //      GPS_AR[2] = (dataVerify(gps.altitude.feet(), GPS_AR[2]) + GPS_AR[2]) / 2;
      //      GPS_AR[3] = (dataVerify(gps.speed.mph(), GPS_AR[3]) + GPS_AR[3]) / 2;
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
    //flag = 1;
  }
  else {
    //    BME_AR[0] = (dataVerify(bme.temperature, BME_AR[0]) + BME_AR[0]) / 2;
    //    BME_AR[1] = (dataVerify((bme.pressure / 100.0), BME_AR[1]) + BME_AR[1]) / 2;
    //    BME_AR[2] = (dataVerify(bme.humidity, BME_AR[2]) + BME_AR[2]) / 2;
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
      if (flag == 0) {
        int tempx = sensorValue.un.accelerometer.x;
        int tempy = sensorValue.un.accelerometer.y;
        int tempz = sensorValue.un.accelerometer.z;
        if (abs(tempx) < .01) {
          IMU_AR[0] = 0;
        }
        else{
          IMU_AR[0] = sensorValue.un.accelerometer.x / 9.8;
        }
        if (abs(tempy) < .01) {
          IMU_AR[1] = 0;
        }
        else{
          IMU_AR[1] = sensorValue.un.accelerometer.y / 9.8;
        }
        if (abs(tempz) < .01) {
          IMU_AR[2] = 0;
        }
        else{
          IMU_AR[2] = sensorValue.un.accelerometer.z / 9.8;
        }
        //flag = 1;
      }
      else {
        //                IMU_AR[0] = (dataVerify(sensorValue.un.accelerometer.x / 9.8, IMU_AR[0]) + IMU_AR[0]) / 2;
        //                IMU_AR[1] = (dataVerify(sensorValue.un.accelerometer.y / 9.8, IMU_AR[0]) + IMU_AR[1]) / 2;
        //                IMU_AR[2] = (dataVerify(sensorValue.un.accelerometer.z / 9.8, IMU_AR[0]) + IMU_AR[2]) / 2;

      }
  }





}
