/* Config file for uARM8266 project */

/*
    Some notes about the SD pins
    From the Arduino IDE example of SD card for esp8266:
    
    ** MOSI - pin GPIO13
    ** MISO - pin GPIO12
    ** CLK - pin GPIO14
    ** CS - pin GPIO4 (you can change this)
*/

#define SD_CS_PIN 4 // SD chip select pin

/*
    Pin out the current emulator's speed
    If you want this feature, leave the ENABLE_BUTTON_GET_SPEED
    uncommented. You can also set the pin of the button to get
    speed. Default is 5
*/

#define ENABLE_BUTTON_GET_SPEED

#ifdef ENABLE_BUTTON_GET_SPEED
#define BUTTON_GET_SPEED 5 // GPIO5 by default
#endif

/* 
    Virtual disk image size
    You may set the virtual disk image size here.

    You must set it in number of sectors
    For example: if you set the disk size to be 1gb = 1073741824 bytes
    Then it will be 1073741824 / 512 = 2097152 sectors
*/

#define NUMBER_OF_SECTOR 2097152

/* Virtual disk image file name */
#define DISK_FILE "jaunty.rel.v2" // Original image from dmitry.gr

/* 
    Virtual ram file name

    ESP8266 cannot have enough pin to connect to a real RAM, so instead
    we use virtual RAM file on SD card.
*/
#define RAM_FILE "uARMram.bin"
