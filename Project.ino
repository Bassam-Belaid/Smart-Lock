#define BLYNK_TEMPLATE_ID "TMPL2fczyCYOI"
#define BLYNK_TEMPLATE_NAME "Smart Access Control System"
#define BLYNK_AUTH_TOKEN "0r6eQTGSzjRO_C2y_UE125KcLqK_jf-o"

#define BLYNK_PRINT Serial

#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

const char* ssid = "LTT-4G";
const char* password = "Myphone1271";

#define RST_PIN D2      
#define SS_PIN  D10 

#define Red D8
#define Speaker D8
#define Green D9

int RedValue, GreenValue, AddValue;

int Counter = 0;
String Name = "";
bool IsMessageSented = false;
bool IsMessagePrinted = false;
bool IsQuickAccess = false;
bool IsAddMode = false;
String Message = "";

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

String ReadUserUID() {

String UserUID;

for (byte i = 0; i < mfrc522.uid.size; i++)  {

  UserUID += String(mfrc522.uid.uidByte[i], HEX);

}


return UserUID;

}

bool IsBlocked() {

     return (Counter == 3);

}

void Access(String Name) {
    
    lcd.print("Welcome " + Name); 
    digitalWrite(Green, HIGH);

    delay(2000);

    digitalWrite(Green, LOW);

}

void Denied() {

    lcd.print("Access Denied");  
    digitalWrite(Red, HIGH);
    
    delay(1000);

    digitalWrite(Red, LOW);

    Counter = Counter + 1;

}



BLYNK_WRITE(V0)

{

  RedValue = param.asInt();   

}

void SetAlarm() {

  digitalWrite(Red, HIGH);
  delay(100);
  digitalWrite(Red, LOW);
  delay(100);

}

void Block() {

    Serial.println("take image");
    IsMessageSented = true;

    Blynk.virtualWrite(V2, "The System Is Locked");

    lcd.clear();  
    lcd.print("You Are Blocked");  
    
}

void UnBlock() {

   digitalWrite(Red, LOW);
   Counter = 0;
   RedValue = 0;
   IsMessageSented = false;
   Blynk.virtualWrite(V2, "The System Is UnLocked");
   Blynk.virtualWrite(V0, 0);

}

void QuickAceess() {

    lcd.clear();   
    lcd.print("Access Granted"); 
    digitalWrite(Green, HIGH);
   
}

void ResetSystem() {
    
    digitalWrite(Green, LOW);
    GreenValue = 0;
    IsMessagePrinted = false;

}

BLYNK_WRITE(V1)

{

  GreenValue = param.asInt();   
  
  if(GreenValue == 1) {

     IsQuickAccess = true;
     QuickAceess();

  }

   else {

     IsQuickAccess = false;
     ResetSystem();

   }

}

void Add() {

    Serial.println("add new");
    Serial.println(ReadUserUID());

    while(Message == "") {
  
      if (Serial.available() > 0) 
        Message = Serial.readStringUntil('\n');

    }
  
}

BLYNK_WRITE(V3)

{

  AddValue = param.asInt();  
   
   if(AddValue == 1)
      IsAddMode = true;

}

void setup() {

  Serial.begin(9600);

  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Speaker, OUTPUT); 
  
  lcd.init();     
  lcd.backlight();
  lcd.clear();   
  lcd.setCursor(0, 0);
  
  SPI.begin();		
  mfrc522.PCD_Init();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);


}

void loop() {

 Blynk.run();

if(IsQuickAccess)
   return;

 if(IsBlocked()) {

 if(!IsMessageSented)
    Block();

    if(RedValue != 1) {

       SetAlarm();
       return;

    }

    UnBlock();
    
  }
  
   if (!IsMessagePrinted) {

    lcd.clear();
    lcd.print("Put Your Card");
    IsMessagePrinted = true;

  }

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

if(IsAddMode) {

     Add();

     Message = "";
     AddValue = 0;
     IsAddMode = false;
     Blynk.virtualWrite(V3, 0);

     return;

}

    Name = "";
    Serial.println(ReadUserUID());

    delay(100);

    if (Serial.available() > 0) 
        Name = Serial.readStringUntil('\n');

    lcd.clear();
    IsMessagePrinted = false;

  if (Name != "")
      Access(Name);

  else
      Denied();

}
