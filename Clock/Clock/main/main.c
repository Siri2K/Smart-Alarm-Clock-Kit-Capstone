/* Components Headers */
#include "Button.h"
#include "Buzzer.h"
// #include "LCD.h"
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
#define BUZZER_ON       (EventBits_t)(1<<5) // Buzzer Activates
#define BUZZER_SNOOZED  (EventBits_t)(1<<6) // Buzzer Snoozed


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
    // lcd_t lcd;
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


/* Static Function */
static void chooseClockMode(clock_mode_t clockMode){
    switch (clockMode){
        case CLOCK:
            ESP_LOGI(TAG,"Set to Clock Mode");
            xEventGroupSetBits(eventGroup,EVT_CLOCK_MODE);
            ESP_LOGI(TAG, "chooseClockMode : Event Group State %d",(int)xEventGroupGetBits(eventGroup));
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait 100ms 
            break;
        case TIME:
            ESP_LOGI(TAG,"Set to Time mode");
            xEventGroupSetBits(eventGroup,EVT_TIME_MODE);
            ESP_LOGI(TAG, "chooseClockMode : Event Group State %d",(int)xEventGroupGetBits(eventGroup));
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait 100ms 
            break;
        case ALARM:
            ESP_LOGI(TAG,"Set to Alarm mode");
            xEventGroupSetBits(eventGroup,EVT_ALARM_MODE);
            ESP_LOGI(TAG, "chooseClockMode : Event Group State %d",(int)xEventGroupGetBits(eventGroup));
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait 100ms 
            break;
        default:
            ESP_LOGE(TAG,"chooseClockMode : SlideSwitch not Working");
            break;
        }
}

static void changeTimeDigit(uint8_t *time, uint8_t index){
    if(time[index] >= 13 && index == 0){
        time[index] = 1;
        time[3] = (time[3] == 1)? 0:1; // Toggle AM to PM and vice-versa
    }
    else if(time[index] > 59 && index != 0){
		time[index] = 0;
    }
    else{
		time[index]++;
    }
}

static void updateTime(uint8_t *time, int64_t*buttonPressTimes){
	// Update Current Time based on PressTime
	uint8_t timeIndex = 0;
	
	if(timeIndex < 3){
		if(buttonPressTimes[0] > 100){ // Up Button
			ESP_LOGI(TAG,"Pressed Up Button");
			changeTimeDigit(time,timeIndex);
		}
		else if(buttonPressTimes[1] > 100){ // Down Button
			ESP_LOGI(TAG,"Pressed Down Button");
			changeTimeDigit(time,timeIndex);
		}
		else if(buttonPressTimes[2] > 100){ // Select Button
			ESP_LOGI(TAG,"Pressed Select Button");
			timeIndex++;
		}
	}
	else{
		timeIndex = 0;
	}
	vTaskDelay(pdMS_TO_TICKS(100));
}

static void trigerBuzzer(buzzer_t buzzer, uint8_t*currentTime, uint8_t*alarmTime){
	uint8_t matching_am_or_pm = currentTime[3] == alarmTime[3];
	uint8_t matching_hrs = currentTime[0] == alarmTime[0];
	uint8_t matching_minute = currentTime[1] == alarmTime[1] || currentTime[1] <= alarmTime[1] + 2;

	// Trigger Buzzer
	if(matching_am_or_pm && matching_hrs && matching_minute){
		ESP_LOGI(TAG,"Turn On Buzzer");
		xEventGroupSetBits(eventGroup, BUZZER_ON);
	}
	else{
		ESP_LOGI(TAG,"Turn OFF Buzzer");
		xEventGroupSetBits(eventGroup, BUZZER_SNOOZED);
	}
	vTaskDelay(pdMS_TO_TICKS(100));
}

/* Function Prototypes */
// Control Task
void ControlTask();

// Computation Tasks
void initializeAllHWTask(void *pvParameters);
void checkControlMode(void *pvParameters);
void changeCurrentTime(void *pvParameters);
void changeAlarmTime(void *pvParameters);
void readClockTime(void *pvParameters);
void turnOnBuzzer(void *pvParameters);
// void setWifi(void *pvParameters);
// void bulbthread(void *pvParameters);


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
    
    parts.ssid= WifiSSID; //code won't work currently if this isn't set
    parts.pass=WifiPassword; //code won't work currently if this isn't set
    parts.sku=WifiSku;
    parts.bulbmode=0;
    parts.device=WifiMacAddress;
    parts.key = WifiApiKey;

    wifi_init();
    wifi_configuration(parts.ssid,parts.pass);
    wifi_start();

    ControlTask();
}

void ControlTask(){   
    // Variables
    eventGroup = xEventGroupCreate();

    // Clear All EVENTS
    xEventGroupClearBits(eventGroup,EVT_HW_INITIALIZED);
    xEventGroupClearBits(eventGroup,EVT_TIME_MODE|EVT_ALARM_MODE|EVT_CLOCK_MODE);

    /* Create All Tasks */
    // Initalize All HW
    xTaskCreate(initializeAllHWTask,"HardWare Initialization", sizeof(parts_t),(void*)&parts, 2, &initializeAllHWHandle);
    // Create All Functionality tasks
    xTaskCreate(checkControlMode,"Check Clock Mode", sizeof(parts_t),(void*)&parts,1,&checkControlModeHandle);
    xTaskCreate(readClockTime,"Read Current Time", sizeof(parts_t),(void*)&parts,1,&readClockTimeHandle);
    xTaskCreate(changeCurrentTime,"Change Curent Time", sizeof(parts_t),(void*)&parts,1,&changeCurrentTimeHandle);
    xTaskCreate(changeAlarmTime,"Change Alarm Time", sizeof(parts_t),(void*)&parts,1,&changeAlarmTimeHandle);
    xTaskCreate(turnOnBuzzer,"Turn On Buzzer",sizeof(parts_t),(void*)&parts,1,&setbulbhandle);
    // xTaskCreate(bulbthread,"Bulb Thread",sizeof(parts_t),(void*)&parts,1,&setbulbhandle);

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
    //initializeLCD(&partPtr->lcd);

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
    // Define Variables
    EventBits_t bits;
    parts_t *partPtr;
    
    while(true){
        // Check Event Group
        ESP_LOGI(TAG, "CheckControlMode Function");
        bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED,pdFALSE,pdTRUE,pdMS_TO_TICKS(60000));// Check for 6s
        
        if(bits == EVT_HW_INITIALIZED){
            ESP_LOGI(TAG,"CheckControlMode : Hardware initialized");
            
            // Initialize Synchronized variables
            partPtr = (parts_t*)pvParameters;

            // Setup Clock Modes
            chooseClockMode(partPtr->slide_switch.mode());
        }
        else{
            ESP_LOGI(TAG,"CheckControlMode : Hardware Not Initialized");
        }
        taskYIELD(); // Allow Other Tasks to start
    }

    // Kill After Turning off
    vTaskDelete(checkControlModeHandle);
}

void changeCurrentTime(void *pvParameters){
    // Update Current Time
    EventBits_t bits;
    parts_t *part;
    
    char time_str[10];
	int64_t buttonPressTimes[3];
    // lcd_t lcd = part->lcd;
    
    while (true){
       // Check Event Group
       ESP_LOGI(TAG, "ChangeCurrentTime Function");
       
       // Wait for Bits
       ESP_LOGI(TAG, "changeCurrentTime : Wait for HardWare + Time mode to be set");
       bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED | EVT_TIME_MODE ,pdFALSE,pdFALSE,60000);

       if(bits |= (EVT_HW_INITIALIZED | EVT_TIME_MODE)){ // Check EVT_HW_INITIALIZED & EVT_TIME_MODE are set
            ESP_LOGI(TAG, "changeCurrentTime : HardWare + Time mode are set");
			
			// Synchronize Variables
			part = (parts_t*)pvParameters;

			// Update Displayed Time
			buttonPressTimes[0] = part->button.pressDuration(UP_BUTTON);
			buttonPressTimes[1] = part->button.pressDuration(DOWN_BUTTON);
			buttonPressTimes[2] = part->button.pressDuration(SELECT_BUTTON);
			updateTime(part->currentTime,buttonPressTimes);

			// Display Time
			ESP_LOGI(
				TAG,
				"Display Time : %d:%d:%d",
				part->currentTime[0],
				part->currentTime[1],
				part->currentTime[2]
			);

			// Clear Bit
			xEventGroupClearBits(eventGroup,EVT_TIME_MODE);

       }
	   else if (bits == EVT_HW_INITIALIZED){
			ESP_LOGI(TAG, "changeCurrentTime : HardWare only is set");

			// Display Time
			ESP_LOGI(
				TAG,
				"Display Time : %d:%d:%d",
				part->currentTime[0],
				part->currentTime[1],
				part->currentTime[2]
			);
	   }
	   else{
			ESP_LOGI(TAG, "changeCurrentTime : No Events set");
	   }
		
        // Display Time to LCD
        /*
        ESP_LOGI(TAG,"Display Time on LCD");
        snprintf(time_str, sizeof(time_str), "%02d:%02d", part->currentTime[0], part->currentTime[1]);
        lcd.display((void*)(&lcd),lcdCharacters,0,10,time_str);
        lcd.display((void*)(&lcd),lcdCharacters,0,20,((part->currentTime[3] == 0)? "am" : "pm"));
        */
        
        taskYIELD(); // Allow it to switch to another function
    }

	// Kill After Turning off
    vTaskDelete(changeCurrentTimeHandle);
}

void changeAlarmTime(void *pvParameters){
    // Update Current Time
    EventBits_t bits;
    parts_t *part;
    
    char time_str[10];
	int64_t buttonPressTimes[3];
    // lcd_t lcd = part->lcd;
    
    while (true){
       // Check Event Group
       ESP_LOGI(TAG, "changeAlarmTime Function");
       
       // Wait for Bits
       ESP_LOGI(TAG, "changeAlarmTime : Wait for HardWare + Alarm mode to be set");
       bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED | EVT_ALARM_MODE ,pdFALSE,pdFALSE,60000);

       if(bits |= (EVT_HW_INITIALIZED | EVT_ALARM_MODE)){ // Check EVT_HW_INITIALIZED & EVT_TIME_MODE are set
            ESP_LOGI(TAG, "changeCurrentTime : HardWare + Alarm mode are set");
			
			// Synchronize Variables
			part = (parts_t*)pvParameters;

			// Update Displayed Time
			buttonPressTimes[0] = part->button.pressDuration(UP_BUTTON);
			buttonPressTimes[1] = part->button.pressDuration(DOWN_BUTTON);
			buttonPressTimes[2] = part->button.pressDuration(SELECT_BUTTON);
			updateTime(part->alarmTime,buttonPressTimes);

			// Display Time
			ESP_LOGI(
				TAG,
				"Display Time : %d:%d:%d",
				part->alarmTime[0],
				part->alarmTime[1],
				part->alarmTime[2]
			);

			// Clear Bit
			xEventGroupClearBits(eventGroup,EVT_TIME_MODE);

       }
	   else if (bits == EVT_HW_INITIALIZED){
			ESP_LOGI(TAG, "changeCurrentTime : HardWare only is set");

			// Display Time
			ESP_LOGI(
				TAG,
				"Display Time : %d:%d:%d",
				part->currentTime[0],
				part->currentTime[1],
				part->currentTime[2]
			);
	   }
	   else{
			ESP_LOGI(TAG, "changeCurrentTime : No Events set");
	   }
		
        // Display Time to LCD
        /*
        ESP_LOGI(TAG,"Display Time on LCD");
        snprintf(time_str, sizeof(time_str), "%02d:%02d", part->currentTime[0], part->currentTime[1]);
        lcd.display((void*)(&lcd),lcdCharacters,0,10,time_str);
        lcd.display((void*)(&lcd),lcdCharacters,0,20,((part->currentTime[3] == 0)? "am" : "pm"));
        */
        
        taskYIELD(); // Allow it to switch to another function
    }
	
	// Kill After Turning off
    vTaskDelete(changeAlarmTimeHandle);
}

void readClockTime(void *pvParameters){
  // Update Current Time
   parts_t *part;
   EventBits_t bits;

   // Isolated Needed HW
   // lcd_t lcd = part->lcd;
   char time_str[10];

   // Read Clock Time
   while(true){
		ESP_LOGI(TAG,"readClockTime Function");
		
		// Wait for Bits
		ESP_LOGI(TAG, "changeAlarmTime : Wait for HardWare + Clock mode to be set");
		bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED|EVT_CLOCK_MODE,pdFALSE,pdFALSE,60000); // Check Clock Time
		if(bits |= (EVT_HW_INITIALIZED|EVT_CLOCK_MODE)){
			ESP_LOGI(TAG, "changeCurrentTime : HardWare + Clock mode are set");

			// Synchronize Variables
			part = (parts_t*)pvParameters;

			// Read the Time
			part->rtc.readTime(
				&part->currentTime[0],
				&part->currentTime[1],
				&part->currentTime[2],
				&part->currentTime[3]
			);

			// Trigger Alarm
			trigerBuzzer(part->buzzer,part->currentTime, part->alarmTime);
			taskYIELD();
		}
	}

	// Kill task After Turning Off
    vTaskDelete(readClockTimeHandle);

}

void turnOnBuzzer(void *pvParameters){
    // Turn On Buzzer
    EventBits_t bits;
	parts_t *part;
	int powerLevel = LOW;
	int64_t buzzerStartTime = 0, duration = 0;
	
	while(true){
		ESP_LOGI(TAG,"turnOnBuzzer Function");

		// Wait for Bits
		ESP_LOGI(TAG, "turnOnBuzzer : Wait for HardWare + Clock mode to be set");
		bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED|EVT_CLOCK_MODE|BUZZER_ON|BUZZER_SNOOZED,pdFALSE,pdFALSE,60000); // Check Clock Time
		if(bits |= EVT_HW_INITIALIZED|EVT_CLOCK_MODE|BUZZER_ON){
			ESP_LOGI(TAG, "turnOnBuzzer : HardWare + Clock mode + Buzzer On set");

            part = (parts_t*)pvParameters;

			// Get Buzzer Levels
			if(buzzerStartTime == 0 || (duration % 10000000 == 0)){ // Increemnt Time every 10s
				buzzerStartTime = esp_timer_get_time(); // Record Current Time
                if(powerLevel >= MAX){
                    powerLevel = MAX;
                }
                else{
                    powerLevel++;
                }
			}
			else{
				duration = esp_timer_get_time() - buzzerStartTime;
			}
	
			// Setup PWM
			if(powerLevel == LOW || powerLevel == MID || powerLevel == HIGH || powerLevel == MAX){
				ESP_LOGI(TAG,"Increase Power");
				part->buzzer.powerOn(BUZZER_MODE,BUZZER_CHANNEL,(buzzer_power_t)powerLevel);
				vTaskDelay(pdMS_TO_TICKS(200));
			}
		}
		else if(bits |= EVT_HW_INITIALIZED|EVT_CLOCK_MODE|BUZZER_SNOOZED){
			ESP_LOGI(TAG, "turnOnBuzzer : HardWare + Clock mode + Buzzer Snoozed set");
			part->buzzer.powerOn(BUZZER_MODE,BUZZER_CHANNEL,NONE);
			vTaskDelay(pdMS_TO_TICKS(200));
		}
		else if(bits |= EVT_HW_INITIALIZED|EVT_CLOCK_MODE){
			ESP_LOGI(TAG, "turnOnBuzzer : HardWare + Clock mode set");
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else if(bits |= EVT_HW_INITIALIZED){
			ESP_LOGI(TAG, "turnOnBuzzer : HardWare set");
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else{
			ESP_LOGI(TAG, "turnOnBuzzer : Nothing Set mode set");
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		taskYIELD();
    }
}

/*
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
*/
