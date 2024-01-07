#pragma once

#include "BluetoothSerial.h"

class Battery {
public:
    Battery();
    void initBatt();
    bool isError();
    bool error;
    bool charging;
    bool isCharging();
    bool errors[5] = {false, false, false, false, false}; //PGstat || THERMstat || BATfault || NTCfault1 || NTCfault0
    bool charges[2] = {false, false}; //CHRG_stat1 || CHRG_stat0 

private:
    const int ADDR = 0x6b;
    const int ADDR_REG05 = 0x05; // nastavované bity: 5 4 3 (vychozi = 10011010)
    const int ADDR_REG08 = 0x08; // vyčítané bity: 5 4 2 1
    const int ADDR_REG09 = 0x09; // vyčítané bity: 3 1 0
    const byte Default05 = B10000010;
    BluetoothSerial m_SerialBT;
};