/**
 * 
 * Connections:
 * NodeMCU Lolin
 * DHT 11
 * MQ135
 * 
 * DHT Data: D2
 * MQ135 Data: A0
 * 
 * 
 */

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "DHT.h"

#define DHTPIN D2     // what digital pin we're connected to
#define DHTTYPE DHT11 

#define MQ135_DEFAULTPPM 399 //default ppm of CO2 for calibration
#define MQ135_DEFAULTRO 68550 //default Ro for MQ135_DEFAULTPPM ppm of CO2
#define MQ135_SCALINGFACTOR 116.6020682 //CO2 gas value
#define MQ135_EXPONENT -2.769034857 //CO2 gas value
#define MQ135_MAXRSRO 2.428 //for CO2
#define MQ135_MINRSRO 0.358 //for CO2

 
const char* ssid = "HELMS_DEEP";
const char* password = "1q0o6yhn";
 

WiFiServer server(80);

int pin = A0;
DHT dht(DHTPIN, DHTTYPE);


unsigned long SLEEP_TIME = 30000; // Sleep time between reads (in seconds)
//VARIABLES
float mq135_ro = 10000.0;    // this has to be tuned 10K Ohm
int val = 0;                 // variable to store the value coming from the sensor
float valAIQ =0.0;
float lastAIQ =0.0;
/*
 * get the calibrated ro based upon read resistance, and a know ppm
 */
long mq135_getro(long resvalue, double ppm) {
  return (long)(resvalue * exp( log(MQ135_SCALINGFACTOR/ppm) / MQ135_EXPONENT ));
}

/*
 * get the ppm concentration
 */
double mq135_getppm(long resvalue, long ro) {
  double ret = 0;
  double validinterval = 0;
  validinterval = resvalue/(double)ro;
  if(validinterval<MQ135_MAXRSRO && validinterval>MQ135_MINRSRO) {
  ret = (double)MQ135_SCALINGFACTOR * pow( ((double)resvalue/ro), MQ135_EXPONENT);
  }
  return ret;
}

/*****************************  MQGetPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         pcurve      - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm) 
         of the line could be derived if y(rs_ro_ratio) is provided. As it is a 
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
         value.
************************************************************************************/ 
int  MQGetPercentage(float rs_ro_ratio, float ro, float *pcurve)
{
  return (double)(pcurve[0] * pow(((double)rs_ro_ratio/ro), pcurve[1]));
}


String toJson(float mq, float temp, float hum) {
  char buffer[256];
  //
  // Step 1: Reserve memory space
  //
  StaticJsonBuffer<200> jsonBuffer;
  
  //
  // Step 2: Build object tree in memory
  //
  JsonObject& root = jsonBuffer.createObject();
  root["value"] = mq;
  root["temp"] = temp;
  root["humidity"] = hum;
  
  //
  // Step 3: Generate the JSON string
  //
  root.printTo(buffer, sizeof(buffer));
  return buffer;
}


void setup() {
  // config static IP
  IPAddress ip(192, 168, 1, 124); 
  IPAddress gateway(192, 168, 1, 1); 
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0); 
  WiFi.config(ip, gateway, subnet);
  // put your setup code here, to run once:
  Serial.begin(9600);

  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  dht.begin();
}

void loop() {

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  
  client.flush();

  
  uint16_t valr = analogRead(pin);// Get AIQ value
  
  uint16_t val =  ((float)22000*(1023-valr)/valr); 
  
  //during clean air calibration, read the Ro value and replace MQ135_DEFAULTRO value with it, you can even deactivate following function call.
  //mq135_ro = mq135_getro(val, MQ135_DEFAULTPPM);
  //convert to ppm (using default ro)
  valAIQ = mq135_getppm(val, MQ135_DEFAULTRO);

  Serial.print("Analog: ");
  Serial.print(valr);
  Serial.print ( "Val / Ro / value:");
  Serial.print ( val);
  Serial.print ( " / ");
  Serial.print ( mq135_ro);
  Serial.print ( " / ");
  Serial.println ( valAIQ);

  
  float temp = dht.readTemperature(false);
  float humid = dht.readHumidity();
  int cnt = 0;
  /*while(isnan(temp) || isnan(humid)) {
    temp = dht.readTemperature(false);
    humid = dht.readHumidity();
    cnt++;
    if (cnt > 2000) {
      break;
    }
  }
  */
  
  // Match the request
  
  
  if (request.indexOf("/s") != -1)  {
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println(""); //  do not forget this one
    /*client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<h1>");*/
    client.println(toJson(valAIQ, temp, humid));
    /*
    client.println("</h1>");
    client.println("</html>");
    */
  }
  
  
  
  
  
  
  
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
  delay(1000);
 
}
 




