/*
 * This code need following libraries to be complied:
 * Servo
 * MFRC522
 * Sharp IR
 */

#include <SPI.h>
  
#include <MFRC522.h>

#include <Servo.h>

#include <SharpIR.h>

#define IRQ_PIN 2
#define LOCK_PIN 3
#define LED_PIN A1

volatile bool bNewInt = false;
volatile bool isLocked = false;
extern uint8_t SmallFont[];
byte regVal = 0x7F;
void activateRec(MFRC522 mfrc522);
void clearInt(MFRC522 mfrc522);

MFRC522 mfrc522(10, 9);                 //Create MFRC522 instance.
Servo myservo;                          //Create servo instance.

SharpIR sensor( SharpIR::GP2Y0A21YK0F, A0 ); //Analog distance reader.

unsigned char serNum[5];
  
void setup()
{
  Serial.begin(9600);
  myservo.attach(8);//Attach to the servomotor connected to PIN 8.
  myservo.write(0); //Initialize the servomotor to its original position.
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(IRQ_PIN, INPUT_PULLUP);
  pinMode(LOCK_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  //Use interrupt to disable or enable the system.
  attachInterrupt(digitalPinToInterrupt(LOCK_PIN),lock, FALLING);

  /* setup the IRQ pin*/
  pinMode(IRQ_PIN, INPUT_PULLUP);

  /*
   * Allow the ... irq to be propagated to the IRQ pin
   * For test purposes propagate the IdleIrq and loAlert
   */
  regVal = 0xA0; //rx irq
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, regVal);

  bNewInt = false; //interrupt flag
  /*Activate the interrupt*/
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readCard, FALLING);
  
  do { //clear a spourious interrupt at start
    ;
  } while (!bNewInt);
  bNewInt = false;
  Serial.println(F("End setup"));
}
  
void readCard() {
  //When card present, set it to true to tell the system to read the card.
  bNewInt = true;
}

void lock() {
  //Disable or enable the system when button is pressed.
  switch(isLocked) {
    case false:
      isLocked = true;
      break;
    case true:
      isLocked = false;
      break;
  }
}
  
void loop()
  
{
  if (bNewInt) {
    //A bug appears when below one sentence is removed, so I restrict it to active only on new present presence.
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (isLocked) {
      //If locked, blink the LED twice.
       int count = 0;
       while (count < 2) {
          Serial.println("Blocked");
          blinkLed();
          count++;
        }
        return;
    }
    int count = 0;
    //Read the distance 10 times, once the data is less than 30cm, read the card ID.
    while (count < 10) {
      int distance = sensor.getDistance();
      Serial.println(distance);
      if (distance < 30) break;
      count++;
      delay(500);
      if (count == 10) {
        return;
      }
    }
    
    mfrc522.PICC_ReadCardSerial(); //read the tag data
    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
    //Set the authorized ID here.
    if (mfrc522.uid.uidByte[0] == 57 || mfrc522.uid.uidByte[0] == 126 || mfrc522.uid.uidByte[0] == 29) {
      if(mfrc522.uid.uidByte[0] == 57 && mfrc522.uid.uidByte[1] == 160 && mfrc522.uid.uidByte[2] == 62 && mfrc522.uid.uidByte[3] == 183) {
        Serial.println("Welcome test 1");
        myservo.write(180); 
      }
      if(mfrc522.uid.uidByte[0] == 126 && mfrc522.uid.uidByte[1] == 148 && mfrc522.uid.uidByte[2] == 07 && mfrc522.uid.uidByte[3] == 83) {
        Serial.println("Welcome test 2");
        myservo.write(180); 
      }
       if(mfrc522.uid.uidByte[0] == 29 && mfrc522.uid.uidByte[1] == 182 && mfrc522.uid.uidByte[2] == 43 && mfrc522.uid.uidByte[3] == 216) {
        Serial.println("Welcome test 3");
        myservo.write(180); 
      }
      //Reset the servo motor to original position.
      delay(3000);
      myservo.write(0); 
      Serial.println("closed");
    }
    
    clearInt(mfrc522);
    mfrc522.PICC_HaltA();
    bNewInt = false;
  }

  // The receiving block needs regular retriggering (tell the tag it should transmit??)
  // (mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg,mfrc522.PICC_CMD_REQA);)
  activateRec(mfrc522);
  delay(100);
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i]);
  }
}

/*
 * The function sending to the MFRC522 the needed commands to activate the reception
 */
void activateRec(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
}

/*
 * The function to clear the pending interrupt bits after interrupt serving routine
 */
void clearInt(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
}

void blinkLed() {
  analogWrite(LED_PIN, 200);
  delay(500);
  analogWrite(LED_PIN, 0);
  delay(500);
}
 
