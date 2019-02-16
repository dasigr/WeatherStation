/**
 * Send and Receive SMS with SIM900.
 * 
 * @author ElexParts<contact@elexparts.com>
 * @url https://www.elexparts.com
 */

#include <Arduino.h>
#include <SIM900.h>
#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "inetGSM.h"

// Define constants.
#define DHTPIN  A0                // Temperature Sense pin
#define DHTTYPE DHT11             // DHT 11

// Configure Temperature sensor.
DHT dht(DHTPIN, DHTTYPE);
float humidity;                   // Humidity
float temperature;                // Temperature
unsigned long previousMillis = 0; // Variable to store current time.
const long interval = 5000;       // Set interval to read temperature.

// Configure GSM module.
InetGSM inet;
int httpResponseLength = 1000;
char msg[50];
char inSerial[50];
int numData;                      // Number of data received.
int i = 0;

// Simple sketch to send and receive SMS.
// To change pins for Software Serial, use the two lines in GSM.cpp.
// _GSM_TXPIN_ was set to 2
// _GSM_RXPIN_ was set to 3
int gsmStatusLed = 12;

// Flag to check if GSM has started.
boolean started = false;

void setup() {
  pinMode(gsmStatusLed, OUTPUT);

  // Serial connection.
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Initializing GSM Shield ...");

  while(!started) {
    // Start configuration of shield with baudrate (4800 or lower).
    if (gsm.begin(2400)) {
      Serial.println("\nGSM Status: READY");
      started = true;

      // Turn on LED when GSM is ready.
      digitalWrite(gsmStatusLed, HIGH);
    } else {
      Serial.println("\nGSM Status: IDLE");
    }
  }

  Serial.println("Starting Arduino web client.");

  // connection state
  boolean notConnected = true;

  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (notConnected) {
    if (inet.attachGPRS("http://globe.com.ph", "", "")) {
      notConnected = false;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  // Read IP address.
  gsm.SimpleWriteln("AT+CIFSR");
  delay(3000);

  // Read until serial buffer is empty.
  gsm.WhileSimpleRead();

  // Initialize Temperature sensor.
  dht.begin();
};

/**
 * Serial hardware read.
 */
void serialhwread() {
  i = 0;
  if (Serial.available() > 0) {
    while (Serial.available() > 0) {
      inSerial[i]=(Serial.read());
      delay(10);
      i++;
    }

    inSerial[i]='\0';
    if(!strcmp(inSerial,"/END")) {
      Serial.println("_");
      inSerial[0]=0x1a;
      inSerial[1]='\0';
      gsm.SimpleWriteln(inSerial);
    }

    // Send a saved AT command using serial port.
    if(!strcmp(inSerial,"TEST")) {
      Serial.println("SIGNAL QUALITY");
      gsm.SimpleWriteln("AT+CSQ");
    }

    // Read last message saved.
    if(!strcmp(inSerial,"HTTP Response")) {
      Serial.println(msg);
    } else {
      Serial.println(inSerial);
      gsm.SimpleWriteln(inSerial);
    }
    inSerial[0]='\0';
  }
};

/**
 * Serial software read.
 */
void serialswread() {
  gsm.SimpleRead();
};

/**
 * Log temperature data.
 */
void logTemperature(float temperature) {
  Serial.println("Logged temperature data.");

  // Account credentials, we are using Basic Authentication for now.
  // USERNAME
  // PASSWORD

  // Base-encode credentials.
  // SECRET_CREDENTIAL

  // Get access token.
  numData = inet.httpGET("api.elexlabs.com", 80, "/rest/session/token", msg, httpResponseLength);
  // numData = inet.httpGET("http://192.168.10.10", 80, "/", msg, httpResponseLength);

  // Print the results.
  Serial.println("\nGET - Length of response received:");
  Serial.println(numData);
  Serial.println("\nGET - Response received:");
  Serial.println(msg);

  /**
  HTTP/1.1 200 OK
  Cache-Control: must-revalidate, no-cache, private
  Content-Language: en
  Expires: Sun, 19 Nov 1978 05:00:00 GMT
  Server: nginx
  X-Content-Type-Options: nosniff
  X-Debug-Token: e29f2c
  X-Debug-Token-Link: /admin/reports/profiler/view/e29f2c
  X-Frame-Options: SAMEORIGIN
  X-Generator: Drupal 8 (https://www.drupal.org)
  X-Pantheon-Styx-Hostname: styx-fe3-a-5794d5fd7-qpvrq
  X-Styx-Req-Id: styx-24f5a9ba846f9378b2cb5a6c4998e4a1
  X-Ua-Compatible: IE=edge
  Accept-Ranges: bytes
  Accept-Ranges: bytes
  Via: 1.1 varnish
  Age: 0
  Accept-Ranges: bytes
  Accept-Ranges: bytes
  Date: Sun, 27 Jan 2019 05:57:57 GMT
  Via: 1.1 varnish
  Connection: close
  X-Served-By: cache-mdw17345-MDW, cache-hkg17926-HKG
  X-Cache: MISS, MISS
  X-Cache-Hits: 0, 0
  X-Timer: S1548568881.732217,VS0,VE268
  Vary: 
  RIC: 
  Content-Type: text/plain;charset=UTF-8
  Content-Length: 43

  D6t_CjaEE0buRV677WFUajX28DjBqyfH8iw_99zPMZY

  CLOSED

  */

  // We're able to get an http response when requesting for access token.
  // Now we need to parse it.
  // char access_token[60] = "D6t_CjaEE0buRV677WFUajX28DjBqyfH8iw_99zPMZY";

  // HTTP Headers.
  // Authorization Basic SECRET_CREDENTIAL
  // X-CSRF-Token ACCESS_TOKEN
  // Content-Type application/vnd.api+json
  // Accept application/vnd.api+json

  // Parameters in json format.
  /**
  {
    "data": {
      "type": "node--contact",
      "attributes": {
        "title": "Contact",
        "field_phone_number": "+639000000000",
        "field_message": "SMS message."
      }
    }
  }
  */

  // Do a POST request.
  // const char* parameters;
  // parameters = "{\"data\":{\"type\":\"node--temperature\",\"attributes\":{\"title\":\"Temperature\",\"field_value_temperature\":21}}}";
  // numdata = inet.httpPOST("dev-elexparts-api.pantheonsite.io", 80, "/json/api/node/temperature", parameters, msg, 50);

  // Print the results.
  // Serial.println("\nPOST - Number of data received:");
  // Serial.println(numdata);
  // Serial.println("\nPOST - Data received:");
  // Serial.println(msg);
};

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time we've read the temperature.
    previousMillis = currentMillis;

    // Read for new byte on serial hardware, and write them on NewSoftSerial.
    serialhwread();

    // Read for new byte on NewSoftSerial.
    serialswread();

    // Read data and store it to variables hum and temp.
    humidity = dht.readHumidity();
    temperature= dht.readTemperature();

    // Print temp and humidity values to serial monitor.
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %, Temp: ");
    Serial.print(temperature);
    Serial.print(" *C\n");

    // Log temperature data.
    logTemperature(temperature);
  }
};