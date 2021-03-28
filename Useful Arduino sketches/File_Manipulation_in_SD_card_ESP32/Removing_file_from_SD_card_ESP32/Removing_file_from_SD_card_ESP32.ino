
/*

	SD card --> ESP32
	-----------------

 ** VCC (3V3) --> 3V3
 ** GND --> GND
 ** SCK --> GPIO 18   // (D18) = VSPI CLK
 ** MOSI --> GPIO 23  // (D23) = VSPI MOSI
 ** CS --> GPIO 5     // (D5)  = VSPI CSO
 ** MISO --> GPIO 19  // (D19) = VSPI MISO  (MOSO on my esp32)

====================================================================================================================================

In the methods of the SD library that we must pass as a parameter the name of the file we want to interact with,
methods like SD.open() , SD.exists() , SD.remove, we must NOT just pass its name as a string,

BUT, we must pass the "WHOLE PATH" of the file that it's located in a directory of the SD card

So,

If your file is in the root directory of the SD card (meaning the first folder that pops up when you open the SD card from your PC)
its path name will be --> /filename  and you pass it as a string like this --> "/filename.extension"

(That's where the backslash "/" comes in the programs you saw)


If it is in any other folder in your SD card, the path name will be --> "/foldername1/foldername2..../foldernameN/filename.extension"


NOTE: TO AVOID ANY ISSUES LATER --> ALWAYS have your files in your root "/" folder

====================================================================================================================================

DO NOT USE the "String" type to declare the paths of your file because it doen't work well with some functions

INSTEAD --> Use the " char* " to declare them

Example:

YES --> char* filePath == "/pay.csv"

NO --> String filePath == "/pay.csv"


=====================================================================================================================================


*/

#include "FS.h"
#include <SPI.h>
#include <SD.h>

#define SD_CS 5


char* filePath = "/data.csv";
//char* filePath = "/new.txt";


//-----------------------------------------------------

void setup() {

	// Open serial communications and wait for port to open:

	Serial.begin(115200);

	// Initializing SD card

	Serial.print("Initializing SD card...");

	if (!SD.begin(SD_CS)) {

		Serial.println("initialization failed!");

		return;

	}

	Serial.println("initialization done.");


	// We put this delay here just to catch on with the serial monitor, to be able to reset the board
  	// before the actual program starts, not to lose any execution that happened without as seeing it
  	// and think that there is problem with our code, but in reality it has just executed once and we miss it

  	delay(5000);



	// delete file

	deleteFile(SD, filePath);


}

//-----------------------------------------------------

void loop() {

	// nothing happens after setup finishes.
}

//-----------------------------------------------------

void deleteFile(fs::FS &fs, const char * path){

    Serial.printf("Deleting file: %s\n", path);

    if(fs.remove(path)){

        Serial.println("File deleted");

    } else {

        Serial.println("Delete failed");
    }
}

//-----------------------------------------------------
