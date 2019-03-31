#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>
#define relay 13


modbusDevice regBank;
modbusSlave slave;

MDNSResponder mdns;
ESP8266WebServer server(80);


const char* ssid = "siec";
const char* password = "password";

String state;
String feedback;
int zmienna;
boolean opened = true;
boolean closed = false;

String  remote_fb;
String  availiable_fb;
String  opened_close_fb;
String  tripped_fb;
String  availabe_fb;
String  remote_com;
String  availiable_com;
String  opened_close_com;

boolean remote = false;
boolean tripped = false;

boolean ok = false;  //available
boolean remote_close = false;
boolean remote_open = false;

String  power;
String  current;
String  volt;
String  freq;
String  temp;



void handleRoot() { }

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup()
{
  delay(500);

  regBank.setId(5);
  slave.setBaud(19200);
  slave._device = &regBank;

  pinMode(relay, OUTPUT);

  server.begin();

  delay(500);

  server.on("/temp", []()  {
    state = server.arg("state");
    server.send(200, "text/plain", feedback);
  });

  server.onNotFound(handleNotFound);

  //adresy zamienic na zmienne
  regBank.add(30001);  //CURRENT
  regBank.add(30002);  //POWER
  regBank.add(30003);  //VOLTAGE
  regBank.add(30004);  //FREQUENCY
  regBank.add(30005);  //LIVE BIT
  regBank.add(10001);  //OPENED
  regBank.add(10002);  //CLOSED
  regBank.add(10003);  //REMOTE
  regBank.add(10004);  //AVAILABLE
  regBank.add(10005);  //TRIPPED
  regBank.add(11);     //CLOSE - KM
  regBank.add(12);     //OPEN - KM

  zmienna = 0;
}

void loop()
{


  server.handleClient();

  power   = (state.substring (state.indexOf("P") + 1, state.indexOf(";C")));
  current = (state.substring (state.indexOf("C") + 1, state.indexOf(";V")));
  volt    = (state.substring (state.indexOf("V") + 1, state.indexOf(";F")));
  freq    = (state.substring (state.indexOf("F") + 1, state.indexOf(";T")));
  temp    = (state.substring (state.indexOf("T") + 1, state.indexOf(";")));


  zmienna++;
  if (zmienna == 500)
    zmienna = 0;


  // LOCAL REMOTE SWITCH
  if (state.indexOf("R") > 0)
  {
    remote_close = regBank.get(11);
    remote_open  = regBank.get(12);

    remote = true;
    if (remote_close && !tripped)
    {
      opened = false;
      closed = true;
    }
  }
  else
  { remote = false;


    if (remote_open || tripped )
    {
      opened = true;
      closed = false;
    }

  }

  //TRIPPED FROM PHONE
  if (state.indexOf("B") > 0) {
    tripped = true;
    closed = false;
    // opened = true;
    ok = false;
  }
  if (state.indexOf("E") > 0) {
    tripped = false;
  }

  //AVAILABLE FROM PHONE
  if (state.indexOf("A") > 0  && !tripped) {
    ok = true;
  }
  if (state.indexOf("N") > 0 || tripped) {
    ok = false;
  }


  regBank.set(30001, current.toInt());
  regBank.set(30002, power.toInt());
  regBank.set(30003, volt.toInt());
  regBank.set(30004, freq.toInt());
  regBank.set(30005, zmienna);




  if (remote == false)
  {
    if ((state.indexOf("O") > 0 ) || tripped)
    {
      opened = true;
      closed = false;
    }
    if ((state.indexOf("K") > 0) && !(tripped))
    {
      opened = false;
      closed = true;
    }
  }

  regBank.set(10001, opened);
  regBank.set(10002, closed);
  regBank.set(10003, remote);
  regBank.set(10004, ok);
  regBank.set(10005, tripped);


  if (opened)
  {
   opened_close_fb = 'O';
   digitalWrite(relay, LOW);
   
  }
  else
  {
    digitalWrite(relay, HIGH);
    opened_close_fb = 'K';
  }


  //SEND TO PHONE
  if (ok)
    availiable_fb = 'A';
  else  availiable_fb = 'N';

  if (tripped)
    tripped_fb = 'B';
  else  tripped_fb = 'E';

  if (remote)
    remote_fb = 'R';
  else  remote_fb = 'L';

  feedback = remote_fb + availiable_fb + opened_close_fb + tripped_fb;

  slave.run();

}
