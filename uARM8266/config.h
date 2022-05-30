/* Config file for uARM8266 project */

/*
 * Boot mode:
 * BOOT_SD: Boot from SD card
 * BOOT_NET: Boot from network (settings are below)
 * If you define both, there will be a prompt for selecting mode at the
 * serial
 */

//#define BOOT_SD
#define BOOT_NET

/**********************************************************************/

/*
 * Boot from SD card
 * Settings here will be ignored if BOOT_MODE is 2
 */

#ifdef BOOT_SD

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

#endif

/**********************************************************************/

/*
 * Boot from network
 * Use this if you don't have SD card or have problems with using SD cards
 * Server must run the python script `boot_network.py`
 */

#ifdef BOOT_NET

/*
 * WIFI setting
 * Connect to your wifi to communicate with the server
 */

#define AP_SSID "mywifi"   // Access Point name
#define AP_PASS "passw0rd" // Access Point password (remember not to commit this to your git repository :))

/*
 * Boot server setting
 */

#define SERVER_PORT 1234 // Port to open on esp8266

// If you want, you can add some code for using password to connect
// But I think it has no use for an emulator :)

#define BUFFER_SIZE 4096 // Data read buffer

#endif

/*
    Speed up emulator's boot process (e.g Boot directly to ELLE instead of to different bootloader stages)
    If you want this feature, leave the ENABLE_BUTTON_GET_SPEED
    uncommented. You can also set the pin of the button to get
    speed. Default is 5
*/

//#define ENABLE_BUTTON_GET_SPEED

#ifdef ENABLE_BUTTON_GET_SPEED
#define BUTTON_GET_SPEED 5 // GPIO5 by default
#endif
