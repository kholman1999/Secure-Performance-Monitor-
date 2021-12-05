ba//TIMER INIT START

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
#define TIMER_INTERVAL_5S             5000L
#define TIMER_INTERVAL_3S            3000L


void TimerHandler()
{
  ISR_Timer.run();
}

//TIMER INIT DONE

//////general includes//////
#include <SPI.h>
#include <SD.h>
#include <TinyGPS++.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <Adafruit_BNO08x.h>
#include <SoftwareSerial.h>



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
int basez;
//BigNumber mod;
float tempx;
float tempy;
float tempz;
int stepsCnt = 0;
int highRuns = 0;

//prev array
float GPS_PREV_AR[4] = {};
float BME_PREV_AR[3] = {};
float IMU_PREV_AR[3] = {};



//main array
float GPS_AR[4] = {};
float BME_AR[3] = {};
float IMU_AR[3] = {};

int  pk1 = 129; // public key one (exponent) can be adjusted
char pk2[] = "10283216039871810935867070308763590033267924706279480036805479708407577958672010439491502023522562278580887361154108790868660131671345775095268853731990497";
BigNumber mod = BigNumber(pk2); //converts number in string form to BigNumber data type

//int pk1;
//char pk2[155];

TinyGPSPlus gps;
Adafruit_BME680 bme; // I2C
Adafruit_BNO08x  bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;
HardwareSerial ble(D52, D53); // RX, TX
//SoftwareSerial serial_connection(1, 0);
SoftwareSerial ss(1, 0);

//////FUNCTIONS//////
void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  }
  if (!bno08x.enableReport(SH2_STEP_COUNTER)) {
    Serial.println("Could not enable step counter");
  }
}



void(* resetFunc) (void) = 0;
void removeSpaces(char* s) {
  *std::remove(s, s + strlen(s), ' ') = 0;
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}


void doingSomething1()
{
  if (highRuns > 0)
  {
    return;
  }
  
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
    dtostrf(IMU_AR[0], -3, 2, buf);

    strcat(packet, "X");
    strcat(packet, buf);

    //format y gforce//
    Serial.println("Y G:");
    Serial.println(IMU_AR[1]);
    dtostrf(IMU_AR[1], -3, 2, buf);

    strcat(packet, "Y");
    strcat(packet, buf);

    //format z gforce//
    Serial.println("Z G:");
    Serial.println(IMU_AR[2]);

    dtostrf(IMU_AR[2], -3, 2, buf);
    strcat(packet, "Z");
    strcat(packet, buf);


    //format steps//
    Serial.println("Steps:");
    Serial.println(stepsCnt);

    dtostrf(stepsCnt, 0, 1, buf);
    strcat(packet, "C");
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

void doingSomething2()
{
  if (highRuns <= 0)
  {
    return;
  }
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
    dtostrf(IMU_AR[0], -3, 2, buf);

    strcat(packet, "X");
    strcat(packet, buf);

    //format y gforce//
    Serial.println("Y G:");
    Serial.println(IMU_AR[1]);
    dtostrf(IMU_AR[1], -3, 2, buf);

    strcat(packet, "Y");
    strcat(packet, buf);

    //format z gforce//
    Serial.println("Z G:");
    Serial.println(IMU_AR[2]);

    dtostrf(IMU_AR[2], -3, 2, buf);
    strcat(packet, "Z");
    strcat(packet, buf);


    //format steps//
    Serial.println("Steps:");
    Serial.println(stepsCnt);

    dtostrf(stepsCnt, 0, 1, buf);
    strcat(packet, "C");
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

float Gps_Lat_Check(float Lat)
{
  if (abs(Lat) > 90)
  {
    if (Lat < 0) {
      return -90;
    }
    else {
      return 90;
    }
  }
  else {
    return Lat;
  }
}




float Gps_Lng_Check(float Lng)
{
  if (abs(Lng) > 180)
  {
    if (Lng < 0) {
      return -180;
    }
    else {
      return 180;
    }
  }
  else {
    return Lng;
  }
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




double dataVerify(float newData, float oldData) {
  double dif = abs((newData - oldData) / oldData) * 100;
  if (oldData != 0)  {
    if (abs(dif) > 15) {
      return oldData;
    }
    else {
      return newData;
    }
  }
  else
    return newData;

}


float ZeroHandle (float N, float D)
{
  if (D = 0) {
    return N;
  }
  else
    return N / D;
}

void append(char* s, char c) {
  int len = strlen(s);
  s[len] = c;
  s[len + 1] = '\0';
}



void setup()
{
  //serial and GPS
  Serial.begin(9600);
  ss.begin(9600);//This opens up communications to the GPS
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



  //  //get PUBLIC KEYS from SD card
  //  File securityFile = SD.open("Secure.txt", FILE_READ);
  //  String received = "";
  //  char ch;
  //  while (securityFile.available()) {
  //    ch = securityFile.read();
  //    if (ch == '\n')
  //    {
  //      break;
  //    }
  //    else {
  //      received += ch;
  //    }
  //  }
  //  pk1 = received.toInt();
  //  Serial.println(pk1);
  //
  //  char in[0];
  //  while (securityFile.available()) {
  //    ch = securityFile.read();
  //    if (ch == '\n')
  //    {
  //      break;
  //    }
  //    else {
  //      //      in[0] = ch;
  //      //      Serial.println(ch);
  //      //       Serial.println(in);
  //      //      strcat(pk2, in);
  //      append(pk2, ch);
  //    }
  //  }
  //  removeSpaces(pk2);
  //  Serial.println(pk2);
  //
  //  //close file
  //  securityFile.close();


  //init BME
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


    //    while (1) {
    //      delay(1000);
    //      //ble.write("RESET");
    //    }
  }
  setReports();
  Serial.println("Reading events");
  Serial.println("BNO08x online.");


  Serial.println("all systems nominal.");

  //allow gps to "warm up"
  delay(5000);


  for (int u = 0; u < 10; u++)
  {
    basez += sensorValue.un.accelerometer.z;
  }

  basez = 9.8;
  Serial.println(basez);

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_Timer
  ISR_Timer.setInterval(TIMER_INTERVAL_5S,    doingSomething1);
  ISR_Timer.setInterval(TIMER_INTERVAL_3S,    doingSomething2);
  //ISR_Timer.setInterval(TIMER_INTERVAL_1S, GPSTIMER);




}


void loop()
{
  if (BME_AR[0] > 100 || BME_AR[0] < 80 || GPS_AR[3] > 10)
  {
    highRuns = 20;
  }






  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  if (flag == 0) {
    BME_AR[0] = 1.8 * ( bme.temperature) + 32;
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
      tempx = sensorValue.un.accelerometer.x;
      tempy = sensorValue.un.accelerometer.y;
      tempz = sensorValue.un.accelerometer.z;
      if (tempx < 0.1)
      {
        IMU_AR[0] = 0;
      }
      else {
        IMU_AR[0] = tempx / basez;
      }
      if (tempy < 0.1)
      {
        IMU_AR[1] = 0;
      }
      else {
        IMU_AR[1] = tempy / basez;
      }
      if (tempz < 0.1)
      {
        IMU_AR[2] = 0;
      }
      else {
        IMU_AR[2] = tempz / basez;
      }
      break;
    case SH2_STEP_COUNTER:
      stepsCnt = sensorValue.un.stepCounter.steps;
      break;

  }

  while (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      if (gps.location.isValid()) {
        GPS_AR[0] = Gps_Lat_Check(gps.location.lat()) + 90;
        GPS_AR[1] = Gps_Lng_Check(gps.location.lng()) + 180;


        GPS_AR[2] = gps.altitude.feet();
        GPS_AR[3] = gps.speed.mph();
        break;

      }
    }
  }



  if (highRuns > 0)
  {
    highRuns = highRuns - 1;
  }
  if (highRuns < 0)
  {
    highRuns = 0;
  }
  
  flag = 1;


}
