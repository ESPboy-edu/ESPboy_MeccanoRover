#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"

#include <ESP8266WiFi.h>
#include <espnow.h>

ESPboyInit myESPboy;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

enum enumRSTATUS{WAITING, JUSTSEND, FAILSEND};

struct structMess {
  uint8_t cmd=0;
} mess;

volatile enumRSTATUS sendState = JUSTSEND;
volatile uint32_t packetSnd = 0, packetFlt = 0;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  if (!sendStatus) {
    sendState = JUSTSEND; 
    packetSnd++;}
  else{
    sendState = FAILSEND;
    packetFlt++;}
}


void drawUI(){
  //myESPboy.tft.fillScreen(TFT_BLACK);
  myESPboy.tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  myESPboy.tft.setTextSize(1);
  String toPrint = "Send/fault " + (String)packetSnd + "/" +  (String)packetFlt;
  myESPboy.tft.drawString(toPrint, 0,0);
  myESPboy.tft.setTextSize(2);
  myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
  myESPboy.tft.setCursor(0,15);
  if(sendState == JUSTSEND){
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
    
    if(mess.cmd&PAD_ACT)myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
    else myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK); 
    myESPboy.tft.println("SOUND     ");
    
    if(mess.cmd&PAD_ESC)myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
    else myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK); 
    myESPboy.tft.println("LIGHTS    ");
  }
  else {
    myESPboy.tft.fillScreen(TFT_BLACK);
    myESPboy.tft.setTextColor(TFT_RED, TFT_BLACK);
    myESPboy.tft.println("NO CONNECT");}
};


void setup() {
  myESPboy.begin("Rover-remote");
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init()) {
    myESPboy.tft.setTextColor(TFT_RED, TFT_BLACK);
    myESPboy.tft.drawString("Error init ESP-NOW", 10, 120);
    while(1) delay(100);
  };
  
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

 
void loop() {
  if (sendState == JUSTSEND) {
    mess.cmd = myESPboy.getKeys();
    drawUI();
    sendState = WAITING;
    esp_now_send(broadcastAddress, (uint8_t *) &mess, sizeof(mess));
  }

    if (sendState == FAILSEND) {
      drawUI();
      sendState = WAITING;
    }

   delay(100);
}
