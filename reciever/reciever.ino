#include <SPI.h>            //BLUETOOTH
#include <nRF24L01.h>       //BLUETOOTH
#include <RF24.h>           //BLUETOOTH
//#include <EEPROM.h>         //ARDUINO MEMORY
#include <ArduinoSort.h>

RF24 radio(7, 8); // CE, CSN(Setting up the CE/CSN pins of the bluetooth module)

const byte address[6] = "87231"; //Sets up a frequency for which the bluetooth modules can communicate through

#include <LiquidCrystal.h>      //Library for LCD
LiquidCrystal lcd(10, 9, 5, 4, 3, 2);  //Configures the various LCD pins

//Current time will represent the actual live time of the race that will be displayed on the LCD
//Start time will be the moment in time(since the program started) when the race started
long currentTime = 0, startTime;

boolean started = false; //this boolean states whether the race has started or not(seperate from cross)

int IR_button; //Used to replace the lack of a first IR
char state = '0';

boolean cross = false; //variable to read the state of the first line(crossed or not)
boolean cross_2 = false; // varible to read the state of the second line

String array[] = {"EMPTY", "EMPTY", "EMPTY", "EMPTY"}; //string array of times
int array2[] = { -1, -1, -1, -1}; //int array of times
int count = 0; //array counter

int memory_sec, memory_milliSec; //These will be used to store the seconds and tenths of seconds in the EEPROM memory respectively
char text[32];
void setup() {
  // LCD:
  lcd.begin(16, 2);       //Configures LCD pins
  pinMode(6, INPUT);
  pinMode(13, INPUT);
  lcd.print("Time:");

  // BLUETOOTH:
  Serial.begin(9600);
  //Starts to "listen" to any incoming signals
  radioSet();
  pinMode(A0, OUTPUT);   //Configures IR pins
  digitalWrite(A0, LOW);
  reset();
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void radioSet() {
  radio.begin();                          //Starts bluetooth communication
  radio.openReadingPipe(0, address);      //Sets this bluetooth module to receiving mode
  radio.setPALevel(RF24_PA_MIN); //sets power level
  radio.startListening(); //starts listening to transmitter
}

void reset() { //resets logical components
  Serial.println("Reset Done.");
  state = '0';
  cross = false;
  started = false;
  cross_2 = false;
  lcd.clear();
  lcd.print("Time:");
  currentTime = 0;
  startTime = 0;
  for ( int i = 0; i < sizeof(text);  ++i ) {
    text[i] = (char)0;  //resets the transmitter package previously recieved
  }
  radio.powerDown();
  radio.powerUp();
  radioSet();
}

void hardReset() { //literally resets the hardware
  Serial.println("Hard Reset Done");
  delay(500);
  resetFunc();
}

void buzzerStart() { //beginning buzzer warning for start
  digitalWrite(A0, HIGH);
  delay(200);
  digitalWrite(A0, LOW);
  delay(200);
  digitalWrite(A0, HIGH);
  delay(200);
  digitalWrite(A0, LOW);
  delay(200);
  digitalWrite(A0, HIGH);
  delay(500);
  digitalWrite(A0, LOW);
}

void buzzerEnd() { //buzzer when sensor is triggered
  digitalWrite(A0, HIGH);
  delay(200);
  digitalWrite(A0, LOW);
}

void display() { //displays last 4 times
  Serial.println("Last 4 Times");
  for ( int i = 0; i < 4;  i++ ) {
    Serial.println(array[i]);
  }
  state = '0';

}

void first() { //shows first place time
  boolean found  = false;
  Serial.println("First Place");
  sortArray(array2, 4); //sorts the array using a library
  for (int i = 0; i < 4; i++) {
    if (array2[i] != -1) { //checks if spots are empty
      //  Serial.println(array2[i]);
      Serial.println(String((array2[i] / 1000)) + " Seconds and " + String(array2[i] % 1000) + " Milliseconds");
      found = true;
      break;
    }
  }
  if (!found) //if none recorded then output none
  {
    Serial.println("NO RECORDS");
  }  state = '0';
}
void loop() { 
  //digitalWrite(A0, HIGH);
  if (Serial.available() > 0) { // Checks whether data is comming from the serial port
    state = Serial.read();
    // Serial.print(state);
    // Reads the data from the serial port
  }
  IR_button = digitalRead(6);             //Reads the value of the button(first IR)
  if (state == '2') {   //checks the value recived of 2,3,4,5
    reset();
  }

  if (state == '3') {
    hardReset();

  }
  if (state == '4') {
    display();
    // break;
  }
  if (state == '5') {
    first();
    // break;
  }

  if (IR_button == HIGH || state == '1') { //checks whether the IR detects an object or not
    reset(); //soft resets
    buzzerStart(); //starts buzzer warning
    Serial.println("Started");
    cross = true;     //if object is detected, cross is true
    state = '0';
  } else {
    cross = false;         //if no object is detected, cross is false
  }

  if (cross && !started) {     //only run this loop once with the var <started>...gets the start time of the race
    startTime = millis(); 
    started = true;
  }

  //while the first line has been crossed and the second line hasn't been crossed
  //LCD STOPS THE TIME WHEN THE CONDITIONS ARE NO LONGER MET
  while (cross && !cross_2)
  {
    if (Serial.available() > 0) { // Checks whether data is comming from the serial port
      state = Serial.read();
      //  Serial.print(state);
      // Reads the data from the serial port
    }
    if (state == '2') { //checks the value recived of 2,3,4,5 while in the loop
      reset();
      break;
    }
    if (state == '3') {
      hardReset();

    }
    if (state == '4') {
      display();
      break;
    }
    if (state == '5') {
      first();
      break;
    }
    //  Serial.print("Go");
    currentTime = millis() - startTime;  //Gets the active time of the race(to display)
    lcd.setCursor(0, 1);                 //Sets the lcd cursor at column 0, row 1
    lcd.print(currentTime / 1000);       //Prints the SECONDS
    lcd.setCursor(4, 1);                 //Sets the lcd cursor at column 4, row 1
    lcd.print("sec");
    lcd.setCursor(8, 1);                 //Sets the lcd cursor at column 8, row 1
    lcd.print(currentTime % 1000);       //Prints the MILLISECONDS
    //  Serial.print("Go2");
    //Checks if the bluetooth is receiving an incoming message
    if (radio.available()) {
      //  char text[32];
      radio.read(&text, sizeof(text));

      //Checks if the bluetooth message matches with the message sent by the other bluetooth module
      //In this case, the message is "IR_2 CROSSED"
      // Serial.print((String) text);
      if ( (String) text == "IR_2_CROSSED")
      {
        buzzerEnd();
        cross_2 = true;     //Sets the second cross variable to true(I.E the second line has been crossed)
        Serial.println("Finished");
        Serial.print((currentTime / 1000));
        Serial.print(" Seconds and ");
        Serial.print((currentTime % 1000));
        Serial.print(" Milliseconds ");
        //array[count] = (currentTime / 1000);
        if (count < 4) { //checks if last value needs to be overwritten
          array[count] = String((currentTime / 1000)) + " Seconds and " + String((currentTime % 1000)) + " Milliseconds";
          array2[count] = currentTime;
          count++;
        } else { //deletes the last log and fills the first with lastest time
          for (int i = count - 1; i >= 0; i--) {
            array[i] = array[i - 1];
            array2[i] = array2[i - 1];
          }
          array[0] = String((currentTime / 1000)) + " Seconds and " + String((currentTime % 1000)) + " Milliseconds";
        }
        // (String)text = "NO";
        //EEPROM.write(0, (currentTime / 1000));            //Writes the saved time's *SECONDS* to memory address 0
        // EEPROM.write(1, (currentTime % 1000) / 10);       //Writes the saved time's *MILLISECONDS* to memory address 0

        //The following section of code can be used to test whether the EEPROM has actually saved the time to memory or not using Serial monitor
        /*
        memory_sec = EEPROM.read(0);
        memory_milliSec = EEPROM.read(1);

        Serial.println(memory_sec);
        Serial.println(memory_milliSec);
        */

        break;
      }
    }

  }
}

