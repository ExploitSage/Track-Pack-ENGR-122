/*
 *	Engineering 122 Freshman Design Project Client
 *	Copyright (C) 2014	Gustave Abel Michel III, Joey Higuera, 
 *	Jordan Lofton, and Nathan Slaughter
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

#define DEBUG true

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
// Watchdog Timeout
	#include <avr/wdt.h> //WatchDog

// Contant Declarations
#define USER true
#define DISH false
#define KEY "password1"

#define DIRECTION_PIN 12
#define PWM_PIN 3
#define BRAKE_PIN 9
#define CURRENT_PIN 0 //analog
#define CLOCKWISE false
#define CCLOCKWISE true

#define SPEED 85 //0-255
#define NUM_AVG 1
#define DEGREE_ERROR 15

#define ADJUST_ANGLE 55
#define NEGATIVE_ADJUST 55
#define POSITIVE_ADJUST 305

//char server[] = "wifi.gustavemichel.com";
//int = port 80;
char server[] = "gustave.me";
int port = 4500;
//IPAddress server(192,168,0,2);
//int port = 4500;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // 0xDEADBEEFFEED because WHY NOT!
IPAddress ip(192,168,0,3); //default

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
const unsigned long postingInterval = 4000;		// delay between updates, in milliseconds

void setup() {
	//Start Serial
	Serial.begin(9600);

	//Setup Motor Shield
	pinMode(DIRECTION_PIN, OUTPUT);
	digitalWrite(DIRECTION_PIN, CLOCKWISE);
	pinMode(BRAKE_PIN, OUTPUT);
	digitalWrite(BRAKE_PIN, LOW);
	pinMode(PWM_PIN, OUTPUT);
	analogWrite(PWM_PIN, 0);
	

	//Start Compass
	Wire.begin();
	compass = HMC5883L(); 
	compass.SetScale(1.3); 
	compass.SetMeasurementMode(Measurement_Continuous);

	// give the ethernet module time to boot up:
	delay(1000);
	Serial.println("Start!");
	// start the Ethernet connection using a fixed IP address:
	if (Ethernet.begin(mac) == 0) { //attempt DHCP
		Serial.println("Failed to configure Ethernet using DHCP");
		Ethernet.begin(mac, ip);
	}
	// print the Ethernet board/shield's IP address:
	Serial.print("My IP address: ");
	Serial.println(Ethernet.localIP());

	birthWatchdog(WDTO_4S);
}

void loop() {
	//Is not connected and is time to reconnect
	if((!client.connected()) && (millis() - lastConnectionTime > postingInterval)) {
		if(httpRequest()) { //success
			bool con = true;
			double mag_angle = 0;
			
			double target_angle = (atan2(user_coords.lat - dish_coords.lat,user_coords.lon - dish_coords.lon))*(180/M_PI);
			if(target_angle < 0) {
				target_angle+=360;
			}

			do {
				//Update Magetometer Values
				mag_angle = 0;
				for(int i = 0; i < NUM_AVG; i++) { //Average 3 reads
					readMagnetometer();
					mag_angle+=yDegrees;
					delay(20); //don't read too fast?
				}
				mag_angle /= NUM_AVG;
				//*
				if(mag_angle > ADJUST_ANGLE)
					mag_angle -= NEGATIVE_ADJUST;
				else
					mag_angle += POSITIVE_ADJUST;
				//*/
				if(DEBUG) {
					Serial.print("Mag Angle: ");
					Serial.println(yDegrees,10);
					Serial.print("Cor Angle: ");
					Serial.println(mag_angle,10);
					Serial.print("Pos Angle: ");
					Serial.println(target_angle,10);
					//Serial.println();
				}

				//*
				if(DEBUG)
					Serial.print("Motor: ");

				if(target_angle > mag_angle && (target_angle - mag_angle) > DEGREE_ERROR) {
					con = true;
					if(DEBUG)
						Serial.println("CounterClockwise");
					digitalWrite(BRAKE_PIN, false);
					digitalWrite(DIRECTION_PIN, CCLOCKWISE);
					analogWrite(PWM_PIN, SPEED);
				} else if(mag_angle > target_angle && (mag_angle - target_angle) > DEGREE_ERROR) {
					con = true;
					if(DEBUG)
						Serial.println("Clockwise");
					digitalWrite(BRAKE_PIN, false);
					digitalWrite(DIRECTION_PIN, CLOCKWISE);
					analogWrite(PWM_PIN, SPEED);
				} else {
					con = false;
					if(DEBUG)
						Serial.println("Stopped");
					analogWrite(PWM_PIN, 0);
					digitalWrite(BRAKE_PIN, true);
				}

				if(DEBUG) {
					Serial.print("Current: ");
					Serial.println(analogRead(CURRENT_PIN));
				}
				//*/
				feedWatchdog();
			}while(con);
		}
	} else {
		if(client.connected()) {
			client.stop();
		}
	}
	feedWatchdog();
}


// this function makes a HTTP connection to the server and parses the json it receives:
int httpRequest() {
	if(DEBUG)
		Serial.println("connecting.");
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
		if(DEBUG) {
			Serial.println("connection failed.");
			Serial.println("disconnecting.");
		}
		client.stop();
		return(0);
	}
	delay(100);
	if(client.available()) {
			parseJson(DISH); //Parse
			Serial.println("disconnecting.");
			client.stop();
	}

	if(DEBUG)
		Serial.println("connecting.");
	if (client.connect(server, port)) { // Successful connection?
		// note the time that the connection was made:
		lastConnectionTime = millis();

		// send the HTTP PUT request:
		client.println("GET /api/user/password1");
		client.println("Host: 10.10.10.2");
		client.println("User-Agent: arduino-ethernet");
		client.println("Connection: close");
		client.println();
	} else { // No connection... :(
		if(DEBUG) {
			Serial.println("connection failed.");
			Serial.println("disconnecting.");
		}
		client.stop();
		return(0);
	}
	delay(100);
	if(client.available()) {
		parseJson(USER); //Parse
		if(DEBUG)
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

void birthWatchdog(uint16_t timeout)
	{wdt_enable(timeout);}
void killWatchdog()
	{wdt_disable();}
void feedWatchdog()
	{wdt_reset();}