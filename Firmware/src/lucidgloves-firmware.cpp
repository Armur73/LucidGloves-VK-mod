/*
 * LucidGloves Firmware Version 4
 * Author: Lucas_VRTech - LucidVR
 * lucidvrtech.com
 */


#include "src/Main.h"
//#include "src/Display/Display.h"
//#include "src/Display/BatteryManager.h"
/*
 * 
 *  THE CONFIG SETTINGS ARE NO LONGER STORED HERE. THEY HAVE BEEN MOVED TO Config.h
 *  
 *  You can now change the settings over at config.h before uploading to your gloves.
 * 
 */


Main mainClass;
//Display mainDisplay;
//Battery BatMan;

void setup(){
  mainClass.setup();
  //mainDisplay.setupDisplay();
  //BatMan.initBatt();

}

void loop(){
  mainClass.loop();
  //mainDisplay.Baterie();
  //BatMan.isError();
}
