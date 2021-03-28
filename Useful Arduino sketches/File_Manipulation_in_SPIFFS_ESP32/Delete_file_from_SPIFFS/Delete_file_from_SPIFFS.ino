

#include "SPIFFS.h"


char* filePath = "/index.html";


void listAllFiles(){


	File root = SPIFFS.open("/");

  	File file = root.openNextFile();

  	while(file){

		Serial.print("FILE: ");
  		Serial.println(file.name());

  		file = root.openNextFile();

  	}

}

void setup() {

	// just to catch on with the serial monitor, to be able to reset in time and not lose the first execution of the program
	delay(5000);

	Serial.begin(115200);


  	if (!SPIFFS.begin(true)) {

		Serial.println("An Error has occurred while mounting SPIFFS");
		return;

  	}

  	Serial.println("\n\n----Listing files BEFORE file removal----");

  	listAllFiles();


	// remove file

  	SPIFFS.remove(filePath);


  	Serial.println("\n\n----Listing files AFTER file removal----");

  	listAllFiles();


}

void loop() {

}
