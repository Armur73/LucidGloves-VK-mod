#pragma once

#include <mutex>
#include "Controller/Gesture.h"
#include "Controller/Haptics.h"
#include "../AdvancedConfig.h"
#include "Communication/ICommunication.h"
#include "Encoding/IEncoding.h"
#include "Util/ConfigUtils.h"
#include "Controller/InputManager.h"
//#include "Display/Display.h"


class Main {
public:
  Main();
  void setup();
  void loop();
  int servoPos[5] = {0,0,0,0,0};
  int fingerPosi[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  //static int* GetFingerPosi();


  #if ESP32_DUAL_CORE_SET
  static void getInputsWrapper(void* _this);
  #endif

private:

  ICommunication* comm;
  IEncoding* encoding;
  Haptics haptics;
  Gesture gesture;
  InputManager input;

  
  
  OutboundData data = {};

  bool calibrate = false;
  int fingerPos[10] = {0,0,0,0,0,0,0,0,0,0};
  int loops = 0;

  #if ESP32_DUAL_CORE_SET
  void getInputs();
  static void OLEDloop(void*);
  ordered_lock* fingerPosLock = new ordered_lock();
  TaskHandle_t Task1;
  TaskHandle_t Task2;
  int threadLoops = 1;
  int threadLoopsD = 1;
  #endif

};
