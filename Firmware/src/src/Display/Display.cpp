#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "Display.h"
#include "../Controller/InputManager.h"
#include "../Util/ConfigUtils.h"
#include "../../Config.h"
#include "../Main.h"
//#include "BatteryManager.h"



Display::Display() {
}

bool jeBT = false;
InputManager mainInputManager;
//Battery BatMan;
//Main main;



U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 32, 33);




void Display::setupDisplay(){
    u8g2.begin();

    /*u8g2.clearBuffer();					
    u8g2.setFont(u8g2_font_ncenB12_tr);	
    u8g2.drawStr(0,11,BTSERIAL_DEVICE_NAME);	
    u8g2.sendBuffer();*/
    vTaskDelay(1);
    vypis(1,11,0,"Pripraveno",BTSERIAL_DEVICE_NAME,1);

    delay(1000);
}

void Display::loopDisplay(int *prsty) {
  
  for (int i = 0; i < 5; i++){
    fingers[i] = prsty[i];
    vTaskDelay(1);
  }
  for (int i = 0; i < 5; i++){
    servo[i] = prsty[i+11];
    vTaskDelay(1);
  }
  //baterie = prsty[15];
  
  //delayMicroseconds(MULTIPLEXER_DELAY);
  /*u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(1, 19);
  u8g2.print(loops);
  if(loops > 1) {
    loops = loops - 1;
  } */
  Baterie();
}

void Display::Baterie() {

  baterie = mainInputManager.readMux(0);
  if (baterie >= 0 && baterie < 1499) {
    //ikBat = 0;
  } else if (baterie >= 1500 && baterie < 1799) {
    ikBat = 1;
  } else if (baterie >= 1800 && baterie < 2199) {
    ikBat = 2;
  } else if (baterie >= 2200 && baterie < 2499) {
    ikBat = 3;
  } else if (baterie >= 2500 && baterie < 2799) {
    ikBat = 4;
  } else if (baterie >= 2800 && baterie < 3049) {
    ikBat = 5;
  } else if (baterie >= 3050 && baterie < 4096) {
    ikBat = 6;
  }
  vTaskDelay(1);
  /*if (BatMan.isCharging()) {
    ikBat = 6;
  }*/
  //ikBat = 5;
  //u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_battery19_tn);
  u8g2.setCursor(120, 19);
  u8g2.print(ikBat);
  
  #if COMMUNICATION == COMM_BTSERIAL
    if(m_SerialBT.connected(1) && m_SerialBT.hasClient() && !jeBT){
      jeBT = true;
      u8g2.clearBuffer();
    } else {
      jeBT = false;
      loops = 2;
    }
  #endif
  vTaskDelay(1);
  if (jeBT && (loops == 1 || loops == 2)) { //jeBT
    u8g2.setFont(u8g2_font_battery19_tn);
    u8g2.setCursor(120, 19);
    u8g2.print(ikBat);

    u8g2.setFont(u8g2_font_siji_t_6x10);
    u8g2.drawGlyph(119, 31, 0xe00B);
    //int fingers[5] = {3056, 2150, 1920, 2400, 1500};
    
    //main.servoPos;
    //float servo[5] = {700, 700, 700, 700, 700};
    vTaskDelay(1);

    for (int i = 0; i < 5; i++){
      u8g2.drawFrame((5+(i*24)),3,10,29);
      
      u8g2.drawBox((6+(i*24)),(32-static_cast<int>((float)fingers[i]/4095*29)) ,8,static_cast<int>((float)fingers[i]/4095*29));
      
      u8g2.drawHLine((2+(i*24)),(32-static_cast<int>((float)servo[i]/1000*29)),  16 );
      vTaskDelay(1);
    }
    //int* fingers = Main::GetFingerPosi();
    //Main mainClass;
    //int prst = Main::fingerPosi[0];
    
  vTaskDelay(1);
  } else if (!jeBT && loops == 2) {		
    u8g2.clearBuffer();	
    u8g2.setFont(u8g2_font_ncenB10_tr);	
    u8g2.drawStr(1,11,"Pripraveno");	
    u8g2.drawStr(1,31,BTSERIAL_DEVICE_NAME);	
    loops = 1;
    Baterie();
  }

  u8g2.sendBuffer();
}

void Display::vypis(int x, int y, bool oneLine, char* text1, char* text2, int loop) {
  if(oneLine){
    u8g2.clearBuffer();					
    u8g2.setFont(u8g2_font_ncenB10_tr);	
    u8g2.drawStr(x,y,text1);
    loops = loop;	
    if(jeBT) {
      u8g2.setFont(u8g2_font_siji_t_6x10);
      u8g2.drawGlyph(119, 31, 0xe00B);
    }
    Baterie();
  } else {
    u8g2.clearBuffer();					
    u8g2.setFont(u8g2_font_ncenB10_tr);	
    u8g2.drawStr(x,y,text1);	
    u8g2.drawStr(x,y+21,text2);	
    loops = loop;
    if(jeBT) {
      u8g2.setFont(u8g2_font_siji_t_6x10);
      u8g2.drawGlyph(119, 31, 0xe00B);
    }
    Baterie();
  }
  
}

