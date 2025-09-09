#define IR_SMALLD_NEC
#include <IRsmallDecoder.h>  //include IR library
#include <LiquidCrystal.h>   //include LCD library
#include <Stepper.h>         //include Stepper library
#include <MFRC522.h>         //include RFID library
#include <SPI.h>             ///Additional library for transfer of data

const int stepsPerRevolution = 2048;                  //number of steps
const int rolePerMinute = 15;                         //creates variable for motor speed
Stepper stepper(stepsPerRevolution, A2, A4, A3, A5);  //initialize stepper library as function and sets pins

IRsmallDecoder IRsensor(2);  //initialize IR library and sensor pin
irSmallD_t irData;           //function that stores IR sensor data

const int rs = 3, en = 4, d4 = 5, d5 = 6, d6 = 7, d7 = 8;  //declare variables for the pins of LCD screen
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);                 //create function for LCD library
int location = 0;
int L = 0;
String displayMessage = "";  //creates a string variable to store intended message
String lastDisplayed = "";   //creates a string variable to limit flickering and determine when to switch display
String ID = "";              //creates string to store UID of scanned card
bool accessGranted = false;  //boolean to determine state of access to system                                        //integer variable thats value is used to display position of motor

#define RST_PIN 9                // define RST pin
#define SDA_PIN 10               //define SDA pin
MFRC522 rfid(SDA_PIN, RST_PIN);  //creates RFID variable and sets pins
String UID = "D4 13 2D 00";      //Creates array with UID of access card
String UIDB = "4B 6F CE 01";     //Creates array with UID of access chip (blue chip)
bool cardPresent = true;         //boolean used to detect when a card is presented

void scannedUID() {
  ID = "";  //empties ID variable
  for (byte i = 0; i < rfid.uid.size; i++) {
    //for loop that adds each element of the scanned card to the ID variable in order

    ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    ID.concat(String(rfid.uid.uidByte[i], HEX));
  }
  ID.toUpperCase();  //makes sure that formatting of the elements in ID is correct
}

int Motion() {
  if (location >= 1024) {        //makes the variable circular
    location = location - 1024;  //constrains variable to below 1024
  }
  if (location <= 128 || location >= 896) {        //read position stepper
    displayMessage = "Hat 1";                      //change display variable
  } else if (128 < location && location <= 384) {  //read position stepper
    displayMessage = "Hat 2";                      //change display variable
  } else if (384 < location && location <= 640) {  //read position stepper
    displayMessage = "Hat 3";                      //change display variable
  } else if (640 < location && location < 896) {   //read position stepper
    displayMessage = "Hat 4";                      //change display variable
  }
  if (location >= 1024) {        //makes the variable circular
    location = location - 1024;  //constrains variable to below 1024
  }
  return location;
}
int Location() {
  if (L <= 128 || L >= 896) {        //read position stepper
    displayMessage = "Hat 1";        //change display variable
  } else if (128 < L && L <= 384) {  //read position stepper
    displayMessage = "Hat 2";        //change display variable
  } else if (384 < L && L <= 640) {  //read position stepper
    displayMessage = "Hat 3";        //change display variable
  } else if (640 < L && L < 896) {   //read position stepper
    displayMessage = "Hat 4";        //change display variable
  }
}
void setup() {
  SPI.begin();                      //SPI starts
  rfid.PCD_Init();                  //Initialize the RFID module
  stepper.setSpeed(rolePerMinute);  //sets speed of stepper motor
  lcd.begin(16, 2);                 //lights up LCD screen
  lcd.clear();                      //clears LCD
  lcd.print("Locked");              //prints message on first line
  Serial.begin(9600);               //initialize serial port
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    //if a new card is detected and it has been read
    if (cardPresent) {
      //if variable cardPresent is false
      scannedUID();  //run function scannedUID
      ID.trim();     //delete extra space from ID
      if (ID == UID || ID == UIDB) {
        //if the scanned card's UID is equal to one of the two accepted UIDs
        accessGranted = !accessGranted;  //flip accessGranted
        if (accessGranted == true) {
          //if accessGranted is true
          lcd.clear();             //clear LCD
          lcd.print("Unlocked!");  //print message on first line
          delay(2000);             //allow message to be read

          lcd.clear();                  //clear LCD
          lcd.print("Welcome to the");  //prints message on first line
          lcd.setCursor(0, 1);          //allows next message to print on second line
          lcd.print("Hat Holder!");     //prints message on second line
          delay(1000);                  //allows time for message to be read
          cardPresent = false;
        } else if (accessGranted == false) {
          //if accessGranted is false
          lcd.clear();          //clear LCD
          lcd.print("Locked");  //prints message on first line
          delay(100);           //delay to prevent flickering
        }
        rfid.PICC_HaltA();    //stops communication with RFID module
        cardPresent = false;  //sets cardPresent to false
      }
    }
  } else {
    cardPresent = true;  //resets cardPresent to true
  }
  if (accessGranted == true) {
    //if access is granted
    L = Motion();
    Location();
    if (displayMessage != lastDisplayed) {  //if the display variable changes
      lcd.clear();                          //Clear LCD
      lcd.setCursor(0, 0);                  //Ensure printing starts at the top-left corner
      lcd.print(displayMessage);            //Display the display variable
      lastDisplayed = displayMessage;       //set the lastdisplay variable equal to display variable
    }
  } else {
    //locked until accessGranted is true
    lcd.clear();
    lcd.print("Locked");
    lastDisplayed = "Locked";
  }
}