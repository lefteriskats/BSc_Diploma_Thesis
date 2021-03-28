

#include "SPIFFS.h"

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

	Serial.begin(115200);


  	if (!SPIFFS.begin(true)) {

		Serial.println("An Error has occurred while mounting SPIFFS");

		return;

  	}

  	Serial.println("\n\n----Listing files in SPIFFS----");

  	listAllFiles();


}

void loop() {

}
