/* Components Headers */
#include "Button.h"
#include "Buzzer.h"
#include "LCD.h"
#include "RealTimeClock.h"
#include "SlideSwitch.h"

/* FreeRTOS Header */
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* Debug Logs */
#include "esp_log.h"

/* Defnitions */
// initialization
#define HW_INITIALIZE   (EventBits_t)(1<<0) // HW Initialized

// Clock Modes
#define CLOCK_MODE_BIT  (EventBits_t)(1<<1) // Slide_Switch on Clock Mode
#define TIME_MODE_BIT   (EventBits_t)(1<<2) // Slide_Switch on Time Mode
#define ALARM_MODE_BIT  (EventBits_t)(1<<3) // Slide_Switch on Alarm Mode

// Sensors
#define READ_TIME_BIT   (EventBits_t)(1<<4) // Slide_Switch on Alarm Mode
#define BUZZER_ON       (EventBits_t)(1<<5) // Buzzer Activates
#define BUZZER_SNOOZED  (EventBits_t)(1<<6) // Buzzer Activates

/* Global Structures */
typedef struct parts_t{
    // Components
    button_t button[3];
    buzzer_t buzzer;
    lcd_t lcd;
    real_time_clock_t rtc;
    slide_switch_t slide_switch;

    // Variables
    uint8_t currentTime[4], alarmTime[4];
}parts_t;


/* Global Objects/Variables */
// Parts
parts_t parts;

// Variables
// Constant
const uint8_t lcdCharacters[] = {
    0x7E, 0x81, 0x81, 0x81, 0x81, 0x7E, 0x00, 0x00,  // Code for char 0
    0x00, 0x42, 0x41, 0x7F, 0x40, 0x40, 0x00, 0x00,  // Code for char 1
    0xC6, 0xE3, 0xB1, 0x99, 0x8F, 0x86, 0x00, 0x00,  // Code for char 2
    0x00, 0xD3, 0x91, 0x91, 0x91, 0xFF, 0x00, 0x00,  // Code for char 3
    0x00, 0x00, 0x18, 0x14, 0x12, 0xFF, 0x10, 0x00,  // Code for char 4
    0x00, 0x4F, 0x89, 0x89, 0x89, 0xD9, 0x71, 0x00,  // Code for char 5
    0x7E, 0x91, 0x89, 0x89, 0x89, 0x7A, 0x00, 0x00,  // Code for char 6
    0x00, 0x01, 0x01, 0x01, 0x01, 0xFF, 0x00, 0x00,  // Code for char 7
    0x7E, 0x91, 0x91, 0x91, 0x91, 0x7E, 0x00, 0x00,  // Code for char 8
    0x4E, 0x89, 0x91, 0x91, 0x89, 0x7E, 0x00, 0x00,  // Code for char 9
    0x00, 0x00, 0x00, 0x33, 0x33, 0x00, 0x00, 0x00,  // Code for char :
    0x7E, 0x42, 0x42, 0x42, 0xFE, 0x40, 0x00, 0x00,  // Code for char a
    0xFC, 0x90, 0x90, 0xF0, 0x00, 0x00, 0x00, 0x00,  // Code for char b
    0xF0, 0x10, 0x08, 0xF8, 0x08, 0x10, 0xF0, 0x00,  // Code for char m
    0xFC, 0x24, 0x24, 0x3C, 0x00, 0x00, 0x00, 0x00  // Code for char p
};

// FreeRTOS
EventGroupHandle_t eventGroup; // Event Group Handle
TaskHandle_t initializeAllHWHandle; // HW Handle
TaskHandle_t checkControlModeHandle; // SlideSwitch Handle
TaskHandle_t readTimeHandle, changeCurrentTimeHandle, changeAlarmTimeHandle; // SlideSwitch Handle
TaskHandle_t readClockTimeHandle; // RTC Handle

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

// Helper Function
EventBits_t synchronizeWait(const EventBits_t uxBitsToSet);
EventBits_t synchronizeSet(const EventBits_t uxBitsToSet);

/* Main Functions */
void app_main(void)
{
    // Initialize Time
    ESP_LOGI(TAG,"Initialize Time");
    for(int i=0; i<3; i++){
        parts.currentTime[i] = (i==0)?12:0;
        parts.alarmTime[i] = (i==0)?12:0;
    }

    ControlTask((void*)(&parts));
}

void ControlTask(void *pvParameters){
    // Log
    ESP_LOGI(TAG,"Create Seperate Tasks");
    
    // Variables
    eventGroup = xEventGroupCreate();
    parts_t *part = (parts_t*)pvParameters;

    /* Create All Tasks */
    // Initalize All HW
    xTaskCreatePinnedToCore(initializeAllHWTask,"HardWare Initialization", sizeof(parts_t),part, configMAX_PRIORITIES -1, &initializeAllHWHandle,1);
    
    // Create All Functionality tasks
    xTaskCreatePinnedToCore(checkControlMode,"Check Clock Mode", sizeof(parts_t),part, tskIDLE_PRIORITY + 3,&checkControlModeHandle,1);
    xTaskCreatePinnedToCore(readClockTime,"Read Current Time", sizeof(parts_t),part, tskIDLE_PRIORITY,&readTimeHandle,1);
    xTaskCreatePinnedToCore(changeCurrentTime,"Change Curent Time", sizeof(parts_t),part, tskIDLE_PRIORITY + 1,&changeCurrentTimeHandle,1);
    xTaskCreatePinnedToCore(changeAlarmTime,"Change Alarm Time", sizeof(parts_t),part, tskIDLE_PRIORITY + 1,&changeAlarmTimeHandle,1);

    // Start Scheduler
    vTaskStartScheduler();
}

EventBits_t synchronizeWait(const EventBits_t uxBitsToSet){
    return xEventGroupSetBits(eventGroup, uxBitsToSet); // Set Bit
}

EventBits_t synchronizeSet(const EventBits_t uxBitsToSet){
    return xEventGroupWaitBits( // Receive Wait Bit
        eventGroup, 
        uxBitsToSet,
        pdFALSE, 
        pdTRUE, 
        portMAX_DELAY
    );
}

EventBits_t synchronizeClear(const EventBits_t uxBitsToSet){
    return xEventGroupClearBits(eventGroup,uxBitsToSet);
}

void initializeAllHWTask(void *pvParameters){
    // Log
    ESP_LOGI(TAG,"Initialize All HW Components Task");
    
    // Convert to Parts
    parts_t *part = (parts_t*)pvParameters; 

    // Initialize Buttons
    initializeButton(&(part->button[0]), UP_BUTTON);
    initializeButton(&(part->button[1]), DOWN_BUTTON);
    initializeButton(&(part->button[2]), SELECT_BUTTON);

    // Initialize Other Components
    initializeSlideSwitch(&part->slide_switch);
    initializeBuzzer(&part->buzzer);
    initializeRTC(&part->rtc);
    initializeLCD(&part->lcd);

    // Intialize Current and Alarm Time
    ESP_LOGI(TAG,"Set Default Time to RTC");

    part->rtc.writeTime(
        part->currentTime[0], // hour = 0100 1100 -> 12AM
        part->currentTime[1], // minute = 0x00 -> 00
        part->currentTime[2], // Second = 0x00 -> 00
        part->currentTime[3] 
    );
    
    // Set Event
    synchronizeSet(HW_INITIALIZE);
}

void checkControlMode(void *pvParameters){
    // Log
    ESP_LOGI(TAG,"Check Control Mode Task");

    synchronizeClear(TIME_MODE_BIT|ALARM_MODE_BIT|CLOCK_MODE_BIT);
    while(true){
        synchronizeWait(HW_INITIALIZE); // Do Not Start Until Hardware Is Intialized
        
        // Extract variables
        parts_t *part = (parts_t*)pvParameters;
        slide_switch_t slideSwitch = part->slide_switch;
        clock_mode_t *clockMode = slideSwitch.mode();
        
        // Choose Time Tasks
        switch (*clockMode){
        case CLOCK:
            ESP_LOGI(TAG,"Set to Clock Mode");
            synchronizeClear(TIME_MODE_BIT|ALARM_MODE_BIT);
            synchronizeSet(CLOCK_MODE_BIT);
            break;
        case TIME:
            ESP_LOGI(TAG,"Set to Time mode");
            synchronizeClear(CLOCK_MODE_BIT|ALARM_MODE_BIT);
            synchronizeSet(TIME_MODE_BIT);
            break;
        case ALARM:
            ESP_LOGI(TAG,"Set to Alarm mode");
            synchronizeClear(TIME_MODE_BIT|CLOCK_MODE_BIT);
            synchronizeSet(ALARM_MODE_BIT);
            break;
        default:
            break;
        }
    }
}

void changeCurrentTime(void *pvParameters){
    // Log
    ESP_LOGI(TAG,"Change Current Time Task");
    
    parts_t *part = (parts_t*)pvParameters;

    // Isolate Needed HW
    button_t button[] = {part->button[0],part->button[1],part->button[2]};
    lcd_t lcd = part->lcd;
    uint8_t timeIndex = 0;
    char time_str[10];
    
    // Update Current Time
    while (true){
        synchronizeWait(HW_INITIALIZE| TIME_MODE_BIT); // Update Clock Time

        // Act based on button pressed
        if(timeIndex < 3){
            uint8_t currentTime = part->currentTime[timeIndex];
            if(button[0].pressed){ // Up Button
                ESP_LOGI(TAG,"Pressed Up Button");
                if(currentTime >= 13 && timeIndex == 0){
                    part->currentTime[timeIndex] = 1;
                    part->currentTime[3] = (part->currentTime[3] == 1)? 0:1; // Toggle AM to PM and vice-versa
                }
                else if(currentTime > 59 && timeIndex != 0){
                    part->currentTime[timeIndex] = 0;
                }
                else{
                    part->currentTime[timeIndex]++;
                }
            }
            else if(button[1].pressed){ // Down Button
                ESP_LOGI(TAG,"Pressed Down Button");
                if(currentTime <= 0 && timeIndex == 0){
                    part->currentTime[timeIndex] = 12;
                    part->currentTime[3] = (part->currentTime[3] == 1)? 0:1; // Toggle AM to PM and vice-versa
                }
                else if(currentTime <= 0 && timeIndex != 0){
                    part->currentTime[timeIndex] = 59;
                }
                else{
                    part->currentTime[timeIndex]--;
                }
            }
            else if(button[2].pressed){ // Select Button
                ESP_LOGI(TAG,"Pressed Select Button");
                timeIndex++;
            }
        }
        else{
            timeIndex = 0;
        }

        // Display Time to LCD
        ESP_LOGI(TAG,"Display Time on LCD");
        snprintf(time_str, sizeof(time_str), "%02d:%02d", part->currentTime[0], part->currentTime[1]);
        lcd.display((void*)(&lcd),lcdCharacters,0,10,time_str);
        lcd.display((void*)(&lcd),lcdCharacters,0,20,((part->currentTime[3] == 0)? "am" : "pm"));
    }
    
    
}

void changeAlarmTime(void *pvParameters){
    // Log
    ESP_LOGI(TAG,"Change Current Alarm Task");
    
    parts_t *part = (parts_t*)pvParameters;

    // Isolate Needed HW
    button_t button[] = {part->button[0],part->button[1],part->button[2]};
    lcd_t lcd = part->lcd;
    uint8_t timeIndex = 0;
    char time_str[10];
    
    // Change Time
     while(true){
        synchronizeWait(HW_INITIALIZE| ALARM_MODE_BIT); // Update Clock Time

        // Act based on button pressed
        if(timeIndex < 3){
            uint8_t alarmTime = part->alarmTime[timeIndex];
            if(button[0].pressed){ // Up Button
                ESP_LOGI(TAG,"Pressed Up Button");
                if(alarmTime >= 13 && timeIndex == 0){
                    part->alarmTime[timeIndex] = 1;
                    part->alarmTime[3] = (part->alarmTime[3] == 1)? 0:1; // Toggle AM to PM and vice-versa
                }
                else if(alarmTime > 59 && timeIndex != 0){
                    part->alarmTime[timeIndex] = 0;
                }
                else{
                    part->alarmTime[timeIndex]++;
                }
            }
            else if(button[1].pressed){ // Down Button
                ESP_LOGI(TAG,"Pressed Down Button");
                if(alarmTime <= 0  && timeIndex == 0){
                    part->alarmTime[timeIndex] = 12;
                    part->alarmTime[3] = (part->alarmTime[3] == 1)? 0:1; // Toggle AM to PM and vice-versa
                }
                else if(alarmTime <= 0 && timeIndex != 0){
                    part->alarmTime[timeIndex] = 59;
                }
                else{
                    part->alarmTime[timeIndex]--;
                }
            }
            else if(button[2].pressed){ // Select Button
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
        vTaskDelay(pdMS_TO_TICKS(20)); // Update every 20ms
    }
    
}

void readClockTime(void *pvParameters){
   // Log
   ESP_LOGI(TAG,"Read Clock Time");
   
    // Components
   parts_t *part = (parts_t*)pvParameters;

   // Isolated Needed HW
   real_time_clock_t rtc = part->rtc;
   lcd_t lcd = part->lcd;
   buzzer_t buzzer = part->buzzer;
   char time_str[10];

   // Read Clock Time
   while(true){
    synchronizeWait(HW_INITIALIZE| CLOCK_MODE_BIT); // Do Not Read until Read Time Clock Time event is active
    synchronizeClear(READ_TIME_BIT);

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
    synchronizeSet(READ_TIME_BIT);

    // Setup Alarm
    uint8_t triggerAlarm = (part->currentTime[0] == part->alarmTime[0]) & (part->currentTime[3] == part->alarmTime[3]); // Check for Hour and AM & PM
    triggerAlarm &= (part->currentTime[1] == part->alarmTime[1]) || (part->currentTime[1] <= part->alarmTime[1] + 2); // Check for Minutes
    if(triggerAlarm){
        ESP_LOGI(TAG,"Turn ON Buzzer");
        synchronizeSet(BUZZER_ON);
    }
    else{
        ESP_LOGI(TAG,"Turn ON Buzzer");
        buzzer.powerOn(BUZZER_MODE,BUZZER_CHANNEL,NONE); // Kill Alarm
        synchronizeClear(BUZZER_ON);
    }
   }
}

void turnOnBuzzer(void *pvParameters){
    // Components
    parts_t *part = (parts_t*)pvParameters;
    buzzer_t buzzer = part->buzzer;
    int64_t buzzerStartTime = 0, duration = 0;
    int powerLevel = LOW;

    // Turn On Buzzer
    while(true){
        synchronizeWait(HW_INITIALIZE| CLOCK_MODE_BIT | BUZZER_ON); // Do Not Read until Read Time Clock Time event is active
    
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

