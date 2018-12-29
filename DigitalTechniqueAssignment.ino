#include <SPI.h>
  
#include <RFID.h>

#include <Servo.h>

#include <SharpIR.h>

#define OPEN_I_PIN 2
#define LOCK_PIN 3
#define LED_PIN A1
Servo myservo;
volatile bool bNewInt = false;
volatile bool isLocked = false;
extern uint8_t SmallFont[];
  
RFID     rfid(10,9);    //D10--接上SDA、D8--接RST  D13--接上SCK D11--接上MOSI D12--接上MISO RQ不接

SharpIR sensor( SharpIR::GP2Y0A21YK0F, A0 );

unsigned char serNum[5];
  
void setup()
{
  Serial.begin(9600);//串口用来读取需要添加的卡号，然后手动写到程序里
  myservo.attach(8);//舵机针脚位8
  myservo.write(0); //舵机初始化0度
  SPI.begin();
  rfid.init();
  pinMode(OPEN_I_PIN, INPUT_PULLUP);
  pinMode(LOCK_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(OPEN_I_PIN), readCard, FALLING);
  attachInterrupt(digitalPinToInterrupt(LOCK_PIN),lock, FALLING);
  Serial.println(F("End setup"));
}
  
void readCard() {
  bNewInt = true;
}

void lock() {
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
  Serial.println(isLocked);
  delay(500);
  
  if (bNewInt) {
    if (isLocked == 1) {
    int count = 0;
    while (count < 2) {
      Serial.println("Here");
      analogWrite(LED_PIN, 200);
      delay(500);
      analogWrite(LED_PIN, 0);
      delay(500);
      count++;
    }
    bNewInt = false;
    return;
  }
  int count = 0;
  while(count < 10) {
    int distance = sensor.getDistance();
    Serial.println(distance);
    if (distance < 30) break;
    count++;
    delay(500);
    if (count == 10) {
      bNewInt = false;
      return;
    }
  }
  long randNumber = random(0, 20);
  unsigned char i,tmp;
  unsigned char status;
  unsigned char str[MAX_LEN];
  unsigned char RC_size;
  count = 0;
  while (count < 100000) {
    //找卡
    rfid.isCard();
    //读卡号
    if (rfid.readCardSerial())
    {
      count = 10;
      Serial.print("your card id is   : ");
      Serial.print(rfid.serNum[0]);
      Serial.print(" , ");
      Serial.print(rfid.serNum[1],BIN);
      Serial.print(" , ");
      Serial.print(rfid.serNum[2],BIN);
      Serial.print(" , ");
      Serial.print(rfid.serNum[2],BIN);
      Serial.print(" , ");
      Serial.print(rfid.serNum[4],BIN);
      Serial.println(" ");
      //下面是改卡号区域
      if(rfid.serNum[0]==57||rfid.serNum[0]==126||rfid.serNum[0]==136||rfid.serNum[0]==161||rfid.serNum[0]==68){//第一道筛选
//        for(int i=0;i<100;i++)
//        {
          if(rfid.serNum[0]==57||rfid.serNum[0]==126||rfid.serNum[0]==155)//第二道筛选，支持一人一号
          {
            Serial.println("Welcome test 1");
            myservo.write(180); 
          }
          if(rfid.serNum[0]==148||rfid.serNum[0]==68||rfid.serNum[0]==161)//rfid.serNum[0]==161 这个0是id位置，可以自行更改
          {
            Serial.println("Welcome test 2");
            myservo.write(180); 
          }
          if(rfid.serNum[0]==136)
          {
            Serial.println("Welcome test 3");
            myservo.write(180);
          }
//        }     
        delay(3000);
        myservo.write(0); 
        Serial.println("closed");
      }
      break;
    }  
    if (!rfid.readCardSerial()){
      }
      count++;
    }
  bNewInt = false;
  rfid.halt(); //休眠
  }
}
  
 
