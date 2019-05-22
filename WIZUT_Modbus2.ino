#include <Adafruit_BME280.h>


#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>
#include <Adafruit_Sensor.h>


Adafruit_BME280 bmp; // I2C

modbusDevice regBank;
modbusSlave slave;
int foto ;
int humidity ;
int temperature ;
int pressure;
int zmienna;
int fotoPin = 0;
boolean status_bmp;
void setup()
{

  regBank.setId(2);
  slave.setBaud(19200);
  slave._device = &regBank;
  delay(500);

  regBank.add(30001);  //LIGHT
  regBank.add(30002);  //TEMP
  regBank.add(30003);  //HUMIDITY
  regBank.add(30004);  //PRESSURE
  regBank.add(30005);  //ROLLING COUNTER out

  pinMode(fotoPin, INPUT);
  
  if (!bmp.begin())
    status_bmp = false;
  else
    status_bmp = true;
}

void loop()
{
  zmienna++;
  if (zmienna == 500)
    zmienna = 0;
  foto = analogRead(fotoPin);


  regBank.set(30001, foto);
  
  if (status_bmp)
  { 
  humidity =  bmp.readHumidity();
    temperature = bmp.readTemperature();
   pressure = bmp.readPressure();

    regBank.set(30002, temperature);
    regBank.set(30003, humidity);
    regBank.set(30004, pressure);
  }
  
  regBank.set(30005, zmienna);

  slave.run();
  // delay(100);
}
