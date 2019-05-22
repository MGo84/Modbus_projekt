#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "siec";
const char* password = "password";

#define relay 13


modbusDevice regBank;
modbusSlave slave;

const short int lampka = 16;//GPIO16

MDNSResponder mdns;
ESP8266WebServer server(80);

String state;
String feedback;
int rolling_counter;
int temperature, pressure, humidity, light;
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
String status_esp1_km_fb;
String status_esp2_km_fb;

boolean remote = false;
boolean tripped = false;
boolean status_M2 = false;

boolean ok = false;  //available
boolean remote_close = false;
boolean remote_open = false;

String  power;
String  current;
String  volt;
String  freq;
String  temp;
String  counter_app;


int rolling_counter1;
int rolling_counter2;
int rolling_counter3;
int rolling_counter4;
int rolling_counter5;

int rolling_counter1_app;
int rolling_counter2_app;
int rolling_counter3_app;
int rolling_counter4_app;
int rolling_counter5_app;


void handleRoot() {}

void handleNotFound() {
  //digitalWrite(led, 1);
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

void setup(void) {

 //n delay(500);

  regBank.setId(5);
  slave.setBaud(19200);
  slave._device = &regBank;

  pinMode(relay, OUTPUT);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }



  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  server.on("/dane", []()  {
    state = server.arg("state");
    server.send(200, "text/plain", feedback);
     delay (100);
  });


  server.onNotFound(handleNotFound);

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
  regBank.add(10006);  //WIFI STATUS
  regBank.add(11);     //CLOSE - KM
  regBank.add(12);     //OPEN - KM
  regBank.add(13);     //STATUS MODUL 2 KM
  rolling_counter = 0;

  regBank.add(40001);  //TEMPEREATURE
  regBank.add(40002);  //HUMIDITY
  regBank.add(40003);  //PRESSURE
  regBank.add(40004);  //LIGHT
  regBank.add(40005);  //LIVE BIT



  server.begin();

}

void loop(void) {



  server.handleClient();


  power   = (state.substring (state.indexOf("P") + 1, state.indexOf(";C")));
  current = (state.substring (state.indexOf("C") + 1, state.indexOf(";V")));
  volt    = (state.substring (state.indexOf("V") + 1, state.indexOf(";F")));
  freq    = (state.substring (state.indexOf("F") + 1, state.indexOf(";T")));



  counter_app = (state.substring (state.indexOf("X;"), state.indexOf(";;H")));
  rolling_counter5_app = counter_app.toInt();

  rolling_counter++;
  if (rolling_counter == 500)
    rolling_counter = 0;


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


    if (remote_open || tripped )
    {
      opened = true;
      closed = false;
    }
  }
  else
  { remote = false;
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
  regBank.set(30005, rolling_counter);





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



  
rolling_counter5 = regBank.get(40005);



  status_M2 = regBank.get(13);

  if (status_M2)
  {  status_esp2_km_fb = 'Z';
  temperature = regBank.get(40001);
  pressure = regBank.get(40002);
  humidity = regBank.get(40003);
  light = regBank.get(40004);
  }
  else
    status_esp2_km_fb = '0';

  



  rolling_counter1_app = rolling_counter2_app;
  rolling_counter2_app = rolling_counter3_app;
  rolling_counter3_app = rolling_counter4_app;
  rolling_counter4_app = rolling_counter5_app;

  if ((rolling_counter1_app == rolling_counter2_app) && (rolling_counter2_app == rolling_counter3_app) && (rolling_counter3_app == rolling_counter4_app) && (rolling_counter4_app == rolling_counter5_app))
    regBank.set(10006, 0);
  else
    regBank.set(10006, 1);



  feedback = remote_fb + availiable_fb + opened_close_fb + tripped_fb + status_esp2_km_fb + 'X' + ';' + rolling_counter + ';' + ';' + 'H' + humidity + ';' + ';' + 'T' + temperature + ';' + ';' + 'P' + pressure + ';' + ';' + 'F' + light + ';' + ';' + 'S' + rolling_counter5+ ';'+ ';'+ ';';



  slave.run();
 
}
