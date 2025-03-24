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
#define EVT_BUZZER_ON       (EventBits_t)(1<<5) // Buzzer Activates
#define EVT_BUZZER_SNOOZED  (EventBits_t)(1<<6) // Buzzer Snoozed

// BLE Initialized
#define EVT_BLE_INITIALIZED     (EventBits_t)(1<<7) // Confirms Bulb is configured
#define EVT_BLE_RECEIVED_WIFI_DATA    (EventBits_t)(1<<7) // Confirms Bulb is configured

// WIFI
#define EVT_EVT_WIFI_CONNECTED  (EventBits_t)(1<<8) // Verify Wifi Connection
#define EVT_WIFI_FAIL       (EventBits_t)(1<<9) // Verify Wifi fail
#define EVT_WIFI_CONNECT    (EventBits_t)(1<<10) // Wifi connect
#define EVT_WIFI_CONFIG     (EventBits_t)(1<<11) // Confirms Wifi is Configured

#define EVT_BULB_CONFIG     (EventBits_t)(1<<12) // Confirms Bulb is configured


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
// Part
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

TaskHandle_t turnOnBuzzerHandle; // RTC Handle

TaskHandle_t startBLEHandle; // BLE handle
TaskHandle_t setupWifiHandle; // Setup Wifi
TaskHandle_t startWifiHandle; // Start Wifi
TaskHandle_t controlBulbHandle; // Control Wifi

// Logging
static const char *TAG = "Clock Log:";


/* Static Function */
static void chooseClockMode(clock_mode_t clockMode){
    switch (clockMode){
        case CLOCK:
            ESP_LOGI(TAG,"Set to Clock Mode");
            xEventGroupSetBits(eventGroup,EVT_CLOCK_MODE);
            ESP_LOGI(TAG, "chooseClockMode : Event Group State %d",(int)xEventGroupGetBits(eventGroup));
            vTaskDelay(100/portTICK_PERIOD_MS); // Wait 100ms 
            break;
        case TIME:
            ESP_LOGI(TAG,"Set to Time mode");
            xEventGroupSetBits(eventGroup,EVT_TIME_MODE);
            ESP_LOGI(TAG, "chooseClockMode : Event Group State %d",(int)xEventGroupGetBits(eventGroup));
            vTaskDelay(100/portTICK_PERIOD_MS); // Wait 100ms 
            break;
        case ALARM:
            ESP_LOGI(TAG,"Set to Alarm mode");
            xEventGroupSetBits(eventGroup,EVT_ALARM_MODE);
            ESP_LOGI(TAG, "chooseClockMode : Event Group State %d",(int)xEventGroupGetBits(eventGroup));
            vTaskDelay(100/portTICK_PERIOD_MS); // Wait 100ms 
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
	vTaskDelay(100/portTICK_PERIOD_MS);
}

static void triggerBuzzer(buzzer_t buzzer, uint8_t*currentTime, uint8_t*alarmTime){
	uint8_t matching_am_or_pm = currentTime[3] == alarmTime[3];
	uint8_t matching_hrs = currentTime[0] == alarmTime[0];
	uint8_t matching_minute = currentTime[1] == alarmTime[1] || currentTime[1] <= alarmTime[1] + 2;

	// Trigger Buzzer
	if(matching_am_or_pm && matching_hrs && matching_minute){
		ESP_LOGI(TAG,"Turn On Buzzer");
		xEventGroupSetBits(eventGroup, EVT_BUZZER_ON);
	}
	else{
		ESP_LOGI(TAG,"Turn OFF Buzzer");
		xEventGroupSetBits(eventGroup, EVT_BUZZER_SNOOZED);
	}
	vTaskDelay(100/portTICK_PERIOD_MS);
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
void startBLE(void *pvParameters);
void setupWifi(void *pvParameters);
void startWifi(void *pvParameters);
void controlBulb(void *pvParameters);

/* Main Functions */
void app_main(void){
    esp_err_t status = ESP_OK;

    // Initialize NVS
    status = nvs_flash_init();
    while(status != ESP_OK){
        if (status == ESP_ERR_NVS_NO_FREE_PAGES || status == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            status = nvs_flash_init();
        }
    }
    
    // Release Uneccesary Bluetooth emory
    status = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if(status != ESP_OK){
        ESP_LOGI(TAG,"Bluetooth Memory Not Released");
    }



    // 2) Initialize Time
    ESP_LOGI(TAG,"Initialize Time");
    for(int i=0; i<3; i++){
        parts.currentTime[i] = (i==0)?12:0;
        parts.alarmTime[i] = (i==0)?12:0;
    }
    // 3) Store Time
    for(int i=0;i<4;i++){
        storedWatchData[i] = 0;
    }
    storedWatchData[4] = parts.currentTime[0];
    storedWatchData[5] = parts.currentTime[1];

    // Set Alarm Time to 12:01
    parts.alarmTime[1] = 1;

    // Control Task
    ControlTask();
}

void ControlTask(){   
    // Variables
    eventGroup = xEventGroupCreate();

    // Clear All EVENTS
    xEventGroupClearBits(eventGroup,EVT_HW_INITIALIZED);
    xEventGroupClearBits(eventGroup,EVT_TIME_MODE|EVT_ALARM_MODE|EVT_CLOCK_MODE);

    /* Create All Tasks */
    // Configure Wifi & BLE
    xTaskCreatePinnedToCore(startBLE,"Start the BLE",CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,5,&startBLEHandle,0);
    xTaskCreate(setupWifi,"Configure Wifi",CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,4,&setupWifiHandle);
    xTaskCreatePinnedToCore(startWifi,"Start the Wifi",CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,3,&startWifiHandle,1);
    xTaskCreate(controlBulb,"Control the LightBulb",CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,1,&controlBulbHandle);
    
    
    // Initalize All HW
    xTaskCreate(initializeAllHWTask,"HardWare Initialization", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL, 4, &initializeAllHWHandle);
    // Create All Functionality tasks
    xTaskCreate(checkControlMode,"Check Clock Mode", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,3,&checkControlModeHandle);
    xTaskCreate(readClockTime,"Read Current Time", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,2,&readClockTimeHandle);
    xTaskCreate(changeCurrentTime,"Change Curent Time", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,2,&changeCurrentTimeHandle);
    xTaskCreate(changeAlarmTime,"Change Alarm Time", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,2,&changeAlarmTimeHandle);
    xTaskCreate(turnOnBuzzer,"Turn On Buzzer",CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,1,&turnOnBuzzerHandle);

    xTaskCreatePinnedToCore(startBLE,"Start the BLE",CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,2,&startBLEHandle,0);
    xTaskCreatePinnedToCore(setupWifi,"Configure Wifi",CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,3,&setupWifiHandle,1);
    xTaskCreatePinnedToCore(startWifi,"Start the Wifi",CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,2,&startWifiHandle,1);
    xTaskCreate(controlBulb,"Control the LightBulb",CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE,NULL,1,&controlBulbHandle);

    // Start Schedule
    vTaskStartScheduler();
}

void initializeAllHWTask(void *pvParameters){
    // Initialize Components
    initializeSlideSwitch(&parts.slide_switch);
    initializeButton(&parts.button);
    initializeBuzzer(&parts.buzzer);
    initializeRTC(&parts.rtc);
    //initializeLCD(&parts.lcd);

    // Intialize Current and Alarm Time
    ESP_LOGI(TAG,"Set Default Time to RTC");

    parts.rtc.writeTime(
        parts.currentTime[0], // hour = 0100 1100 -> 12AM
        parts.currentTime[1], // minute = 0x00 -> 00
        parts.currentTime[2], // Second = 0x00 -> 00
        parts.currentTime[3] 
    );
    
    // Set Event
    xEventGroupSetBits(eventGroup,EVT_HW_INITIALIZED);
    xEventGroupSetBits(eventGroup,EVT_WIFI_CONFIG);

    // Yield 
    //taskYIELD();
    vTaskDelete(initializeAllHWHandle);
}

void checkControlMode(void *pvParameters){
    // Define Variables
    EventBits_t bits;
    
    while(true){
        // Check Event Group
        ESP_LOGI(TAG, "CheckControlMode Function");
        bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED,pdTRUE,pdFALSE,1000/portTICK_PERIOD_MS);// Check for 6s
        
        if((bits & EVT_HW_INITIALIZED) == EVT_HW_INITIALIZED){
            ESP_LOGI(TAG,"CheckControlMode : Hardware initialized");
            
            // Setup Clock Modes
            chooseClockMode(parts.slide_switch.mode());
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
    
    char time_str[10];
	int64_t buttonPressTimes[3];
    // lcd_t lcd = parts.lcd;
    
    while (true){
       // Check Event Group
       ESP_LOGI(TAG, "ChangeCurrentTime Function");
       
       // Wait for Bits
       ESP_LOGI(TAG, "changeCurrentTime : Wait for HardWare + Time mode to be set");
       bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED | EVT_TIME_MODE ,pdTRUE,pdFALSE,1000/portTICK_PERIOD_MS);

       if((bits & (EVT_HW_INITIALIZED | EVT_TIME_MODE)) == (EVT_HW_INITIALIZED | EVT_TIME_MODE)){ // Check EVT_HW_INITIALIZED & EVT_TIME_MODE are set
            ESP_LOGI(TAG, "changeCurrentTime : HardWare + Time mode are set");
			// Update Displayed Time
			buttonPressTimes[0] = parts.button.pressDuration(UP_BUTTON);
			buttonPressTimes[1] = parts.button.pressDuration(DOWN_BUTTON);
			buttonPressTimes[2] = parts.button.pressDuration(SELECT_BUTTON);
			updateTime(parts.currentTime,buttonPressTimes);

			// Display Time
			ESP_LOGI(
				TAG,
				"Display Time : %d:%d:%d",
				parts.currentTime[0],
				parts.currentTime[1],
				parts.currentTime[2]
			);

			// Clear Bit
			xEventGroupClearBits(eventGroup,EVT_TIME_MODE);

       }
	   else if ((bits & EVT_HW_INITIALIZED) == EVT_HW_INITIALIZED){
			ESP_LOGI(TAG, "changeCurrentTime : HardWare only is set");

			// Display Time
			ESP_LOGI(
				TAG,
				"Display Time : %d:%d:%d",
				parts.currentTime[0],
				parts.currentTime[1],
				parts.currentTime[2]
			);
	   }
	   else{
			ESP_LOGI(TAG, "changeCurrentTime : No Events set");
	   }
		
        // Display Time to LCD
        /*
        ESP_LOGI(TAG,"Display Time on LCD");
        snprintf(time_str, sizeof(time_str), "%02d:%02d", parts.currentTime[0], parts.currentTime[1]);
        lcd.display((void*)(&lcd),lcdCharacters,0,10,time_str);
        lcd.display((void*)(&lcd),lcdCharacters,0,20,((parts.currentTime[3] == 0)? "am" : "pm"));
        */
        
        taskYIELD(); // Allow it to switch to another function
    }

	// Kill After Turning off
    vTaskDelete(changeCurrentTimeHandle);
}

void changeAlarmTime(void *pvParameters){
    // Update Current Time
    EventBits_t bits;
    
    
    char time_str[10];
	int64_t buttonPressTimes[3];
    // lcd_t lcd = parts.lcd;
    
    while (true){
       // Check Event Group
       ESP_LOGI(TAG, "changeAlarmTime Function");
       
       // Wait for Bits
       ESP_LOGI(TAG, "changeAlarmTime : Wait for HardWare + Alarm mode to be set");
       bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED | EVT_ALARM_MODE ,pdTRUE,pdFALSE,1000/portTICK_PERIOD_MS);

       if((bits & (EVT_HW_INITIALIZED | EVT_ALARM_MODE)) == (EVT_HW_INITIALIZED | EVT_ALARM_MODE)){ // Check EVT_HW_INITIALIZED & EVT_TIME_MODE are set
            ESP_LOGI(TAG, "changeCurrentTime : HardWare + Alarm mode are set");
			
			// Synchronize Variables
			

			// Update Displayed Time
			buttonPressTimes[0] = parts.button.pressDuration(UP_BUTTON);
			buttonPressTimes[1] = parts.button.pressDuration(DOWN_BUTTON);
			buttonPressTimes[2] = parts.button.pressDuration(SELECT_BUTTON);
			updateTime(parts.alarmTime,buttonPressTimes);

			// Display Time
			ESP_LOGI(
				TAG,
				"Display Time : %d:%d:%d",
				parts.alarmTime[0],
				parts.alarmTime[1],
				parts.alarmTime[2]
			);

			// Clear Bit
			xEventGroupClearBits(eventGroup,EVT_TIME_MODE);

       }
	   else if ((bits & EVT_HW_INITIALIZED) == EVT_HW_INITIALIZED){
			ESP_LOGI(TAG, "changeCurrentTime : HardWare only is set");

            // Synchronize Variables
			

			// Display Time
			ESP_LOGI(
				TAG,
				"Display Time : %d:%d:%d",
				parts.currentTime[0],
				parts.currentTime[1],
				parts.currentTime[2]
			);
	   }
	   else{
			ESP_LOGI(TAG, "changeCurrentTime : No Events set");
	   }
		
        // Display Time to LCD
        /*
        ESP_LOGI(TAG,"Display Time on LCD");
        snprintf(time_str, sizeof(time_str), "%02d:%02d", parts.currentTime[0], parts.currentTime[1]);
        lcd.display((void*)(&lcd),lcdCharacters,0,10,time_str);
        lcd.display((void*)(&lcd),lcdCharacters,0,20,((parts.currentTime[3] == 0)? "am" : "pm"));
        */
        
        taskYIELD(); // Allow it to switch to another function
    }
	
	// Kill After Turning off
    vTaskDelete(changeAlarmTimeHandle);
}

void readClockTime(void *pvParameters){
  // Update Current Time
   
   EventBits_t bits;

   // Isolated Needed HW
   // lcd_t lcd = parts.lcd;
   char time_str[10];

   // Read Clock Time
   while(true){
		ESP_LOGI(TAG,"readClockTime Function");
		
		// Wait for Bits
		ESP_LOGI(TAG, "changeAlarmTime : Wait for HardWare + Clock mode to be set");
		bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED|EVT_CLOCK_MODE,pdTRUE,pdFALSE,1000/portTICK_PERIOD_MS); // Check Clock Time
		if((bits & (EVT_HW_INITIALIZED|EVT_CLOCK_MODE)) == (EVT_HW_INITIALIZED|EVT_CLOCK_MODE)){
			ESP_LOGI(TAG, "changeCurrentTime : HardWare + Clock mode are set");

			// Synchronize Variables
			

			// Read the Time
			parts.rtc.readTime(
				&parts.currentTime[0],
				&parts.currentTime[1],
				&parts.currentTime[2],
				&parts.currentTime[3]
			);

			// Trigger Alarm
			triggerBuzzer(parts.buzzer,parts.currentTime, parts.alarmTime);
			
		}
        else if((bits & EVT_HW_INITIALIZED) == EVT_HW_INITIALIZED){
            ESP_LOGI(TAG, "changeCurrentTime : HardWare is set");
            vTaskDelay(100/portTICK_PERIOD_MS);;
        }
        taskYIELD();
	}

	// Kill task After Turning Off
    vTaskDelete(readClockTimeHandle);

}

void turnOnBuzzer(void *pvParameters){
    // Turn On Buzzer
    EventBits_t bits;
	int powerLevel = LOW;
    int8_t count = 0;
	int64_t buzzerStartTime = 0, duration = 0;
	
	while(true){
		ESP_LOGI(TAG,"turnOnBuzzer Function");

		// Wait for Bits
		ESP_LOGI(TAG, "turnOnBuzzer : Wait for HardWare + Clock mode to be set");
		bits = xEventGroupWaitBits(eventGroup,EVT_HW_INITIALIZED|EVT_CLOCK_MODE|EVT_BUZZER_ON|EVT_BUZZER_SNOOZED,pdTRUE,pdFALSE,1000/portTICK_PERIOD_MS); // Check Clock Time
		if((bits & (EVT_HW_INITIALIZED|EVT_CLOCK_MODE|EVT_BUZZER_ON)) == (EVT_HW_INITIALIZED|EVT_CLOCK_MODE|EVT_BUZZER_ON)){
			ESP_LOGI(TAG, "turnOnBuzzer : HardWare + Clock mode + Buzzer On set");

			// Get Buzzer Levels
			if(buzzerStartTime == 0 || (duration % 10000000 == 0)){ // Increment Time every 10s
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
                count++;
				parts.buzzer.powerOn(BUZZER_MODE,BUZZER_CHANNEL,(buzzer_power_t)powerLevel);
				vTaskDelay(200/portTICK_PERIOD_MS);
			}
		}
		else if((bits & (EVT_HW_INITIALIZED|EVT_CLOCK_MODE|EVT_BUZZER_SNOOZED)) == (EVT_HW_INITIALIZED|EVT_CLOCK_MODE|EVT_BUZZER_SNOOZED)){
			ESP_LOGI(TAG, "turnOnBuzzer : HardWare + Clock mode + Buzzer Snoozed set");

			parts.buzzer.powerOn(BUZZER_MODE,BUZZER_CHANNEL,NONE);
			vTaskDelay(200/portTICK_PERIOD_MS);
		}
		else if((bits & (EVT_HW_INITIALIZED|EVT_CLOCK_MODE)) == (EVT_HW_INITIALIZED|EVT_CLOCK_MODE)){
			ESP_LOGI(TAG, "turnOnBuzzer : HardWare + Clock mode set");
			vTaskDelay(100/portTICK_PERIOD_MS);
		}
		else if((bits & EVT_HW_INITIALIZED) == EVT_HW_INITIALIZED){
			ESP_LOGI(TAG, "turnOnBuzzer : HardWare set");
			vTaskDelay(100/portTICK_PERIOD_MS);
		}
		else{
			ESP_LOGI(TAG, "turnOnBuzzer : Nothing Set mode set");
			vTaskDelay(100/portTICK_PERIOD_MS);
		}
		taskYIELD();
    }
    vTaskDelete(turnOnBuzzerHandle);
}

void startBLE(void *pvParameters){
    initializeBLE();
    xEventGroupSetBits(eventGroup,EVT_BLE_INITIALIZED);
    vTaskDelete(startBLEHandle);
}

void setupWifi(void *pvParameters){
    // Components
    EventBits_t bits;
    
    int8_t wifiConfigured[5] = {0};

    // Old Wifi Set
    char ssid[64];
    char pass[64];
    char sku[64];
    char device[64];
    char key[64];

    
    while(true){
        ESP_LOGI(TAG,"setupWifi Function");
        
        // Wait for Bits
		ESP_LOGI(TAG, "setupWifi : Wait for BLE to be initialized to be set");
        bits = xEventGroupWaitBits(eventGroup,EVT_BLE_INITIALIZED,pdTRUE,pdFALSE,1000/portTICK_PERIOD_MS); // Check Clock Time    
        if((bits & EVT_BLE_INITIALIZED) == EVT_BLE_INITIALIZED){
            // Log BLE Intiialization
            ESP_LOGI(TAG, "setupWifi : BLE intiialized");

            // Configure Each Wifi Part
            if(strcmp(WifiSSID,ssid) != 0){
                ESP_LOGI(TAG, "SSID = %s",WifiSSID);
                parts.ssid = ssid;
                wifiConfigured[0] = 1;
            }

            if(strcmp(WifiPassword,pass) != 0){
                ESP_LOGI(TAG, "Password = %s",WifiPassword);
                parts.pass = pass;
                wifiConfigured[1] = 1;
            }

            if(strcmp(WifiSku,sku) != 0){
                ESP_LOGI(TAG, "SKU = %s",WifiSku);
                parts.sku = sku;
                wifiConfigured[2] = 1;
            }

            if(strcmp(WifiMacAddress,device) != 0){
                ESP_LOGI(TAG, "Wifi Mac Address = %s",WifiMacAddress);
                parts.device = device;
                wifiConfigured[3] = 1;
            }

            if(strcmp(WifiApiKey,ssid) != 0){
                ESP_LOGI(TAG, "API Key = %s",WifiApiKey);
                parts.key = key;
                wifiConfigured[4] = 1;
            }

            // Check If Wifi is Set Configured
            if(wifiConfigured[0] == 1 && wifiConfigured[1] == 1 && wifiConfigured[2] == 1 && wifiConfigured[3] == 1 && wifiConfigured[4] == 1){
                ESP_LOGI(TAG,"setupWifi BLE has received WIFI Data");
                parts.bulbmode = 0;
                xEventGroupSetBits(eventGroup,EVT_BLE_RECEIVED_WIFI_DATA);
                vTaskDelay(1000/portTICK_PERIOD_MS);;
            }
        }
        else{
            ESP_LOGI(TAG, "turnOnBuzzer : Nothing Set mode set");
			vTaskDelay(100/portTICK_PERIOD_MS);
        }
        taskYIELD();
    }
    vTaskDelete(setupWifiHandle);
}

void startWifi(void *pvParameters){
    // Components
    EventBits_t bits;
    
    int8_t wifiConfigured[5] = {0};

    while(true){
        ESP_LOGI(TAG,"startWifi Function");
            
        // Wait for Bits
        ESP_LOGI(TAG, "startWifi : Wait for BLE to receive data to be set");
        bits = xEventGroupWaitBits(eventGroup,EVT_BLE_RECEIVED_WIFI_DATA,pdTRUE,pdFALSE,1000/portTICK_PERIOD_MS); // Check Clock Time
        if((bits & EVT_BLE_RECEIVED_WIFI_DATA) == EVT_BLE_RECEIVED_WIFI_DATA){
            ESP_LOGI(TAG, "startWifi :  BLE to receive data is set");

            // Start Wifi
            wifi_init();
            wifi_configuration(parts.ssid,parts.pass);
            wifi_start();
            vTaskDelay(1000/portTICK_PERIOD_MS); //gives enough time for the wifi to connect to the network
            xEventGroupClearBits(eventGroup,EVT_BLE_RECEIVED_WIFI_DATA);
            break;
        }
        else{
            ESP_LOGI(TAG, "startWifi :  BLE to receive data is not set");
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }

    vTaskDelete(startWifiHandle);
}

void controlBulb(void *pvParameters){
    // Components
    EventBits_t bits;
    
    
    while(true){
        ESP_LOGI(TAG,"controlBulb Function");

        // Wait for Bits
        ESP_LOGI(TAG, "controlBulb : Wait for Buzzer to Turn On");
        bits = xEventGroupWaitBits(eventGroup,EVT_BUZZER_ON,pdTRUE,pdFALSE,1000/portTICK_PERIOD_MS); // Check Clock Time
        if(bits == EVT_BUZZER_ON){
            ESP_LOGI(TAG, "controlBulb : Buzzer On is set");
         
            switch (*(parts.bulbmode)){
                case BULB_CONFIG:
                    bconfig(parts.sku,parts.device,parts.key);
                    vTaskDelay(100/portTICK_PERIOD_MS);;
                    break;
                case BULB_ON:
                    ESP_LOGI(TAG,"Turn light on");
                    sendRequest(1);
                    vTaskDelay(100/portTICK_PERIOD_MS);;
                    break;
                case BULB_OFF:
                    ESP_LOGI(TAG,"Turn light off");
                    sendRequest(0);
                    vTaskDelay(100/portTICK_PERIOD_MS);;
                    break;
                default:
                    vTaskDelay(100/portTICK_PERIOD_MS);;
                    break;
            }
        }
        else{
            ESP_LOGI(TAG, "controlBulb : Buzzer On not set");
        }
        taskYIELD();
    }
    vTaskDelete(controlBulbHandle);
}