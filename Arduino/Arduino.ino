/*
 *	Engineering 122 Freshman Design Project Client
 *	Copyright (C) 2014	Gustave Abel Michel III
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// Libraries
//SPI
	#include "SPI.h"
//I2C
	#include <Wire.h>
//Ethernet
	#include "Ethernet.h"
//Servo
	#include "Servo.h"
// GY-217 Compass
	#include "HMC5883L.h"
// JSON Parser

// Contant Declarations
#define CW_Switch 2
#define CCW_Switch 3
#define USER true
#define DISH false
#define key "password1"

//char server[] = "wifi.gustavemichel.com";
// int port 80
//char server[] = "gustave.me";
// int port 4500
IPAddress server(10,10,10,1);
int port = 4500;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // 0xDEADBEEFFEED because WHY NOT!
IPAddress ip(10,10,10,2); //default

// Variable/Object Declarations
struct Coords {
	double lat;
	double lon;
} dish_coords, user_coords;
struct Heading {
	double radians;
	double degrees;
};
EthernetClient client;
HMC5883L compass;
double xDegrees, yDegrees, zDegrees;


unsigned long lastConnectionTime = 0;			// last time you connected to the server, in milliseconds
boolean lastConnected = false;					// state of the connection last time through the main loop
const unsigned long postingInterval = 6000;		// delay between updates, in milliseconds

void setup() {
	//Start Serial
	Serial.begin(9600);

	//Start Limit Switches
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);
	pinMode(CW_Switch, INPUT_PULLUP);
	pinMode(CCW_Switch, INPUT_PULLUP);
	attachInterrupt(0, CWSwitch, FALLING);
	attachInterrupt(1, CCWSwitch, FALLING);

	//Start Compass
	Wire.begin();
	compass = HMC5883L(); 
	compass.SetScale(1.3); 
	compass.SetMeasurementMode(Measurement_Continuous);

	// give the ethernet module time to boot up:
	delay(1000);
	// start the Ethernet connection using a fixed IP address:
//	if (Ethernet.begin(mac) == 0) { //attempt DHCP
		Serial.println("Failed to configure Ethernet using DHCP");
		Ethernet.begin(mac, ip);
//	}
	// print the Ethernet board/shield's IP address:
	Serial.print("My IP address: ");
	Serial.println(Ethernet.localIP());

}

void loop() {
	//Is not connected and is time to reconnect
	if((!client.connected()) && (millis() - lastConnectionTime > postingInterval)) {
		if(httpRequest()) { //success
			//Set angle
			readMagnetometer();		//Update Magetometer Values
			double corrected = 0;
			if(yDegrees < 270)
				corrected = yDegrees+90;
			else {
				corrected = yDegrees-270;
			}

			double angle = (atan2(user_coords.lat - dish_coords.lat,user_coords.lon - dish_coords.lon))*(180/M_PI);
			if(angle < 0) {
				angle+=360;
			}
			Serial.print("Mag Angle: ");
			Serial.println(yDegrees,10);
			Serial.print("Cor Angle: ");
			Serial.println(corrected,10);
			Serial.print("Pos Angle: ");
			Serial.println(angle,10);
		}

	}

	readMagnetometer();		//Update Magetometer Values
	delay(1000);
}


// this function makes a HTTP connection to the server and parses the json it receives:
int httpRequest() {
	Serial.println("connecting...");
	if (client.connect(server, port)) { // Successful connection?
		// note the time that the connection was made:
		lastConnectionTime = millis();

		// send the HTTP PUT request:
		client.println("GET /api/dish/password1");
		client.println("Host: 10.10.10.2");
		client.println("User-Agent: arduino-ethernet");
		client.println("Connection: close");
		client.println();
	} else { // No connection... :(
		Serial.println("connection failed");
		Serial.println("disconnecting.");
		client.stop();
		return(0);
	}
	delay(100);
	if(client.available()) {
			parseJson(DISH); //Parse
			Serial.println("disconnecting.");
			client.stop();
	}

	Serial.println("connecting...");
	if (client.connect(server, port)) { // Successful connection?
		// note the time that the connection was made:
		lastConnectionTime = millis();
		Serial.println("connecting...");

		// send the HTTP PUT request:
		client.println("GET /api/user/password1");
		client.println("Host: 10.10.10.2");
		client.println("User-Agent: arduino-ethernet");
		client.println("Connection: close");
		client.println();
	} else { // No connection... :(
		Serial.println("connection failed");
		Serial.println("disconnecting.");
		client.stop();
		return(0);
	}
	delay(100);
	if(client.available()) {
		parseJson(USER); //Parse
		Serial.println("disconnecting.");
		client.stop();
	}

	return(1);
}

void parseJson(bool user) {
	bool foundJson = false;
	int stringPos = 0; // string index counter
	char inString[51]; // string for incoming serial data
	memset( &inString, 0, 51 ); //clear inString memory

	while(client.available()) {
		char c = client.read();
		if(c == '{')
			foundJson = true;
		if(foundJson) {
			inString[stringPos] = c;
			stringPos++;
		}
		if(c == '}')
			break;
	}

	String RawJSON = String(inString);
	char firstNumber[20];
	RawJSON.substring(RawJSON.indexOf(':')+1,RawJSON.indexOf(',')).toCharArray(firstNumber, 20);
	char secondNumber[20];
	RawJSON.substring(RawJSON.indexOf(':',RawJSON.indexOf(':')+1)+1,RawJSON.indexOf('}')).toCharArray(secondNumber, 20);

	if(user == DISH) {
		dish_coords.lat = (double)atof(firstNumber);
		dish_coords.lon = (double)atof(secondNumber);
	}
	if(user == USER) {
		user_coords.lat = (double)atof(firstNumber);
		user_coords.lon = (double)atof(secondNumber);
	}
}
void readMagnetometer() {
	MagnetometerRaw raw = compass.ReadRawAxis(); //Read Data
	MagnetometerScaled scaled = compass.ReadScaledAxis(); 
	double xHeading = atan2(scaled.YAxis, scaled.XAxis); //Calculate Heading
	double yHeading = atan2(scaled.ZAxis, scaled.XAxis); 
	double zHeading = atan2(scaled.ZAxis, scaled.YAxis); 
	if(xHeading < 0) 	//for radians conversion?
		xHeading += 2*PI;
	if(xHeading > 2*PI) 
		xHeading -= 2*PI; 
	if(yHeading < 0) 
		yHeading += 2*PI; 
	if(yHeading > 2*PI) 
		yHeading -= 2*PI; 
	if(zHeading < 0) 
		zHeading += 2*PI; 
	if(zHeading > 2*PI) 
		zHeading -= 2*PI; 

	xDegrees = xHeading * 180/M_PI; //Convert radians to degrees
	yDegrees = yHeading * 180/M_PI; 
	zDegrees = zHeading * 180/M_PI; 
}

//Direction Switch Functions (To prevent wire tangle)
void CWSwitch() {
	digitalWrite(13, HIGH);
}

void CCWSwitch() {
	digitalWrite(13, LOW);
}
