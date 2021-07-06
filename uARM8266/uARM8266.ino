/*
    uARM8266: Linux on ARM on esp8266.
    Original work on AVR by Dmitry Grinberg
    
    You can see his project here:
    http://dmitry.gr/?r=05.Projects&proj=07.%20Linux%20on%208bit

    For license: see LICENSE.txt in the repo

    Ported to esp8266 by @raspiduino
*/

#include <SD.h>
#include <Ticker.h>

extern "C" {
#include "SoC.h"
#include "callout_RAM.h"
#include "config.h"
}

/* For more configs about pins, SD... see config.h */

Ticker rtc; // Emulate rtc timer
volatile UInt32 gRtc;

static int readchar(void){
    if(Serial.available() > 0){  
        return Serial.read();
    }
    else return CHAR_NONE;
}

void writechar(int chr){
    Serial.print(chr);
}

void rtc_tick(){

    static UInt8 tik = 0;

    if(tik) gRtc++;
    tik ^= 1;

    // Feed watchdog
    ESP.wdtFeed();
}

void err_str(const char* str){
    
    char c;
    
    while((c = *str++) != 0) writechar(c);
}

UInt32 rtcCurTime(void){
    
    UInt32 t;
    
    do{
        
        t = gRtc;
        
    }while(t != gRtc);
    
    return t;
}

void* emu_alloc(_UNUSED_ UInt32 size){
    
    err_str("No allocations in esp8266 mode please!");
    
    return 0;
}

File file;

Boolean sdSecRead(UInt32 sec, void* buf){
    UInt32* tbuf = static_cast<UInt32*>(buf);

    if(!SD.exists(DISK_FILE)){
        Serial.print(DISK_FILE);
        Serial.println("not found! Halting...");

        while(1);
    }
    
    file = SD.open(DISK_FILE, FILE_READ);
    if(!file) return false;
    if(!file.seek(sec*512)) return false;
    
    for(int i = 0; i < 128; i++){
        UInt8 tmp[4];

        for(int j = 0; j < 4; j++){
            tmp[j] = file.read();
        }

        tbuf[i] = tmp[0] | tmp[1] << 8 | tmp[2] << 16 | tmp[3] << 24;
    }

    return true;
}

Boolean sdSecWrite(UInt32 sec, void* buf){
    UInt32* tbuf = static_cast<UInt32*>(buf);

    if(!SD.exists(DISK_FILE)){
        Serial.print(DISK_FILE);
        Serial.println("not found! Halting...");

        while(1);
    }

    file = SD.open(DISK_FILE, FILE_WRITE);
    if(!file) return false;
    if(!file.seek(sec*512)) return false;

    for(int i = 0; i < 128; i++){
        file.write(tbuf[i]);
    }
    
    return true;
};

void ramRead(UInt32 addr, void* buf, UInt8 sz){

    UInt8* b = static_cast<UInt8*>(buf);

    file = SD.open(RAM_FILE, FILE_WRITE);
    file.seek(addr);
    file.read(b, sz);
}
void ramWrite(UInt32 addr, void* buf, UInt8 sz){

    UInt8* b = static_cast<UInt8*>(buf);

    file = SD.open(RAM_FILE, FILE_WRITE);
    file.seek(addr);
    file.write(b, sz);
}

int rootOps(void* userData, UInt32 sector, void* buf, UInt8 op){

    UInt32* tbuf = static_cast<UInt32*>(buf);
    
    switch(op){
        case BLK_OP_SIZE:
            
            if(sector == 0){    //num blocks
                *(unsigned long*)tbuf = NUMBER_OF_SECTOR;
            }
            else if(sector == 1){   //block size
                
                *(unsigned long*)tbuf = 512;
            }
            else return 0;
            return 1;
        
        case BLK_OP_READ:
            
            return sdSecRead(sector, tbuf);
            
        
        case BLK_OP_WRITE:
            
            return sdSecWrite(sector, tbuf);
    }
    return 0;   
}

void coRamAccess(UInt32 c_addr, UInt8 c_size, UInt8 c_write, void* c_b){

    if(c_write) ramWrite(c_addr, c_b, c_size);
    else ramRead(c_addr, c_b, c_size);
}

static SoC soc;

void setup() {
    
    // UART
    Serial.begin(115200);
    
    //RTC timer
    rtc.attach(0.5, rtc_tick);

    // Button to get speed
    #ifdef ENABLE_BUTTON_GET_SPEED
    pinMode(BUTTON_GET_SPEED, INPUT_PULLUP);
    #endif
    
    if(!SD.begin(SD_CS_PIN)) err_str("sd init failed");
    
    socInit(&soc, socRamModeCallout, coRamAccess, readchar, writechar, rootOps, 0);

    #ifdef ENABLE_BUTTON_GET_SPEED
    if(!(digitalRead(BUTTON_GET_SPEED))){ //hack for faster boot in case we know all variables & button is pressed
        UInt32 i, s = 786464UL;
        UInt32 d = 0xA0E00000;
        UInt16 j;
        UInt8* b = (UInt8*)soc.blkDevBuf;

        for(i = 0; i < 4096; i++){
            sdSecRead(s++, b);
            for(j = 0; j < 512; j += 32, d+= 32){
                
                ramWrite(d, b + j, 32);
            }
        }
        soc.cpu.regs[15] = 0xA0E00000UL+512UL;
    }
    #endif

    socRun(&soc, 0);
}

void loop() {
    // Nope :)
}
