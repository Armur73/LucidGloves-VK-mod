#include <Arduino.h>

#include "BatteryManager.h"
#include <Wire.h>
#include "Display.h"

Display OLED;

Battery::Battery() {
    
}

void Battery::initBatt() {
    Wire.beginTransmission(ADDR);
	Wire.write(ADDR_REG05);
    Wire.write(Default05);
    Wire.endTransmission();
}

bool Battery::isError() {
    Wire.beginTransmission(ADDR);
	Wire.write(ADDR_REG08);
	Wire.requestFrom(ADDR, (int)1,HIGH); 
    byte REG08byte = Wire.read();
    Wire.endTransmission();
    
    Wire.beginTransmission(ADDR);
	Wire.write(ADDR_REG09);
	Wire.requestFrom(ADDR, (int)1,HIGH); 
    byte REG09byte = Wire.read();
    Wire.endTransmission();

    errors[0] = (REG08byte & (1 << 2)) != 0; //PGstat
    errors[1] = (REG08byte & (1 << 1)) != 0; //THERMstat
    errors[2] = (REG09byte & (1 << 3)) != 0; //BATfault
    errors[3] = (REG09byte & (1 << 1)) != 0; //NTCfault1
    errors[4] = (REG09byte & (1 << 0)) != 0; //NTCfault0
    error = (errors[0] || errors[1] || errors[2] || errors[3] || errors[4]);

    if(error){
        if(errors[0]) {
            OLED.vypis(1, 11, 0, "CHYBA", "PG",3);
        }
        if(errors[1]) {
            OLED.vypis(1, 11, 0, "CHYBA", "THERM", 3);
            m_SerialBT.disconnect();
        }
        if(errors[2]) {
            OLED.vypis(1, 11, 0, "CHYBA", "BAT_FAULT", 3);
        }
        if(errors[3]) {
            OLED.vypis(1, 11, 0, "CHYBA", "COLD", 3);
        }
        if(errors[4]) {
            OLED.vypis(1, 11, 0, "CHYBA", "HOT", 3);
            m_SerialBT.disconnect();
        }
    }

    return error;
}

bool Battery::isCharging() {
    Wire.beginTransmission(ADDR);
	Wire.write(ADDR_REG08);
	Wire.requestFrom(ADDR, (int)1,HIGH); 
    byte REG08byte = Wire.read();
    Wire.endTransmission();

    charges[0] = (REG08byte & (1 << 5)) != 0; //CHRGstat1
    charges[1] = (REG08byte & (1 << 4)) != 0; //CHRGstat2

    charging = ((!charges[1] && charges[0]) || (charges[1] && !charges[0] ));

    return charging;
}