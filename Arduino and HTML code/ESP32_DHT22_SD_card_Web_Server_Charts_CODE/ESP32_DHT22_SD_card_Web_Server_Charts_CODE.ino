
/* NOTES:
   1) For more explanation on how the system works refer to the flow chart and the thesis pdf file.
   2) The comments above most of the code blocks were placed mainly as a guide for people still new to these consepts.
   3) Contact me if there is any problem with the code.
*/

// SD card libraries
#include "FS.h"
#include <SD.h>
#include <SPI.h>

// Timer library
// We use the the Ticker library to execute the reading and storing of our sensor data
// in the setup() function, in order to control how often we call our functions.

#include <Ticker.h>

// Libraries for: Wifi, Web Server και NTP client
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>

// SPIFFS library
#include <SPIFFS.h>

// DHT sensor library
#include <DHT.h>

// Initialize the DHT22 sensor
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// CS pin for the SD card module
#define SD_CS 5

// Replace the "ssidname" and "password" with the ones
// of the wifi you want to connect to
const char* ssid     = "ssidname";
const char* password = "password";

// We create a filePath variable and assign it the path
// of the file in the SD card where we want our sensor data to be stored.
// For convenience we name the file "data.csv".
char* filePath = "/data.csv";

// In this variable we store every sampling we receive from the sensor
// and the NTP client (for the timestamp), in order to add them
// to the file in the SD card
String dataMessage;

// They hold the temperature (Celsius) and humidity data
float temperature_C;
float humidity;

// A simple counter to assign a number next to each sampling
// We initialize it with 1 because we want our numbering to start from 1
int dataID = 1;

// Initialize 3 Ticker objects:
// one to read the data from the sensor
// one to store the data in the SD card
// and one to receive the date and time from the NTP client
Ticker readData;
Ticker saveData;
Ticker getTime;

// The .attach() method by default takes as first argument the time period (in seconds)
// after which it calls a function and as second argument the function that it calls.
// Therefore, we declare the variable execPeriod that will hold the time period we want
// to pass to the method and initilize it with the value 10.
// Now, each function that we will pass to it will be called every 10 seconds.
float execPeriod = 10;

// We declare the NTP client that will receive the date and time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to store the date and time we receive from the NTP client
String formattedDate;
String dayStamp;
String timeStamp;

// We create an AsyncWebServer object in port 80
AsyncWebServer server(80);


//============================================================================

void setup(){

        // Initialize serial port to 115200 baudrate
        Serial.begin(115200);

		// We set the GPIO 4 of the ESP32 to logic 1, otherwise the DHT22 sensor
		// (whose data pin is connected there) does not provide stable and accurate readings

        pinMode(4,INPUT_PULLUP);

// --------------------------------------------------------------------------

        // Initialize the SPIFFS memory
        if(!SPIFFS.begin()){
                Serial.println("An Error has occurred while mounting SPIFFS");
                return;
        }
// ---------------------------------------------------------------------------

        // Function to connect to wifi
        wifiConnect();

		// Initialize the NTPClient to receive the date and time
        timeClient.begin();

		// We declare an offset time (in seconds) to adjust the data
		// to Greece's timezone (change it accordingly for your country)
		// For Greece it is GMT +3. Thus, the offset = 3600*3 = 10800
		// In other words find the GMT of your country, multiply it with 3600
		// to convert in seconds and put the result as argument of the following method
        timeClient.setTimeOffset(10800);

        // Initialize SD card
        SD.begin(SD_CS);

		// We check if there is any problem with the SC card before use
        checkSDCard();

		// First thing we do when our file "data.csv" is created, is to
		// place the labels of our data on top
        addLabels();

		// We start the sensor
        dht.begin();

		// We call our main functions to read and store our data with their timestamps
		// With the ticker objects the whole sampling process starts 10 seconds after the
		// above initialization and repeats every 10 seconds
		// Thus, if we want to begin the sampling process right after the initialization,
		// we just call these 3 functions once, before the code block of the ticker objects
		// starts executing
        getReadings();
        getTimeStamp();
        logSDCard();


		// First, we collect the data from the DHT22 sensor
        readData.attach(execPeriod, getReadings);

		// Next, we receive the date and time from the NTP client
        getTime.attach(execPeriod, getTimeStamp);

		// And store the collected data in the SD card
        saveData.attach(execPeriod, logSDCard);

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

  // We establish the communication with the server like this:
  // When the ESP32 receives request to the root "/", we send the HTML information
  // that is located in the "index.html" file in the SPIFFS memory, which contains
  // all the information about the template of our graphs.
  // When the ESP32 receives request to the “/temperature” and “/humidity”,
  // we send the temperature and humidity data we receive from the DHT22 sensor
  // respectively (in plain text format), to be then added on the graphs for
  // real-time visualization

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(temperature_C).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(humidity).c_str());
  });


// ---------------------------------------------------------------------------

 // We start the server
  server.begin();

}

void loop(){

		// There is nothing being executed in the loop() function
}


//============================================================================
// --------------------------------------------------------------------
// Function to connect to WiFi with SSID and password (Can be modified)
// --------------------------------------------------------------------

void wifiConnect(){

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");

	// If the connection is successful, the IP adress that has been assigned
	// to the ESP32, will be displayed on the Serial monitor for you
	// to copy/paste in a new tab in your browser to start the visualization

    Serial.println("");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

}
//============================================================================

//============================================================================
// ------------------------------------------------------
// Function to check SD card before use (Can be modified)
// ------------------------------------------------------

void checkSDCard(){

    Serial.print("Initializing SD card...");

    if (!SD.begin(SD_CS)) {
    Serial.println("initialization failed!");
    return;
    }
    Serial.println("initialization done.");

	// During the build of the system, I did not connect an electrolytic capacitor
	// on the ESP32 to avoid accidental damage to it. So, to upload the
	// code to the microcontroller I need to keep the button "BOOT" pressed
	// for 2-3 seconds until the upload produre is complete.
	// Therefore, we put the following delay of 5 seconds here to keep up with the
	// serial monitor, and be able to press the reset button "EN" in time
	// for the correct printing of our messages on the screen,
	// before our code starts executing.
	// This way, we will not miss any printing of our code before we can see it,
	// which can lead to the false conclusion that there is an error in the code,
	// when in fact it has simply been executed without being able to see its result.

    delay(5000);
}
//===========================================================================

//============================================================================
// ---------------------------------------------------------------------------
// Function to add the labels of the data at the top of the newly created
// empty file in the SD card (Can be modified)
// ---------------------------------------------------------------------------

void addLabels(){

    File file = SD.open(filePath);

    if(!file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    delay(1000);

    Serial.println("File created");

    delay(1000);
    Serial.println("Saving labels to file...");

    writeFile(SD, filePath, "DataID, Day, Hour, Temperature, Humidity \r\n");
  }
  else {
    Serial.println("File already exists");
  }

  file.close();

}
//===========================================================================

//===========================================================================
// ---------------------------------------------------------------------
// Function for receiving the temperature and humidity data from the
// DHT22 sensor (Can be modified)
// ---------------------------------------------------------------------

void getReadings(){

    temperature_C = dht.readTemperature();
    humidity = dht.readHumidity();

    if (isnan(humidity) || isnan(temperature_C)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
    }
    Serial.println("Getting the Data...");

    Serial.print("Temperature (Celsius): ");
    Serial.println(temperature_C);

    Serial.print("Humidity: ");
    Serial.println(humidity);

    Serial.println("Data collected!");
}
//===========================================================================

//===========================================================================
// -----------------------------------------------------------------------------
// Function for receiving the date and time from the NTPClient (Can be modified)
// -----------------------------------------------------------------------------

void getTimeStamp() {

    while(!timeClient.update()) {
        timeClient.forceUpdate();
    }

	// The variable formattedDate has the following format:
    // 2018-05-28T16:00:13Z
	// Thus, we need to exctract from it the date and time
	// with simple string slicing
    formattedDate = timeClient.getFormattedDate();
    Serial.println(formattedDate);


    // Exctract date
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    Serial.println(dayStamp);

    // Exctract time
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
    Serial.println(timeStamp);

    Serial.println("Time and Date collected!");
}
//===========================================================================

//===========================================================================
// ------------------------------------------------------------------
// Function to write the sensor data in the SD card (Can be modified)
// ------------------------------------------------------------------

// We use the dataID counter to number each data entry, starting from "1"

void logSDCard() {

dataMessage = String(dataID) + "," + String(dayStamp) + "," + String(timeStamp) + "," + String(temperature_C) + "," + String(humidity) + "\r\n";

	// We print each data entry on the Serial monitor
    Serial.print("Save Data: ");
    Serial. println(dataMessage);

	// and then we add it to our file in the SC card
    appendFile(SD, filePath, dataMessage.c_str());

	// We increase the counter by 1 after each entry
    dataID += 1;
}
//===========================================================================

//===========================================================================
//------------------------------------------------------
// Function to write data in the SD card (Do not modify)
//------------------------------------------------------

void writeFile(fs::FS &fs, const char * path, const char * message){

    Serial.printf("Writing file: %s\n", path);
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    if(file.print(message)){
      Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}
//===========================================================================

//===========================================================================
//---------------------------------------------------------
// Function to append data in the SD card (Do not modify)
//---------------------------------------------------------

void appendFile(fs::FS &fs, const char * path, const char * message){

    Serial.printf("Appending to file: %s\n", path);
    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}
