#include "TinyGPS++.h"

HardwareSerial ble(D52, D53); // RX, TX
HardwareSerial serial_connection(D0, D1); //RX=pin 0, TX=pin 1
TinyGPSPlus gps;
char buffer[33];//buffer 4 itoa
void setup()
{
  Serial.begin(9600);
  serial_connection.begin(9600);//comms to the GPS
  Serial.println("GPS Start");//verify
  ble.begin(9600);//start ble
  Serial.println("BLE start");//verify
}

void loop()
{
  while(serial_connection.available())//wait for data
  {
    gps.encode(serial_connection.read());//feed the serial NMEA data 
  }
  if(gps.location.isUpdated())//full package only
  {
    //data
    Serial.println("Satellite Count:");
    Serial.println(gps.satellites.value());
    itoa(gps.satellites.value(),buffer,10);
    ble.write(buffer);
    ble.write("\r\n");

    Serial.println("Latitude:");
    Serial.println(gps.location.lat(), 6);
    itoa(gps.location.lat(),buffer,10);
    ble.write(buffer);
    ble.write("\r\n");
    
    Serial.println("Longitude:");
    Serial.println(gps.location.lng(), 6);
    itoa(gps.location.lng(),buffer,10);
    ble.write(buffer);
    ble.write("\r\n");
    
    Serial.println("Speed MPH:");
    Serial.println(gps.speed.mph());
    Serial.println("Altitude Feet:");
    Serial.println(gps.altitude.feet());
    printDateTime(gps.date, gps.time);
    Serial.println("");
    Serial.println("");
  }
}


static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }
  
  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (serial_connection.available())
      gps.encode(serial_connection.read());
  } while (millis() - start < ms);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}
