/*
    uARM8266: Linux on ARM on esp8266.
    Original work on AVR by Dmitry Grinberg
    
    You can see his project here:
    http://dmitry.gr/?r=05.Projects&proj=07.%20Linux%20on%208bit

    For license: see LICENSE.txt in the repo

    Ported to esp8266 by @raspiduino
*/

#include <Ticker.h>

extern "C" {
#include "SoC.h"
#include "callout_RAM.h"
#include "config.h"
}

#ifdef BOOT_NET
#include <ArduinoWebsockets.h> // Websocket library (https://github.com/gilmaimon/ArduinoWebsockets)
#include <ESP8266WiFi.h>

using namespace websockets;
#endif 

#ifdef BOOT_SD
#include <SD.h>
#endif

/******************************SD boot helper code******************************/
#ifdef BOOT_SD

File file;

Boolean sdInit(int cs){
    return SD.begin(cs);
}

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
#endif

/******************************* Network boot helper code******************************/
#ifdef BOOT_NET
void wifiInit(String ssid, String pass){
    WiFi.begin(ssid, pass); // Connect to Wifi
    while(WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(200);
    }
}

WebsocketsServer server;
WebsocketsClient client;

void startServer(){
    server.listen(SERVER_PORT); // Listen
    Serial.print(F("Is server live? "));
    Serial.println(server.available());
    Serial.println("Please connect to this webSocket server now!");

    // Wating for client to connect
    while (true) {
        client = server.accept();
        if(client.available()) {
            // If an uARM network boot client connected, it will sent its version number

            WebsocketsMessage msg = client.readBlocking();
            Serial.print("Client connected. Client version: ");
            Serial.println(msg.data());
            client.send("connected"); // Send connected message to client
            break; // Done waiting for client
        }
    }
}

Boolean sdSecRead(UInt32 sec, void* buf) {
    const char* b = static_cast<const char*>(buf);
    while(!client.send("sec_read " + String(sec))); // Send command to client
    while(!client.available()); // Waiting for response

    auto msg = client.readBlocking();
    b = msg.c_str();

    while(!client.send("done")); // Done
    return true;
}

Boolean sdSecWrite(UInt32 sec, void* buf) {
    const char* b = static_cast<const char*>(buf);
    while(!client.send("sec_write " + String(sec))); // Send command to client
    while(!client.available()); // Waiting for response
    auto msg = client.readBlocking(); // Client will send back "ok"

    while(!client.send(b));
    client.send("done"); // Done

    while(!client.available()); // Waiting for response
    client.readBlocking(); // Client will send back "ok"
    return true;
}

void ramRead(UInt32 addr, void* buf, UInt8 sz) {
    const char* b = static_cast<const char*>(buf);
    while(!client.send("ram_read " + String(addr) + String(sz))); // Send command to client
    while(!client.available()); // Waiting for response

    auto msg = client.readBlocking();
    b = msg.c_str();

    while(!client.send("done")); // Done
}

void ramWrite(UInt32 addr, void* buf, UInt8 sz) {
    const char* b = static_cast<const char*>(buf);
    while(!client.send("sec_write " + String(addr) + String(sz))); // Send command to client
    while(!client.available()); // Waiting for response
    auto msg = client.readBlocking(); // Client will send back "ok"

    while(!client.send(b));
    client.send("done"); // Done

    while(!client.available()); // Waiting for response
    client.readBlocking(); // Client will send back "ok"
}
#endif

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

String read_eol() {
    int read;
    String value; // Placeholder string

    while (true) {
        if (Serial.available() > 0) {
            read = Serial.read();
            if (read == -1 || read == 10 || read == 13)
                if (read == 13)
                    break; // User pressed enter -> break loop
            else
                value = String(value + char(read)); // Add char to string
        }
    }

    return value; // Return
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

int rootOps(void* userData, UInt32 sector, void* buf, UInt8 op){

    UInt32* tbuf = static_cast<UInt32*>(buf);
    
    switch(op){
        case BLK_OP_SIZE:
            
            if(sector == 0){    //num blocks
                *(unsigned long*)tbuf = 2097152;
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

#ifdef BOOT_SD
void init_sd() {
    int cs_pin = SD_CS_PIN;
    
    // Ask for chip select pin and initilize SD card
    while (true) {
        Serial.printf("\nEnter CS pin (default is %d, press enter only to accept): ", cs_pin);
        String choice = read_eol();
        if (choice != "") {
            cs_pin = choice.toInt();
            Serial.printf("%d\n", cs_pin);
        }

        if(sdInit(cs_pin))
            break;
        else
            Serial.printf("\n SD with CS pin %d not working, please check the connection!", cs_pin);
    }
}
#endif

#ifdef BOOT_NET
void init_net() {
    String ssid = AP_SSID;
    String password = AP_PASS;

    // Ask for SSID
    Serial.print(F("Enter SSID (default is "));
    Serial.print(AP_SSID);
    Serial.print(F(" , press enter to accept): "));
    String choice = read_eol();

    if (choice != "") {
        ssid = choice;
        Serial.println(ssid);
    }

    // Ask for password
    Serial.print(F("Enter password (default password not shown, press enter to accept): "));
    choice = read_eol();

    if (choice != "") {
        password = choice;

        for (int i = 0; i < password.length(); i++) {
            Serial.print("*");
        }
        Serial.println("");
    }

    Serial.print(F("Connecting to WiFi"));
    wifiInit(ssid, password);
    Serial.printf("\nConnected to WiFi successfully. IP: ");
    Serial.println(WiFi.localIP());

    Serial.printf("Starting websocket server on port %d\n", SERVER_PORT);
    startServer();

}
#endif

static SoC soc;

void setup() {
    
    /**************************** Initialize devices ****************************/
    // UART
    Serial.begin(115200);
    
    //RTC timer
    rtc.attach(0.5, rtc_tick);

    // Button to boot faster
    #ifdef ENABLE_BUTTON_GET_SPEED
    pinMode(BUTTON_GET_SPEED, INPUT_PULLUP);
    #endif

    /* Boot menu */
    Serial.println(F("uARM on esp8266"));
    Serial.println(F("Copyright (C) Dmitry Grinberg (dmtirygr@gmail.com)"));
    Serial.printf("Ported to esp8266 by Giang Vinh Loc (giangvinhloc610@gmail.com)\naka @raspiduino on github.com");

    #if defined(BOOT_SD) && defined(BOOT_NET)
    while (true) {
        Serial.println(F("This version have both support for booting from SD card and booting from network"));
        Serial.printf("Choose one:\n1. Boot from SD card\n 2. Boot from network\nEnter 1 or 2 in the serial: ");

        String choice = read_eol(); // Read user's choice
        if (choice == "1")
            Serial.println("1");
            init_sd(); // Initialize SD card
            break;
        else if (choice == "2")
            Serial.println("2");
            init_net(); // Initialize network boot
            break;
        else
            Serial.println(F("Invaild choice, enter 1 or 2."));
    }
    #endif

    #if defined(BOOT_SD) && !(defined(BOOT_NET))
    init_sd();
    #endif

    #if (!defined(BOOT_SD)) && defined(BOOT_NET)
    init_net();
    #endif

    /* Prepare emulator */
    
    socInit(&soc, socRamModeCallout, coRamAccess, readchar, writechar, rootOps, 0);

    #ifdef ENABLE_BUTTON_GET_SPEED
    if(!(digitalRead(BUTTON_GET_SPEED))){ //hack for faster boot in case we know all variables & button is pressed
        Serial.println(F("Boot directly to ELLE..."));
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

    Serial.println(F("Starting emulator..."));

    socRun(&soc, 0); // Run!

    Serial.println("Emulator exited! System now halt!");
}

void loop() {
    // Nope :)
}
