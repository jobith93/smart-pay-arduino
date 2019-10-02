
/******************************************
  PURPOSE:  
  Created by Jobith M Basheer, Jijo Joseph & Chris Thomas
  DATE:   10/2017
*******************************************/

/*
   PIN Description
   Mega - Arduino
   RST  D49
   MISO D50
   MOSI D51
   SCK  D52
   SDA  D53  ss
*/

#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53                          //slave select pin
#define RST_PIN 49                         //reset pin
MFRC522 mfrc522(SS_PIN, RST_PIN);         //instatiate a MFRC522 reader object.
MFRC522::MIFARE_Key key;                  //create a MIFARE_Key struct named 'key', which will hold the card information


int block = 4;
byte readbackblock[18];


const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8, 9}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


String cmd;

bool isPAY = false;
bool isPIN = false;
bool isWAIT = false;

int lenPIN = 4;
int len = 0;
String pin = "";

int ledR = 11;
int ledY = 12;
int buzzer = 10;



void setup() {

  pinMode(ledR, OUTPUT);
  pinMode(ledY, OUTPUT);
  pinMode(buzzer, OUTPUT);

  sing(1);
  
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  digitalWrite(ledR, HIGH);
  digitalWrite(ledY, LOW);

  Serial.println("0_Device Ready!");

}

void loop() {
  
  // if ISPIn then collect PIn from user
  if(isPIN){
    blinkR(50);
    char key = keypad.getKey();
    if (key) {
      Serial.print("*");
      pin.concat(key);
      beep(200);
      len++;
    }
    // if collected length == 4 then stop and save PIN
    if(len==lenPIN){
      Serial.println("\n1_" + pin);
      isPIN = false;
      isPAY = false;
      len = 0;
      pin = "";
    }
  }

  
  mfrc522.PCD_Init();
  byte blockcontent[16] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

  if (Serial.available() > 0) {

    cmd = Serial.readString();

    if(cmd!="pay"){
      isPAY = false;
      return;
    }
    else{
      isPAY = true;
      isWAIT = true;
      Serial.print("4_Waiting for Card!\n");
    }

  }

  if(isPAY){
    if(isWAIT){
      blinkY(50);
    }
    
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
  
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    
    
    Serial.print("2_");
    readBlock(block, readbackblock);
    for (int j = 0 ; j < 16 ; j++)
    {
      Serial.write(readbackblock[j]);
    }
    Serial.print("\n");
    beep(500);
    isWAIT = false;
    acceptPIN();
  }
  

  delay(500);
  Serial.flush();

}



int readBlock(int blockNumber, byte arrayAddress[])
{
  int largestModulo4Number = blockNumber / 4 * 4;
  int trailerBlock = largestModulo4Number + 3;


  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return 3;
  }

  byte buffersize = 18;
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);
  if (status != MFRC522::STATUS_OK) {
    return 4;
  }
}

void acceptPIN(){
  isPIN = true;
  Serial.print("\n3_Enter 4-digit PIN: ");
}

void blinkY(int dur){
  digitalWrite(ledY, HIGH);
  delay(dur);
  digitalWrite(ledY, LOW);
}

void blinkR(int dur){
  digitalWrite(ledR, LOW);
  delay(dur);
  digitalWrite(ledR, HIGH);
}

void beep(int dur){
  digitalWrite(ledY, HIGH);
  digitalWrite(buzzer, HIGH);
  //tone(buzzer, freq); // Send 1KHz sound signal...
  delay(dur); 
  digitalWrite(ledY, LOW);  
  digitalWrite(buzzer, LOW);     
  //noTone(buzzer);     // Stop sound...
}


/////////////////////////////////// 
#define melodyPin 10

int melody[] = {  2637, 2637, 0, 2637, 0, 2637, 2637 };
int tempo[] = { 12, 12, 12, 12, 12, 12, 12 };

int song = 0;
 
void sing(int s) {
    // iterate over the notes of the melody:
    song = s;
  
    int size = sizeof(melody) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {
 
      // to calculate the note duration, take one second
      // divided by the note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000 / tempo[thisNote];
 
      buzz(melodyPin, melody[thisNote], noteDuration);
 
      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
 
      // stop the tone playing:
      buzz(melodyPin, 0, noteDuration);
 
    }
  
}

 
void buzz(int targetPin, long frequency, long length) {
  digitalWrite(13, HIGH);
  digitalWrite(ledR, HIGH);
  digitalWrite(ledY, HIGH);
  
  long delayValue = 1000000 / frequency / 2; // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length / 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to
  //// get the total number of cycles to produce
  for (long i = 0; i < numCycles; i++) { // for the calculated length of time...
    digitalWrite(targetPin, HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin, LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait again or the calculated delay value
  }
  digitalWrite(13, LOW);
  digitalWrite(ledR, LOW);
  digitalWrite(ledY, LOW);
}

