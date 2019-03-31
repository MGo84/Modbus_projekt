#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <SPI.h>

Adafruit_BME280 bmp; // I2C

modbusDevice regBank;
modbusSlave slave;

int zmienna;
int fotoPin = 0;

void setup(void)
{

  regBank.setId(2);
  slave.setBaud(19200);
  slave._device = &regBank;

  regBank.add(30001);  //LIGHT
  regBank.add(30002);  //TEMP
  regBank.add(30003);  //HUMIDITY
  regBank.add(30004);  //PRESSURE
  regBank.add(30005);  //ROLLING COUNTER

  Serial.begin(9600);
  Serial.println(F("BMP280 test"));

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);

    zmienna = 0;

  }

  void loop(void) {

    zmienna++;
    if (zmienna == 500)
      zmienna = 0;

    regBank.set(30001, analogRead(fotoPin));
    regBank.set(30002, bmp.readTemperature());
    regBank.set(30003, bmp.readHumidity());
    regBank.set(30004, bmp.readPressure());
    regBank.set(30005, zmienna);

    slave.run();
  }
