/**
 * SMuFF EStopMux Firmware
 * Copyright (C) 2021 Technik Gegg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDRESS     0x3E    // hardcoded in the SMuFF

#define VERSION         1
#define MAX_PINS        7       // though, only 5 needed for the Splitter option

#define GET_TOOL        0
#define SET_TOOL        1
#define GET_VERSION     2
#define GET_MAXPINS     3
#define RESET_MUX       99

#define SW0_PIN         4
#define SW1_PIN         5
#define SW2_PIN         6
#define SW3_PIN         7
#define SW4_PIN         8
#define SW5_PIN         9
#define SW6_PIN         10
#define OUT_PIN         16

#define DEBUG           1
#define ArraySize(arr) (sizeof(arr) / sizeof(arr[0]))

void wireOnReceive(int bytes);
void __debugS(const char *fmt, ...);

int8_t currentTool = 0;
bool pinStates[MAX_PINS];
uint32_t cnt = 0;

void setup()
{
    pinMode(OUT_PIN, OUTPUT);
    for(uint8_t i=0; i < MAX_PINS; i++) {
        pinStates[i] = false;
        pinMode(i+SW0_PIN, INPUT_PULLUP);
    }
    Serial.begin(115200);

    Wire.begin(I2C_ADDRESS);
    Wire.onReceive(wireOnReceive);
}

void loop()
{
    // read all pin states and save them
    for(uint8_t i=0; i < MAX_PINS; i++)
        pinStates[i] = digitalRead(i+SW0_PIN);
    // just for debugging
    if(currentTool != -1) {
      digitalWrite(OUT_PIN, pinStates[currentTool]);
      if(cnt % 100 == 0) { // every 2 seconds
          __debugS("T%d: %d", currentTool, pinStates[currentTool]);
      }
    }
    cnt++;
    delay(20);
}

void wireOnReceive(int bytes) {

    while(Wire.available()) {
      uint8_t fnc = Wire.read();
      switch(fnc) {
        case GET_TOOL:
            __debugS("TOOL requested");
            Wire.write((byte)currentTool);
            break;
        case SET_TOOL:
            currentTool = Wire.read();
            __debugS("TOOL NOW: %d", currentTool);
            break;
        case GET_VERSION:
            __debugS("VERSION requested");
            Wire.write((byte)VERSION);
            break;
        case GET_MAXPINS:
            __debugS("MAX_PINS requested");
            Wire.write((byte)MAX_PINS);
            break;
        case RESET_MUX:
            __debugS("RESET requested");
            asm volatile ("jmp 0");
            break;
      }
    }
}

void __debugS(const char *fmt, ...)
{
#ifdef DEBUG
    char _dbg[512];
    va_list arguments;
    va_start(arguments, fmt);
    vsnprintf_P(_dbg, ArraySize(_dbg), fmt, arguments);
    va_end(arguments);
    Serial.println(_dbg);
#endif
}
