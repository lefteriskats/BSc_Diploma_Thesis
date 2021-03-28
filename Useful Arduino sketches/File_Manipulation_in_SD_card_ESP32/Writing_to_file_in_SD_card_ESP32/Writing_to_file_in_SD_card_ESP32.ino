
/*
  SD card read/write
  This example shows how to read and write data to and from an SD card file


  SD card module attached to SPI bus of ESP32 as follows:


	SD card --> ESP32
	-----------------

 ** VCC (3V3) --> 3V3
 ** GND --> GND
 ** CS --> GPIO 5     // (D5)  = VSPI CSO
 ** MOSI --> GPIO 23  // (D23) = VSPI MOSI
 ** SCK --> GPIO 18   // (D18) = VSPI CLK
 ** MISO --> GPIO 19  // (D19) = VSPI MISO  (MOSO on my esp32)

Link1: https://github.com/esp8266/Arduino/blob/master/libraries/SD/examples/ReadWrite/ReadWrite.ino

Link2: https://steemit.com/utopian-io/@kimp0gi/create-write-read-and-delete-file-into-sd-card-using-sd-card-adapter-module-in-arduino


=====================================================================================================================================

In the methods of the SD library that we must pass as a parameter the name of the file we want to interact with,
methods like SD.open() , SD.exists() , SD.remove, we must NOT just pass its name as a string,

BUT, we must pass the "WHOLE PATH" of the file that it's located in a directory of the SD card

So,

If your file is in the root directory of the SD card (meaning the first folder that pops up when you open the SD card from your PC)
its path name will be --> /filename  and you pass it as a string like this --> "/filename.extension"

(That's where the backslash "/" comes in the programs you saw)


If it is in any other folder in your SD card, the path name will be --> "/foldername1/foldername2..../foldernameN/filename.extension"


NOTE: TO AVOID ANY ISSUES LATER --> ALWAYS have your files in your root "/" folder

=====================================================================================================================================

DO NOT USE the "String" type to declare the paths of your file because it doen't work well with some functions

INSTEAD --> Use the " char* " to declare them

Example:

YES --> char* filePath == "/pay.csv"

NO --> String filePath == "/pay.csv"


=====================================================================================================================================

*/

// Include the SD library to use the functions inside on it.
// And also the SPI to communicate the module to the Arduino Board

#include "FS.h"
#include "SPI.h"
#include "SD.h"

# define SD_CS 5

char* filePath = "/data.csv";


// just to wait a little before we write to file again

int delayTime = 3000;

String dataMessage;

// a simple counter

int counter = 0;


//--------------------------------------------------------------------------------------

void setup() {

	// Open serial communications and wait for port to open:

	Serial.begin(115200);

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


	// If the data.txt file doesn't exist

	// Create a file on the SD card and write the data labels

	File file = SD.open(filePath);

	if(!file) {

		Serial.println("File doens't exist");
		Serial.println("Creating file...");

		writeFile(SD, filePath, "Hello \r\n");

	}

	else {

		Serial.println("File already exists");
	}

	file.close();

}

//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------

void loop() {

    delay(delayTime);

    dataMessage = String(counter) + "\r\n";


    // We append to file
    //------------------

	Serial.print("Save data: ");
	Serial.println(dataMessage);

	appendFile(SD, filePath, dataMessage.c_str());

  counter += 1;

}

//--------------------------------------------------------------------------------------


// Write to the SD card (DON'T MODIFY THIS FUNCTION)
//--------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------


// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
//--------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------
