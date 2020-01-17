// Bluetooth libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // Sets the CE, CSN pins

// Sets the address which the Bluetooth communicates on
const byte address[6] = "87231";

// Boolean to determine if the IR sensor has been crossed
boolean cross2 = false;

void setup() {
  // Sets serial port to enable serial printing
  Serial.begin(9600);
 
  // Debugging message
  Serial.println("PROGRAM SETUP");
 
  // Starts the radio transmitter, and opens a writing pipe to send information.
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  // IR Sensor Configuration:
  pinMode(A0,INPUT);
  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  digitalWrite(A2,HIGH);
  digitalWrite(A1,LOW);
}
 
void loop() {
  Serial.println("CHECKING FOR CROSS");
 
  if(analogRead(A0) < 250) // Determines if the IR sensor has been crossed
  {
	cross2 = true;     	// If an object has been detected, set the boolean varible to true
    
	// Sends a Bluetooth message with the text, "IR_2_CROSSED" over the writing pipe to the reciever Arduino
	const char text[] = "IR_2_CROSSED";
	radio.write(&text, sizeof(text));
    
	// Debugging messages
	Serial.println("IR 2 CROSSED SENDING MESSAGE");
	Serial.print("MESSAGE: ");
	Serial.println(text);
  }
}
