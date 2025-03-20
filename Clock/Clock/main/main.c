/* Components Headers */
#include "Button.h"
#include "Buzzer.h"
#include "LCD.h"
#include "InternalRealTimeClock.h"
// #include "RealTimeClock.h"
#include "SlideSwitch.h"

/* Services Headers */
#include "BLE.h"
#include "Wifi.h"
#include "bulb.h"


/* FreeRTOS Header */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* Debug Logs */
#include "esp_log.h"

/* Defnitions */
// initialization
#define EVT_HW_INITIALIZED   (EventBits_t)(1<<0) // HW Initialized

// Clock Modes
#define EVT_CLOCK_MODE  (EventBits_t)(1<<1) // Slide_Switch on Clock Mode
#define EVT_TIME_MODE   (EventBits_t)(1<<2) // Slide_Switch on Time Mode
#define EVT_ALARM_MODE  (EventBits_t)(1<<3) // Slide_Switch on Alarm Mode

// Sensors
#define READ_TIME_BIT   (EventBits_t)(1<<4) // Slide_Switch on Alarm Mode
#define BUZZER_ON       (EventBits_t)(1<<5) // Buzzer Activates
#define BUZZER_SNOOZED  (EventBits_t)(1<<6) // Buzzer Activates


// WIFI
#define WIFI_CONNECTED  (EventBits_t)(1<<7) // Verify Wifi Connection
#define WIFI_FAIL       (EventBits_t)(1<<8) // Verify Wifi fail
#define WIFI_CONNECT    (EventBits_t)(1<<9) // Wifi connect
#define WIFI_CONFIG     (EventBits_t)(1<<10) // confirms existing configuration

#define BULB_CONFIG     (EventBits_t)(1<<11) // confirms existing configuration


/* Global Structures */
typedef struct parts_t{
    // Components
    slide_switch_t slide_switch;
    button_t button;
    buzzer_t buzzer;
    lcd_t lcd;
    internal_real_time_clock_t rtc;
    

    // Variables
    uint8_t currentTime[4], alarmTime[4];

    
    //wifi
       char* ssid; 
       char* pass;
    
    //bulb
        char* sku; //device sku
        char* device; //device mac address
        char* key; //govee key
        int* bulbmode; //use Bulb mode
    
        
}parts_t;

/* Global Objects/Variables */
// Parts
parts_t parts;

// Variables
uint8_t alarmActive = 0;

// Constant
const uint8_t lcdCharacters[] = {
    0x06, 0x00, 0x00, 0xFE, 0x82, 0x82, 0xFE, 0x00, 0x00,  // Code for char 0
    0x05, 0x00, 0x00, 0x00, 0x04, 0xFE, 0x00, 0x00, 0x00,  // Code for char 1
    0x06, 0x00, 0x00, 0xF2, 0x92, 0x92, 0x9E, 0x00, 0x00,  // Code for char 2
    0x06, 0x00, 0x00, 0x92, 0x92, 0x92, 0xFE, 0x00, 0x00,  // Code for char 3
    0x06, 0x00, 0x00, 0x1E, 0x10, 0x10, 0xFE, 0x00, 0x00,  // Code for char 4
    0x06, 0x00, 0x00, 0x9E, 0x92, 0x92, 0xF2, 0x00, 0x00,  // Code for char 5
    0x06, 0x00, 0x00, 0xFE, 0x92, 0x92, 0xF2, 0x00, 0x00,  // Code for char 6
    0x06, 0x00, 0x00, 0x02, 0x02, 0x02, 0xFE, 0x00, 0x00,  // Code for char 7
    0x06, 0x00, 0x00, 0xFE, 0x92, 0x92, 0xFE, 0x00, 0x00,  // Code for char 8
    0x06, 0x00, 0x00, 0x9E, 0x92, 0x92, 0xFE, 0x00, 0x00,  // Code for char 9
    0x05, 0x00, 0x00, 0x00, 0x66, 0x66, 0x00, 0x00, 0x00,  // Code for char :
    0x07, 0x00, 0xFF, 0xFF, 0x33, 0x33, 0xFF, 0xFF, 0x00,  // Code for char A
    0x07, 0x00, 0xFF, 0xFF, 0x99, 0x99, 0xFF, 0x7E, 0x00,  // Code for char B
    0x07, 0x00, 0xFF, 0xFF, 0xC3, 0xC3, 0xE7, 0xE7, 0x00,  // Code for char C
    0x07, 0x00, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0x7E, 0x00,  // Code for char D
    0x07, 0x00, 0xFF, 0xFF, 0xDB, 0xDB, 0xDB, 0xDB, 0x00,  // Code for char E
    0x07, 0x00, 0xFF, 0xFF, 0x18, 0x18, 0xFF, 0xFF, 0x00,  // Code for char H
    0x07, 0x00, 0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0, 0x00,  // Code for char L
    0x07, 0x00, 0xFF, 0xFF, 0x1C, 0x38, 0xFF, 0xFF, 0x00,  // Code for char N
    0x07, 0x00, 0x7E, 0xFF, 0xC3, 0xC3, 0xFF, 0x7E, 0x00,  // Code for char O
    0x07, 0x00, 0xFF, 0xFF, 0x19, 0x19, 0x1F, 0x1F, 0x00,  // Code for char P
    0x07, 0x00, 0x03, 0x03, 0xFF, 0xFF, 0x03, 0x03, 0x00,  // Code for char T
    0x07, 0x00, 0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0x00,  // Code for char U
    0x08, 0x7F, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0x7F,  // Code for char W
};

// FreeRTOS
EventGroupHandle_t eventGroup; // Event Group Handle
TaskHandle_t initializeAllHWHandle; // HW Handle
TaskHandle_t checkControlModeHandle; // SlideSwitch Handle
TaskHandle_t changeCurrentTimeHandle, changeAlarmTimeHandle; // SlideSwitch Handle
TaskHandle_t readClockTimeHandle; // RTC Handle

TaskHandle_t setupBLEHandle; // BLE handle


TaskHandle_t setWifihandle; //set wifi handle
TaskHandle_t setbulbhandle; //set wifi handle


// Logging
static const char *TAG = "Clock Log:";

/* Function Prototypes */
// Control Task

void ControlTask(void *pvParameters);

// Computation Tasks
void initializeAllHWTask(void *pvParameters);
void checkControlMode(void *pvParameters);
void changeCurrentTime(void *pvParameters);
void changeAlarmTime(void *pvParameters);
void readClockTime(void *pvParameters);


void setWifi(void *pvParameters);
void bulbthread(void *pvParameters);

// Helper Function
EventBits_t synchronizeWait(const EventBits_t uxBitsToSet);
EventBits_t synchronizeSet(const EventBits_t uxBitsToSet);
EventBits_t synchronizeClear(const EventBits_t uxBitsToSet);
void updateTime(parts_t *part, uint8_t currentTime, uint8_t timeIndex);

/* Main Functions */
void app_main(void){
    // 1) Initialize NVS first (only once!)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    
    // 2) Initialize Time
    ESP_LOGI(TAG,"Initialize Time");
    for(int i=0; i<3; i++){
        parts.currentTime[i] = (i==0)?12:0;
        parts.alarmTime[i] = (i==0)?8:0;
    }
    // 3) Store Time
    for(int i=0;i<4;i++){
        storedWatchData[i] = 0;
    }
    storedWatchData[4] = parts.currentTime[0];
    storedWatchData[5] = parts.currentTime[1];

    // 4) Initialize BLE
    initializeBLE();
    
    parts.ssid= Pixel_GB; //code won't work currently if this isn't set
    parts.pass= ESP324l1fe; //code won't work currently if this isn't set
    parts.sku=WifiSku;
    parts.bulbmode=0;
    parts.device=WifiMacAddress;
    parts.key = WifiApiKey;

    wifi_init();
    wifi_configuration(parts.ssid,parts.pass);
    wifi_start();
    vTaskDelay(10000)
    sendRequest(0);
    vTaskDelay(10000)
    sendRequest(1);

    //ControlTask();
}

void ControlTask(){   
    // Variables
    eventGroup = xEventGroupCreate();

    // Clear All EVENTS
    xEventGroupClearBits(eventGroup,EVT_HW_INITIALIZED);
    xEventGroupClearBits(eventGroup,EVT_TIME_MODE|EVT_ALARM_MODE|EVT_CLOCK_MODE);

    /* Create All Tasks */
    // Initalize All HW
    xTaskCreatePinnedToCore(initializeAllHWTask,"HardWare Initialization", sizeof(parts_t),part, configMAX_PRIORITIES -1, &initializeAllHWHandle,1);
    // Create All Functionality tasks
    // xTaskCreatePinnedToCore(setupBLE,"Setup BLE", sizeof(parts_t),part, tskIDLE_PRIORITY + 4,&setupBLEHandle,0);
    xTaskCreatePinnedToCore(checkControlMode,"Check Clock Mode", sizeof(parts_t),part, tskIDLE_PRIORITY + 3,&checkControlModeHandle,1);
    xTaskCreatePinnedToCore(readClockTime,"Read Current Time", sizeof(parts_t),part, tskIDLE_PRIORITY,&readClockTimeHandle,1);
    xTaskCreatePinnedToCore(changeCurrentTime,"Change Curent Time", sizeof(parts_t),part, tskIDLE_PRIORITY + 1,&changeCurrentTimeHandle,1);
    xTaskCreatePinnedToCore(changeAlarmTime,"Change Alarm Time", sizeof(parts_t),part, tskIDLE_PRIORITY + 1,&changeAlarmTimeHandle,1);
    xTaskCreatePinnedToCore(bulbthread,"Bulb Thread",sizeof(parts_t),part, tskIDLE_PRIORITY + 1,&setbulbhandle,1);

    // Start Schedule
    vTaskStartScheduler();
}

void initializeAllHWTask(void *pvParameters){
    // Convert to Parts
    parts_t *partPtr = (parts_t*)pvParameters; 

    // Initialize Components
    initializeSlideSwitch(&partPtr->slide_switch);
    initializeButton(&partPtr->button);
    initializeBuzzer(&partPtr->buzzer);
    initializeRTC(&partPtr->rtc);
    initializeLCD(&partPtr->lcd);

    // Intialize Current and Alarm Time
    ESP_LOGI(TAG,"Set Default Time to RTC");

    partPtr->rtc.writeTime(
        partPtr->currentTime[0], // hour = 0100 1100 -> 12AM
        partPtr->currentTime[1], // minute = 0x00 -> 00
        partPtr->currentTime[2], // Second = 0x00 -> 00
        partPtr->currentTime[3] 
    );
    
    // Set Event
    xEventGroupSetBits(eventGroup,EVT_HW_INITIALIZED);
    xEventGroupSetBits(eventGroup,WIFI_CONFIG);
}

void checkControlMode(void *pvParameters){
    while(true){
        // Check Event Group
        ESP_LOGI(TAG, "Event Group State %d",(int)xEventGroupGetBits(eventGroup));
        
        ESP_LOGI(TAG,"checkControlMode : Wait For hardware to be initialized");
        EventBits_t bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED,pdFALSE,pdFALSE,portMAX_DELAY);
        if(bits&&EVT_HW_INITIALIZED==EVT_HW_INITIALIZED){
            ESP_LOGI(TAG,"CM Wait success");
        }else{
            ESP_LOGI(TAG,"CM Wait Failed");
            continue;
        }
        ESP_LOGI(TAG,"checkControlMode : Hardware to be initialized");

        // Extract variables
        // esp_task_wdt_reset();
        parts_t *partPtr = (parts_t*)(pvParameters);
        slide_switch_t slideSwitch = partPtr->slide_switch;
        
        clock_mode_t clockMode = slideSwitch.mode();
        ESP_LOGI(TAG,"Clock Mode : %d",clockMode);
        

        // Choose Time Tasks
        switch (clockMode){
        case CLOCK:
            ESP_LOGI(TAG,"Set to Clock Mode");
                xEventGroupSetBits(eventGroup,EVT_CLOCK_MODE);
            ESP_LOGI(TAG, "checkControlMode : Event Group State %d",(int)xEventGroupGetBits(eventGroup));
            taskYIELD(); // Yield and Allow Other States to try
            break;
        case TIME:
            ESP_LOGI(TAG,"Set to Time mode");
            xEventGroupSetBits(eventGroup,EVT_TIME_MODE);
            ESP_LOGI(TAG, "checkControlMode : Event Group State %d",(int)xEventGroupGetBits(eventGroup));
            taskYIELD(); // Yield and Allow Other States to try
            break;
        case ALARM:
            ESP_LOGI(TAG,"Set to Alarm mode");
            xEventGroupSetBits(eventGroup,EVT_ALARM_MODE);
            ESP_LOGI(TAG, "checkControlMode : Event Group State %d",(int)xEventGroupGetBits(eventGroup));
            taskYIELD(); // Yield and Allow Other States to try
            break;
        default:
            ESP_LOGE(TAG,"SlideSwitch not Working");
            break;
        }
    }
    vTaskDelete(checkControlModeHandle);
}

void changeCurrentTime(void *pvParameters){
    // Update Current Time
    while (true){
       // Check Event Group
       ESP_LOGI(TAG, "changeAlarmTime : Event Group State %d",(int)xEventGroupGetBits(eventGroup));

       ESP_LOGI(TAG, "changeAlarmTime : Wait for HardWare to be initialized");
       EventBits_t bits=  xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED | EVT_TIME_MODE ,pdFALSE,pdFALSE,100); // Update Clock Time
       if(bits&&(EVT_HW_INITIALIZED | EVT_TIME_MODE)==(EVT_HW_INITIALIZED | EVT_TIME_MODE)){
        ESP_LOGI(TAG,"CT Wait success");
        }else{
        ESP_LOGI(TAG,"CT Wait Failed");
        continue;
        }
       ESP_LOGI(TAG, "changeAlarmTime : HardWare initialized");

       // Log
       ESP_LOGI(TAG,"changeAlarmTime: Change Current Alarm Task");
        
        parts_t *part = (parts_t*)pvParameters;
        button_t button = part->button;
        lcd_t lcd = part->lcd;
        uint8_t timeIndex = 0;
        char time_str[10];


        // Act based on button pressed
        int64_t upPressTime = button.pressDuration(UP_BUTTON);
        int64_t downPressTime = button.pressDuration(DOWN_BUTTON);
        int64_t selectPressTime = button.pressDuration(SELECT_BUTTON);
        if(timeIndex < 3){
            uint8_t currentTime = part->currentTime[timeIndex];
            if(button.pressed(UP_BUTTON) || upPressTime > 100){ // Up Button
                ESP_LOGI(TAG,"Pressed Up Button");
                updateTime(part,currentTime,timeIndex);
            }
            else if(button.pressed(DOWN_BUTTON) || downPressTime > 100){ // Down Button
                ESP_LOGI(TAG,"Pressed Down Button");
                updateTime(part,currentTime,timeIndex);
            }
            else if(button.pressed(SELECT_BUTTON) || selectPressTime > 100){ // Select Button
                ESP_LOGI(TAG,"Pressed Select Button");
                timeIndex++;
            }
        }
        else{
            timeIndex = 0;
        }

        ESP_LOGI(
            TAG,
            "Display Time : %d:%d:%d",
            part->currentTime[0],
            part->currentTime[1],
            part->currentTime[2]
        );

        // Display Time to LCD
        ESP_LOGI(TAG,"Display Time on LCD");
        snprintf(time_str, sizeof(time_str), "%02d:%02d", part->currentTime[0], part->currentTime[1]);
        lcd.display((void*)(&lcd),lcdCharacters,0,10,time_str);
        lcd.display((void*)(&lcd),lcdCharacters,0,20,((part->currentTime[3] == 0)? "am" : "pm"));
        

        xEventGroupClearBits(eventGroup,EVT_TIME_MODE);
        taskYIELD(); // Allow it to switch to another function
    }
    
    
}

void changeAlarmTime(void *pvParameters){
    // Change Time
    while(true){
        // Check Event Group
        ESP_LOGI(TAG, "changeAlarmTime : Event Group State %d",(int)xEventGroupGetBits(eventGroup));
        ESP_LOGI(TAG,"Task high water mark: %d bytes\n", (int)uxTaskGetStackHighWaterMark(NULL));

        ESP_LOGI(TAG, "changeAlarmTime : Wait for HardWare to be initialized");
        EventBits_t bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED | EVT_ALARM_MODE ,pdFALSE,pdFALSE,1000); // Update Clock Time
        if(bits&&(EVT_HW_INITIALIZED | EVT_ALARM_MODE)==(EVT_HW_INITIALIZED | EVT_ALARM_MODE)){
            ESP_LOGI(TAG,"AT Wait success");
            }else{
                ESP_LOGI(TAG,"AT Wait Failed");
                taskYIELD();
    
            }
            
        ESP_LOGI(TAG, "changeAlarmTime : HardWare initialized");

        // Log
        ESP_LOGI(TAG,"changeAlarmTime: Change Current Alarm Task");
        
        parts_t *part = (parts_t*)pvParameters;
        button_t button = part->button;
        lcd_t lcd = part->lcd;
        uint8_t timeIndex = 0;
        char time_str[10];
        
        // Act based on button pressed
        int64_t upPressTime = button.pressDuration(UP_BUTTON);
        int64_t downPressTime = button.pressDuration(DOWN_BUTTON);
        int64_t selectPressTime = button.pressDuration(SELECT_BUTTON);

        if(timeIndex < 3){
            uint8_t alarmTime = part->alarmTime[timeIndex];
            if(button.pressed(UP_BUTTON) || upPressTime > 100){ // Up Button
                ESP_LOGI(TAG,"Pressed Up Button");
                updateTime(part,alarmTime,timeIndex);
            }
            else if(button.pressed(DOWN_BUTTON) || downPressTime > 100){ // Down Button
                ESP_LOGI(TAG,"Pressed Down Button");
                updateTime(part,alarmTime,timeIndex);
            }
            else if(button.pressed(SELECT_BUTTON) || selectPressTime > 100){ // Select Button
                ESP_LOGI(TAG,"Pressed Select Button");
                timeIndex++;
            }
        }
        else{
            timeIndex = 0;
        }

        // Display Time to LCD
        
        ESP_LOGI(TAG,"Display Time on LCD");
        snprintf(time_str, sizeof(time_str), "%02d:%02d", part->alarmTime[0], part->alarmTime[1]);
        lcd.display((void*)(&lcd),lcdCharacters,1,10,time_str);
        lcd.display((void*)(&lcd),lcdCharacters,1,20,((part->alarmTime[3] == 0)? "am" : "pm"));
        

        // Reset index
        if(timeIndex >=3){
            timeIndex = 0;
        }

        xEventGroupClearBits(eventGroup,EVT_ALARM_MODE);
        taskYIELD();

    }    
}

void readClockTime(void *pvParameters){
   // Log
   ESP_LOGI(TAG,"Read Clock Time");
   
    // Components
   parts_t *part = (parts_t*)pvParameters;

   // Isolated Needed HW
   internal_real_time_clock_t rtc = part->rtc;
   lcd_t lcd = part->lcd;
   buzzer_t buzzer = part->buzzer;
   char time_str[10];

   // Read Clock Time
   while(true){
    EventBits_t bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED|EVT_CLOCK_MODE,pdFALSE,pdFALSE,portMAX_DELAY); // Do Not Read until Read Time Clock Time event is active
    if(bits&&(EVT_HW_INITIALIZED|EVT_CLOCK_MODE)==(EVT_HW_INITIALIZED|EVT_CLOCK_MODE)){
        ESP_LOGI(TAG,"CT Wait success");
    }else{
        ESP_LOGI(TAG,"CT Wait Failed");
        continue;
    }
    xEventGroupClearBits(eventGroup,READ_TIME_BIT);

    ESP_LOGI(TAG,"Read Time from RTC Component");
    rtc.readTime(
        &part->currentTime[0],
        &part->currentTime[1],
        &part->currentTime[2],
        &part->currentTime[3]
    );

    // Display Time on LCD
    ESP_LOGI(TAG,"Display Time on LCD");
    snprintf(time_str, sizeof(time_str), "%02d:%02d", part->currentTime[0], part->currentTime[1]);
    lcd.display((void*)(&lcd),lcdCharacters,0,10,time_str);
    lcd.display((void*)(&lcd),lcdCharacters,0,20,((part->currentTime[3] == 0)? "am" : "pm"));
    xEventGroupSetBits(eventGroup,READ_TIME_BIT);

    // Setup Alarm
    uint8_t triggerAlarm = (part->currentTime[0] == part->alarmTime[0]) & (part->currentTime[3] == part->alarmTime[3]); // Check for Hour and AM & PM
    triggerAlarm &= (part->currentTime[1] == part->alarmTime[1]) || (part->currentTime[1] <= part->alarmTime[1] + 2); // Check for Minutes
    if(triggerAlarm){
        ESP_LOGI(TAG,"Turn ON Buzzer");
        xEventGroupSetBits(eventGroup,BUZZER_ON);
        alarmActive = 1;
    }
    else{
        ESP_LOGI(TAG,"Turn OFF Buzzer");
        buzzer.powerOn(BUZZER_MODE,BUZZER_CHANNEL,NONE); // Kill Alarm
        xEventGroupClearBits(eventGroup,BUZZER_ON);
    }
   }
}

void turnOnBuzzer(void *pvParameters){
    // Turn On Buzzer
    while(true){
        parts_t *part = (parts_t*)pvParameters;
        buzzer_t buzzer = part->buzzer;
        int64_t buzzerStartTime = 0, duration = 0;
        int powerLevel = LOW;

        xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED|EVT_CLOCK_MODE|BUZZER_ON,pdFALSE,pdFALSE,portMAX_DELAY); // Do Not Read until Read Time Clock Time event is active
    
        // Get Buzzer Levels
        if(buzzerStartTime == 0 || duration >= 1000){ // Notify that Buzzer is ON
            buzzerStartTime = esp_timer_get_time(); // Record Current Time
            powerLevel++;
        }
        else{
            duration = esp_timer_get_time() - buzzerStartTime;
        }

        // Setup PWM
        if(powerLevel == LOW || powerLevel == MID || powerLevel == HIGH || powerLevel == MAX){
            ESP_LOGI(TAG,"Increase Power");
            buzzer.powerOn(BUZZER_MODE,BUZZER_CHANNEL,(buzzer_power_t)powerLevel);
            vTaskDelay(pdMS_TO_TICKS(200));
            buzzer.powerOn(BUZZER_MODE,BUZZER_CHANNEL,NONE);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

IRAM_ATTR void setWifi(void *pvParameters){
    // synchronizeWait(WIFI_CONFIG); //makes sure that the wifi SSID and Password are set (needs to be done later with BLE data from phone)
    // initialize Wifi
    ESP_LOGI(TAG,"wifi init");
    //wifi_init();

    // Configure Wifi
    ESP_LOGI(TAG,"wifi config");
    parts_t *cfg=(parts_t*)pvParameters;
    wifi_configuration(cfg->ssid,cfg->pass);
    

    // Start Wifi
    ESP_LOGI(TAG,"wifi start");
    wifi_start();

    // synchronizeSet(WIFI_CONNECTED);
    ESP_LOGI(TAG,"wifi connection");
}


void bulbthread(void *pvParameters){
    
    while(true){
        parts_t *part = (parts_t*)pvParameters;
        // Do Not Read until Read Time Clock Time event is active
        xEventGroupWaitBits(eventGroup,BULB_CONFIG,pdFALSE,pdFALSE,portMAX_DELAY); 
        int *mode = part->bulbmode;
        char* key= part->key;
        char* sku= part->sku;
        char* device=part->device;
        switch (*mode){
            case BULB_CONFIG:
                bconfig(sku,device,key);
                taskYIELD();
                break;
            case BULB_ON:
                ESP_LOGI(TAG,"Turn light on");
                sendRequest(1);
                taskYIELD();
                break;
            case BULB_OFF:
                ESP_LOGI(TAG,"Turn light off");
                sendRequest(0);
                taskYIELD();
                break;
            default:
                taskYIELD();
                break;
            }
    }
}

void updateTime(parts_t *part, uint8_t time, uint8_t index){
    if(time >= 13 && index == 0){
        part->currentTime[index] = 1;
        part->currentTime[3] = (part->currentTime[3] == 1)? 0:1; // Toggle AM to PM and vice-versa
    }
    else if(time > 59 && index != 0){
        part->currentTime[index] = 0;
    }
    else{
        part->currentTime[index]++;
    }

}