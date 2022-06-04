#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"
#include "WEMOS_Motor.h"

#include <ESP8266WiFi.h>
#include <espnow.h>

#define TIMEOUT 200

ESPboyInit myESPboy;

Motor M1(0x30,_MOTOR_A, 1000);
Motor M2(0x30,_MOTOR_B, 1000);

enum enumRSTATUS{WAITINGPACKET, JUSTRECEIVED};

struct structMess {
  uint8_t cmd;
} mess;


volatile enumRSTATUS recieveState = WAITINGPACKET;
volatile uint32_t packetRcv = 0, timeOutCnt = 0;


void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&mess, incomingData, sizeof(mess));
  recieveState = JUSTRECEIVED;
  packetRcv++;
};


void setMotors(){
 
  if(!mess.cmd){
    M1.setmotor(_STOP);
    M2.setmotor( _STOP);     
  }

  if(mess.cmd&PAD_UP){
    M1.setmotor(_CCW, 100);
    M2.setmotor(_CCW, 100);
  }

  if(mess.cmd&PAD_DOWN){
    M1.setmotor(_CW, 100);
    M2.setmotor(_CW, 100);
  }

  if(mess.cmd&PAD_RIGHT){
    M1.setmotor(_CW, 50);
    M2.setmotor(_CCW,50);  
  }

  if(mess.cmd&PAD_LEFT){
    M1.setmotor(_CCW, 50);  
    M2.setmotor(_CW, 50);
  }
    
};


void drawUI(){
  static uint32_t prevTimeOutCnt = 0;
  static bool lightsFlag = false;

  myESPboy.tft.setTextSize(2);
  myESPboy.tft.setCursor(0,15);
  if(recieveState == JUSTRECEIVED && prevTimeOutCnt == timeOutCnt){
    
    if(mess.cmd) myESPboy.myLED.setRGB(0,30,0);
    else myESPboy.myLED.setRGB(0,0,30);
    
    if(!mess.cmd)myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
    else myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK);    
    myESPboy.tft.println("STOP      ");
    
    if(mess.cmd&PAD_UP) myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
    else myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK);
    myESPboy.tft.println("FORWARD   ");
    
    if(mess.cmd&PAD_DOWN) myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
    else myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK);
    myESPboy.tft.println("BACKWARD  ");
    
    if(mess.cmd&PAD_LEFT) myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
    else myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK);
    myESPboy.tft.println("LEFT      ");
    
    if(mess.cmd&PAD_RIGHT)myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
    else myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK);     
    myESPboy.tft.println("RIGHT     ");
    
    if(mess.cmd&PAD_ACT){
      myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
      myESPboy.mcp.digitalWrite(15, LOW);}
    else {
      myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK);
      myESPboy.mcp.digitalWrite(15, HIGH);} 
    myESPboy.tft.println("SOUND     ");
    
    if(mess.cmd&PAD_ESC){
      myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
      if(!lightsFlag){
        lightsFlag = true;
        myESPboy.mcp.digitalWrite(14, LOW);
        myESPboy.mcp.digitalWrite(13, LOW);
        }
      else {
          lightsFlag = false;
          myESPboy.mcp.digitalWrite(14, HIGH);
          myESPboy.mcp.digitalWrite(13, HIGH);}
      while(myESPboy.getKeys()) delay(100);
    }
    else{ 
      if(!lightsFlag)myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK);
      else myESPboy.tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    }
    myESPboy.tft.println("LIGHTS    ");
    
  }
  else {
    myESPboy.tft.fillScreen(TFT_BLACK);
    myESPboy.tft.setTextColor(TFT_RED, TFT_BLACK);
    myESPboy.tft.println("NO CONNECT");
    myESPboy.myLED.setRGB(30,0,0);
  }

    
  myESPboy.tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  myESPboy.tft.setTextSize(1);
  String toPrint = "Recieve/fault " + (String)packetRcv + "/" +  (String)timeOutCnt;
  myESPboy.tft.drawString(toPrint, 0,0);
  prevTimeOutCnt = timeOutCnt;
  
};


 
void setup() {
  myESPboy.begin("Rover-rover");

  for (int i=9;i<16;i++){  
     myESPboy.mcp.pinMode(i, OUTPUT);
     myESPboy.mcp.digitalWrite(i, HIGH);}
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init()) {
    myESPboy.tft.setTextColor(TFT_RED, TFT_BLACK);
    myESPboy.tft.drawString("Error init ESP-NOW", 10, 120);
    while(1) delay(100);
  };

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  M1.setmotor(_STANDBY);
  M2.setmotor(_STANDBY);
};


void loop() {
  static uint32_t timeoutcnt = millis();

  if(recieveState == JUSTRECEIVED){
    drawUI();
    setMotors();
    recieveState = WAITINGPACKET; 
    timeoutcnt = millis(); 
  }

  if((millis() - timeoutcnt) > TIMEOUT){
    timeOutCnt++;
    mess.cmd = 0;
    drawUI();
    timeoutcnt = millis(); 
  }
  
  delay(50);
};
