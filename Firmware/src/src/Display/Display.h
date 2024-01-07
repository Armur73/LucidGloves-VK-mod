#pragma once


//#if COMMUNICATION == COMM_BTSERIAL
    #include "BluetoothSerial.h"
//#endif

class Display {
public:
    Display();
    void loopDisplay(int*);
    void vypis(int, int, bool, char*, char*, int);
    void setupDisplay();
    void Baterie();
    bool jeSpojen;
    
private:
    BluetoothSerial m_SerialBT;
    int fingers[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int servo[5] = {0, 0, 0, 0 , 0};
    int baterie;
    uint8_t ikBat = 0;
    int loops = 1;
};