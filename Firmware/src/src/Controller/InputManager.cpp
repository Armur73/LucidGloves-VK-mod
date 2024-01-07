#include "InputManager.h"
#include "../Display/Display.h"

InputManager::InputManager() {
    // Constructor logic here...
}


Display displayOLED;

#define ANALOG_MAX 4095

void InputManager::setupInputs() {
    
  EEPROM.begin(0x78 + 1);
  if (isSavedLimits()){
    savedTravel = true;
    loadTravel();
  }
  if (isSavedIntermediate()){
    savedInter = true;
    loadIntermediate();
  }

  pinMode(PIN_JOY_BTN, INPUT_PULLUP);
  pinMode(PIN_A_BTN, INPUT_PULLUP);
  pinMode(PIN_B_BTN, INPUT_PULLUP);

  pinMode(PIN_MENU_BTN, INPUT_PULLUP);
  
  #if !TRIGGER_GESTURE
  pinMode(PIN_TRIG_BTN, INPUT_PULLUP);
  #endif

  #if !GRAB_GESTURE
  pinMode(PIN_GRAB_BTN, INPUT_PULLUP);
  #endif

  #if !PINCH_GESTURE
  pinMode(PIN_PNCH_BTN, INPUT_PULLUP);
  #endif

  #if USING_CALIB_PIN
  pinMode(PIN_CALIB, INPUT_PULLUP);
  #endif

  #if USING_MULTIPLEXER
  //pinMode(MUX_INPUT, INPUT);
  {
    byte selectPins[] = {PINS_MUX_SELECT};
    for (int i = 0; i < sizeof(selectPins); i++){
      pinMode(selectPins[i], OUTPUT);
    }
  }
  #endif
}

int InputManager::analogPinRead(int pin) {
  #if USING_MULTIPLEXER
  if (ISMUX(pin)){
    return readMux(UNMUX(pin));
  }
  else{
    return analogRead(pin);
  }
  #else
   return analogRead(UNMUX(pin));
  #endif
}

// Other methods follow the same pattern...

#if USING_MULTIPLEXER
int InputManager::readMux(byte pin) {
  byte selectPins[] = {PINS_MUX_SELECT};
  int numSelectPins = sizeof(selectPins) / sizeof(selectPins[0]);
  /*if(pin == byte(99)){
    for (int i = numSelectPins - 1; i > -1; i--){
      digitalWrite(selectPins[i], LOW);
    }
  } else {*/
    for (int i = numSelectPins - 1; i > -1; i--){
      digitalWrite(selectPins[i], (pin & (1 << i)) ? HIGH : LOW);
    }
  //}
  delayMicroseconds(MULTIPLEXER_DELAY);
  
  /*Serial.print(digitalRead(selectPins[0]));
  Serial.print(digitalRead(selectPins[1]));
  Serial.print(digitalRead(selectPins[2]));
  Serial.print(digitalRead(selectPins[3]));
  Serial.println(analogRead(MUX_INPUT));*/
  return analogRead(MUX_INPUT);
}
#endif

bool bylRST = false;
int bttnPRS = 0;

void InputManager::getFingerPositions(bool calibrating, bool reset, int* fingerPos) {
  #if FLEXION_MIXING == MIXING_NONE //no mixing, just linear
  int rawFingersFlexion[NUM_FINGERS] = {NO_THUMB?0:analogPinRead(PIN_THUMB), analogPinRead(PIN_INDEX), analogPinRead(PIN_MIDDLE), analogPinRead(PIN_RING), analogPinRead(PIN_PINKY)};
  
  #elif FLEXION_MIXING == MIXING_SINCOS
  int rawFingersFlexion[NUM_FINGERS] = {NO_THUMB?0:sinCosMix(PIN_THUMB, PIN_THUMB_SECOND, 0 ), 
                                  sinCosMix(PIN_INDEX, PIN_INDEX_SECOND, 1 ), 
                                  sinCosMix(PIN_MIDDLE,PIN_MIDDLE_SECOND,2 ), 
                                  sinCosMix(PIN_RING,  PIN_RING_SECOND,  3 ), 
                                  sinCosMix(PIN_PINKY, PIN_PINKY_SECOND, 4 )};

  #endif

  int rawFingers[2 * NUM_FINGERS];

  #if USING_SPLAY
    int rawFingersSplay[NUM_FINGERS] = {NO_THUMB?0:analogPinRead(PIN_THUMB_SPLAY), 
                              analogPinRead(PIN_INDEX_SPLAY), 
                              analogPinRead(PIN_MIDDLE_SPLAY), 
                              analogPinRead(PIN_RING_SPLAY), 
                              analogPinRead(PIN_PINKY_SPLAY)};
  #else
    int rawFingersSplay[NUM_FINGERS] = {0,0,0,0,0};
  #endif
    //memcpy(rawFingers, rawFingersFlexion, 5); //memcpy doesn't seem to work here
    //memcpy(&rawFingers[5], rawFingersSplay, 5); 

  for (int i = 0; i < NUM_FINGERS; i++){
    rawFingers[i] = rawFingersFlexion[i];
    rawFingers[i+NUM_FINGERS] = rawFingersSplay[i];
  }
  
  
  //flip pot values if needed
  #if FLIP_FLEXION
  for (int i = 0; i < NUM_FINGERS; i++){
    rawFingers[i] = ANALOG_MAX - rawFingers[i];
  }
  #endif
  
  #if FLIP_SPLAY
  for (int i = NUM_FINGERS; i < 2 * NUM_FINGERS; i++){
    rawFingers[i] = ANALOG_MAX - rawFingers[i];
  }
  #endif
  
  #if ENABLE_MEDIAN_FILTER
  for (int i = 0; i < 2 * NUM_FINGERS; i++){
    rmSamples[i].add( rawFingers[i] );
    rawFingers[i] = rmSamples[i].getMedian();
  }
  #endif

  //reset max and mins as needed
  /*if (reset){
    for (int i = 0; i <2 * NUM_FINGERS; i++){
      #if FLEXION_MIXING == MIXING_SINCOS
      if (i < NUM_FINGERS)
        totalOffset1[i] = 0;
      #endif
      maxFingers[i] = INT_MIN;
      minFingers[i] = INT_MAX;
    }
  }*/
  
  if (reset && !bylRST && bttnPRS == 0) { //první klik
    displayOLED.vypis(1, 11, 0, "ZKALIBROVAN", "   Horni limit",3);
    click = 10000;
    bylRST = true;
    bttnPRS++;
    for (int i = 0; i < NUM_FINGERS; i++){
      minFingers[i] = rawFingers[i];
      /*if (i == 0) {
        Serial.println("minFingers[i]: " + String(i) + ": " + String(minFingers[i])  );
        delay(500);
      }*/
    }

  } else if (reset && !bylRST && bttnPRS == 1) { //druhý klik
    displayOLED.vypis(1, 11, 0, "ZKALIBROVAN", "   Dolni limit",3);
    click = 10000;
    bylRST = true;
    bttnPRS = 0;
    for (int i = 0; i < NUM_FINGERS; i++){
      maxFingers[i] = rawFingers[i];
      /*if (i == 0) {
        Serial.println("maxFingers[i]: " + String(i) + ": " + String(maxFingers[i])  );
        delay(500);
      }*/
    }

  } else if(!reset && bylRST) {
    bylRST = false;
  }
  if(click > 2) {
    click--;
  } else if(click == 2){
    displayOLED.vypis(1, 11, 0, " ", " ",2);
    click = 1;
  }

  //if during the calibration sequence, make sure to update max and mins
  /*if (calibrating){
    for (int i = 0; i < 2*NUM_FINGERS; i++){
      if (rawFingers[i] > maxFingers[i])
        #if CLAMP_SENSORS
          maxFingers[i] = ( rawFingers[i] <= CLAMP_MAX )? rawFingers[i] : CLAMP_MAX;
        #else
          maxFingers[i] = rawFingers[i];
          if (savedTravel && (maxFingers[i] - minFingers[i] > maxTravel[i]))
              minFingers[i] = maxFingers[i] - maxTravel[i];
        #endif
      if (rawFingers[i] < minFingers[i])
        #if CLAMP_SENSORS
          minFingers[i] = ( rawFingers[i] >= CLAMP_MIN )? rawFingers[i] : CLAMP_MIN;
        #else
          minFingers[i] = rawFingers[i];
          if (savedTravel && (maxFingers[i] - minFingers[i] > maxTravel[i]))
              maxFingers[i] = minFingers[i] + maxTravel[i];
        #endif
    }
  }*/
  
  for (int i = 0; i<NUM_FINGERS; i++){
    if (minFingers[i] != maxFingers[i]){
      fingerPos[i] = map( rawFingers[i], minFingers[i], maxFingers[i], 0, ANALOG_MAX );

      /*if (i == 0) {
        Serial.println("fingerPos[i]: " + String(i) + ": " + String(fingerPos[i])  );
        delay(50);
      }*/

      #if CLAMP_ANALOG_MAP
        if (fingerPos[i] < 0)
          fingerPos[i] = 0;
        if (fingerPos[i] > ANALOG_MAX)
          fingerPos[i] = ANALOG_MAX;
      #endif
    }
    else {
      fingerPos[i] = ANALOG_MAX / 2;
    }
    
  }
}

int InputManager::analogReadDeadzone(int pin){
  int raw = analogPinRead(pin);
  if (abs(ANALOG_MAX/2 - raw) < JOYSTICK_DEADZONE * ANALOG_MAX / 100)
    return ANALOG_MAX/2;
  else
    return raw;
}

int InputManager::getJoyX(){
  #if JOYSTICK_BLANK
  return ANALOG_MAX/2;
  #elif JOY_FLIP_X
  return ANALOG_MAX - analogReadDeadzone(PIN_JOY_X);
  #else
  return analogReadDeadzone(PIN_JOY_X);
  #endif
}

int InputManager::getJoyY(){
  #if JOYSTICK_BLANK
  return ANALOG_MAX/2;
  #elif JOY_FLIP_Y
  return ANALOG_MAX - analogReadDeadzone(PIN_JOY_Y);
  #else
  return analogReadDeadzone(PIN_JOY_Y);
  #endif
}

bool InputManager::getButton(byte pin){
  return digitalRead(pin) != HIGH;
}

#if FLEXION_MIXING == MIXING_SINCOS
//mixing
int InputManager::sinCosMix(int sinPin, int cosPin, int i){

  int sinRaw = analogPinRead(sinPin);
  int cosRaw = analogPinRead(cosPin);

  #if INTERFILTER_MODE != INTERFILTER_NONE
    sinSamples[i].add(sinRaw);
    cosSamples[i].add(cosRaw);
    int sinCalib = sinSamples[i].getMedian();
    int cosCalib = cosSamples[i].getMedian();
    #if INTERFILTER_MODE == INTERFILTER_ALL
      sinRaw = sinCalib;
      cosRaw = cosCalib;
    #endif
  #else
    //int sinCalib = sinRaw;
    //int cosCalib = cosRaw;
  #endif 

  /*if (!savedInter){
    //scaling
    sinMin[i] = min(sinCalib, sinMin[i]);
    sinMax[i] = max(sinCalib, sinMax[i]);

    cosMin[i] = min(cosCalib, cosMin[i]);
    cosMax[i] = max(cosCalib, cosMax[i]);
  }*/

  //int sinScaled = map(sinRaw, sinMin[i], sinMax[i], -ANALOG_MAX, ANALOG_MAX);
  //int cosScaled = map(cosRaw, cosMin[i], cosMax[i], -ANALOG_MAX, ANALOG_MAX);
  int sinScaled = map(sinRaw, 2350, 3450, -ANALOG_MAX, ANALOG_MAX);
  int cosScaled = map(cosRaw, 2350, 3450, -ANALOG_MAX, ANALOG_MAX);
  /*Serial.print("sinRaw: " + String(sinRaw));
  Serial.print("cosRaw: " + String(cosRaw));
  Serial.print("sinScaled: " + String(sinScaled));*/
  //Serial.println("sinRaw: " + String(sinRaw) + "cosRaw: " + String(cosRaw) + "sinMin: " + String(sinMin[i]) + "sinMax[i]: " + String(sinMax[i]) + "cosMin[i]: " + String(cosMin[i]) + "cosMax[i]: " + String(cosMax[i]) + "sinCalib: " + String(sinCalib) ) ;

  //trigonometry stuffs
  double angleRaw = atan2(sinScaled, cosScaled);

  //counting rotations
  if (((angleRaw > 0) != atanPositive[i]) && sinScaled > cosScaled){
    totalOffset1[i] += atanPositive[i]?1:-1;
  }
  atanPositive[i] = angleRaw > 0;
  double totalAngle = (angleRaw + 2*PI * totalOffset1[i]);

  if (totalOffset1[i] < -1 || totalOffset1[i] > 1) {
    totalOffset1[i] = 0;
  }
  
  /*if (i == 0) {
    Serial.println("totalAngle: " + String(i) + ": " + String((int)(totalAngle * ANALOG_MAX)) + "  totalOffset: " + String(totalOffset1[i]) + "  angleRawD: " + String(degrees(angleRaw))   );
    delay(75);
  }*/
  //delay(1);
  //delayMicroseconds(10);
  //int angleScaled = map(angleRaw, -1.25, 3, 0, ANALOG_MAX);
  return (int)(totalAngle * ANALOG_MAX);
  //return (int)(((angleRaw+1.25) - 0) * (ANALOG_MAX - 0) / (4.25 - 0) + 0);
  
}
#endif

void InputManager::saveTravel()
{
  byte flags = EEPROM.read(0x00);
  flags |= 0x01;  // Set bit 0
  EEPROM.write(0x00, flags); // Save clamping saved limits flag

  int addr = 0x01;  // Start address for flexion and splay values

  for (int i = 0; i < 2*NUM_FINGERS; i++) {
    int diff = maxFingers[i] - minFingers[i]; // Calculate the difference
    EEPROM.put(addr, diff); // Store the difference into the EEPROM at the current address
    addr += sizeof(int); // Increment the address by 4 because we're storing int values
  }
  
  EEPROM.commit(); // Ensure changes are written to EEPROM

  loadTravel();
}

void InputManager::saveIntermediate()
{
  #if FLEXION_MIXING == MIXING_SINCOS
  byte flags = EEPROM.read(0x00);
  flags |= 0x02;  // Set bit 1
  EEPROM.write(0x00, flags); // Save intermediate values saved flag

  int address = 0x29; // Start address for sin and cos values
  
    for(int i = 0; i < NUM_FINGERS; i++)
    {
      EEPROM.put(address, sinMax[i]);
      address += sizeof(int);
      EEPROM.put(address, sinMin[i]);
      address += sizeof(int);
      EEPROM.put(address, cosMax[i]);
      address += sizeof(int);
      EEPROM.put(address, cosMin[i]);
      address += sizeof(int);
    }
    
    EEPROM.commit(); // Ensure changes are written to EEPROM
  #endif
}

void InputManager::clearFlags()
{
  EEPROM.write(0x00, 0x00); // Clear the flags
  EEPROM.commit(); // Ensure the changes are written to EEPROM
}

bool InputManager::isSavedLimits()
{
  byte flags = EEPROM.read(0x00);
  return flags & 0x01;  // Check bit 0
}

bool InputManager::isSavedIntermediate()
{
  byte flags = EEPROM.read(0x00);
  return flags & 0x02;  // Check bit 1
}

void InputManager::loadTravel()
{
  byte flags = EEPROM.read(0x00);
  if (!(flags & 0x01)) return; // If clamping saved limits flag is not set, do nothing

  int addr = 0x01;  // Start address for flexion and splay values

  for (int i = 0; i < 2*NUM_FINGERS; i++) {
    EEPROM.get(addr, maxTravel[i]); // Load the max travel value from the EEPROM at the current address
    addr += sizeof(int); // Increment the address by 4 because we're storing int values
  }
}

void InputManager::loadIntermediate()
{
  #if FLEXION_MIXING == MIXING_SINCOS
  byte flags = EEPROM.read(0x00);
  if (!(flags & 0x02)) return; // If intermediate values saved flag is not set, do nothing

  int address = 0x29; // Start address for sin and cos values
  
    for(int i = 0; i < NUM_FINGERS; i++)
    {
      //EEPROM.get(address, sinMax[i]);
      address += sizeof(int);

      //EEPROM.get(address, sinMin[i]);
      address += sizeof(int);

      //EEPROM.get(address, cosMax[i]);
      address += sizeof(int);

      //EEPROM.get(address, cosMin[i]);
      address += sizeof(int);
    }
  #endif
}



