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

#define TIMER_INTERVAL_MS         100
#define HW_TIMER_INTERVAL_MS      50

// Init STM32 timer TIM1
STM32Timer ITimer(TIM1);

// Init STM32_ISR_Timer
// Each STM32_ISR_Timer can service 16 different ISR-based timers
STM32_ISR_Timer ISR_Timer;

//interval lengths
#define TIMER_INTERVAL_2r5S             2500L
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
const int chipSelect = 8;

//////global variables//////
int flag = 0;
int mode = 0;
char buf[33];
float GPS_AR[4] = {};
float BME_AR[3] = {};
float IMU_AR[3] = {};


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

void doingSomething1()
{
  char buf[33];
  File dataFile = SD.open("cool.txt", FILE_WRITE);
  if (mode == 0 && dataFile ) {
    char sz[32];
    sprintf(sz, "%04d/%02d/%02d ", gps.date.year(), gps.date.month(), gps.date.day());
    Serial.println("date:");
    Serial.println(sz);
    dataFile.print(sz);
    dataFile.print(", ");





    Serial.println("time");
    sprintf(sz, "%02d:%02d:%02d ", gps.time.hour(), gps.time.minute(), gps.time.second());
    Serial.println(sz);
    Serial.println("");
    dataFile.print(sz);
    dataFile.print(", ");




    Serial.println("Latitude:");
    Serial.println(GPS_AR[0], 6);
    dataFile.print(GPS_AR[0], 6);
    dataFile.print(", ");
    dtostrf(GPS_AR[0], 8, 3, buf);
    ble.write("BEGIN TRANSMISSION \r\n");
    strcat(buf, "A");
    ble.write(buf);
    ble.write("\r\n");

    Serial.println("Longitude:");
    Serial.println(GPS_AR[1], 6);
    dataFile.print(GPS_AR[1], 6);
    dataFile.print(", ");
    dtostrf(GPS_AR[1], 8, 3, buf);
    strcat(buf, "O");
    ble.write(buf);
    ble.write("\r\n");

    Serial.println("Altitude Feet:");
    Serial.println(GPS_AR[2]);
    dataFile.print(GPS_AR[2], 6);
    dataFile.print(", ");
    dtostrf(GPS_AR[2], 8, 3, buf);
    strcat(buf, "E");
    ble.write(buf);
    ble.write("\r\n");

    Serial.println("Speed:");
    Serial.println(GPS_AR[3]);
    dataFile.print(GPS_AR[3], 6);
    dataFile.print(", ");
    dtostrf(GPS_AR[3], 8, 3, buf);
    strcat(buf, "S");
    ble.write(buf);
    ble.write("\r\n");

    Serial.println("temperature:");
    Serial.println(BME_AR[0]);
    dataFile.print(BME_AR[0], 6);
    dataFile.print(", ");
    dtostrf(BME_AR[0], 8, 3, buf);
    strcat(buf, "T");
    ble.write(buf);
    ble.write("\r\n");

    Serial.println("pressure:");
    Serial.println(BME_AR[1]);
    dataFile.print(BME_AR[1], 6);
    dataFile.print(", ");
    dtostrf(BME_AR[1], 8, 3, buf);
    strcat(buf, "P");
    ble.write(buf);
    ble.write("\r\n");

    Serial.println("humidity:");
    Serial.println(BME_AR[2]);
    dataFile.print(BME_AR[2], 6);
    dataFile.println("");
    dtostrf(BME_AR[2], 8, 3, buf);
    strcat(buf, "H");
    ble.write(buf);
    ble.write("\r\n");

    Serial.println("X G:");
    Serial.println(IMU_AR[0]);
    dataFile.print(IMU_AR[0], 6);
    dataFile.print(", ");
    dtostrf(IMU_AR[0], 8, 3, buf);
    strcat(buf, "X");
    ble.write(buf);
    ble.write("\r\n");

    Serial.println("Y G:");
    Serial.println(IMU_AR[1]);
    dataFile.print(IMU_AR[1], 6);
    dataFile.print(", ");
    dtostrf(IMU_AR[1], 8, 3, buf);
    strcat(buf, "Y");
    ble.write(buf);
    ble.write("\r\n");

    Serial.println("Z G:");
    Serial.println(IMU_AR[2]);
    dataFile.print(IMU_AR[2], 6);
    dataFile.print(", ");
    dtostrf(IMU_AR[2], 8, 3, buf);
    strcat(buf, "Z");
    ble.write(buf);
    ble.write("\r\n");

    ble.write("END TRANSMISSION \r\n");





    flag = 0;
    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
  }

}

void doingSomething2()
{
  char buf[33];
  File dataFile = SD.open("cool.txt", FILE_WRITE);
  if (mode == 1 && dataFile ) {
    Serial.println("Latitude:");
    Serial.println(GPS_AR[0], 6);
    dataFile.print(GPS_AR[0], 6);
    dataFile.print(", ");
    //dtostrf(GPS_AR[0], 8, 3, buf);
    //ble.write(buf);
    //ble.write("\r\n");

    Serial.println("Longitude:");
    Serial.println(GPS_AR[1], 6);
    dataFile.print(GPS_AR[1], 6);
    dataFile.print(", ");
    //dtostrf(GPS_AR[1], 8, 3, buf);
    //ble.write(buf);
    //ble.write("\r\n");

    Serial.println("Altitude Feet:");
    Serial.println(GPS_AR[2]);
    dataFile.print(GPS_AR[2], 6);
    dataFile.print(", ");
    //dtostrf(GPS_AR[2], 8, 3, buf);
    //ble.write(buf);
    //ble.write("\r\n");

    Serial.println("temperature:");
    Serial.println(BME_AR[0]);
    dataFile.print(BME_AR[0], 6);
    dataFile.print(", ");

    Serial.println("pressure:");
    Serial.println(BME_AR[1]);
    dataFile.print(BME_AR[1], 6);
    dataFile.print(", ");

    Serial.println("humidity:");
    Serial.println(BME_AR[2]);
    dataFile.print(BME_AR[2], 6);
    dataFile.println("");





    flag = 0;
    dataFile.close();
  }

}






void setup()
{
  //serial and GPS
  Serial.begin(115200);
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




  ble.begin(9600);//start ble
  Serial.println("BLE start");//verify


  Serial.println("all systems nominal.");

  //allow gps to "warm up"
  delay(5000);

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_Timer
  ISR_Timer.setInterval(TIMER_INTERVAL_5S,    doingSomething1);
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
      GPS_AR[0] = (gps.location.lat() + GPS_AR[0]) / 2;
      GPS_AR[1] = (gps.location.lng() + GPS_AR[1]) / 2;
      GPS_AR[2] = (gps.altitude.feet() + GPS_AR[2]) / 2;
      GPS_AR[3] = (gps.speed.mph() + GPS_AR[3]) / 2;
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
    flag = 1;
  }
  else {
    BME_AR[0] = (bme.temperature + BME_AR[0]) / 2;
    BME_AR[1] = ((bme.pressure / 100.0) + BME_AR[1]) / 2;
    BME_AR[2] = (bme.humidity + BME_AR[2]) / 2;
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

      IMU_AR[0] = (sensorValue.un.accelerometer.x) / 9.8;
      IMU_AR[1] = (sensorValue.un.accelerometer.y) / 9.8;
      IMU_AR[2] = (sensorValue.un.accelerometer.z) / 9.8;

  }





}
