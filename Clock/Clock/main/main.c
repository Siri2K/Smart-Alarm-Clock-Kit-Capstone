/* Components Headers */
#include "Button.h"
#include "Buzzer.h"
#include "LCD.h"
#include "RealTimeClock.h"
#include "SlideSwitch.h"

/* FreeRTOS Header */
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* Defnitions */
// initialization
#define HW_INITIALIZE (EventBits_t)(1<<0) // HW Initialized

// Clock Modes
#define CLOCK_MODE_BIT (EventBits_t)(1<<1) // Slide_Switch on Clock Mode
#define TIME_MODE_BIT (EventBits_t)(1<<2) // Slide_Switch on Time Mode
#define ALARM_MODE_BIT (EventBits_t)(1<<3) // Slide_Switch on Alarm Mode

// Sensors
#define READ_TIME_BIT (EventBits_t)(1<<4) // Slide_Switch on Alarm Mode
#define BUZZER_ON (EventBits_t)(1<<5) // Buzzer Activates
#define BUZZER_SNOOZED (EventBits_t)(1<<6) // Buzzer Activates

/* Global Structures */
typedef struct parts_t{
    // Components
    button_t *button;
    buzzer_t *buzzer;
    lcd_t *lcd;
    real_time_clock_t *rtc;
    slide_switch_t *slide_switch;

    // Variables
    uint8_t *currentTime, *alarmTime;
}parts_t;


/* Global Objects/Variables */
// Parts
parts_t *parts = NULL;

// FreeRTOS
EventGroupHandle_t eventGroup; // Event Group Handle
TaskHandle_t createHWHandle; // HW Handle
TaskHandle_t checkControlModeHandle; // SlideSwitch Handle
TaskHandle_t readTimeHandle, changeCurrentTimeHandle, changeAlarmTimeHandle; // SlideSwitch Handle
TaskHandle_t readClockTimeHandle; // RTC Handle

/* Function Prototypes */
// Control Task
void ControlTask(void *pvParameters);

// Computation Tasks
void createAllHWTask(void *pvParameters);
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
    parts = (parts_t*)malloc(sizeof(parts_t));
    
    // Initialize Time
    parts->currentTime = (uint8_t*)malloc(3*sizeof(uint8_t));
    parts->alarmTime = (uint8_t*)malloc(3*sizeof(uint8_t));

    parts->currentTime[0] = 0; parts->alarmTime[0] = 0;
    parts->currentTime[1] = 0; parts->alarmTime[1] = 0;
    parts->currentTime[2] = 0; parts->alarmTime[2] = 0;

    ControlTask((void*)parts);
}

void ControlTask(void *pvParameters){
    // Variables
    parts_t *part = (parts_t*)pvParameters;
    eventGroup = xEventGroupCreate();
    EventBits_t uxBits;

    /* Create All Tasks */
    // Initalize All HW
    xTaskCreatePinnedToCore(createAllHWTask,"HardWare Initialization", sizeof(parts_t),part, configMAX_PRIORITIES -1, &createHWHandle,1);
    
    // Create All Functionality tasks
    xTaskCreatePinnedToCore(checkControlMode,"Check Clock Mode", sizeof(parts_t),part, tskIDLE_PRIORITY + 3,&checkControlModeHandle,1);
    xTaskCreatePinnedToCore(readClockTime,"Read Current Time", sizeof(parts_t),part, tskIDLE_PRIORITY,&readTimeHandle,1);
    xTaskCreatePinnedToCore(changeCurrentTime,"Change Curent Time", sizeof(parts_t),part, tskIDLE_PRIORITY + 1,&changeCurrentTimeHandle,1);
    xTaskCreatePinnedToCore(changeAlarmTime,"Change Alarm Time", sizeof(parts_t),part, tskIDLE_PRIORITY + 1,&changeAlarmTimeHandle,1);

    // Execute based on Events
    while (true){
        uxBits = xEventGroupGetBits(eventGroup);

        // Delete If Create HW Task is done
        if(((uxBits & HW_INITIALIZE) && HW_INITIALIZE) && (createHWHandle != NULL)){
            vTaskDelete(createHWHandle); // Delete Control HW tas after first Iteration
        }
    }
    
    // Kill All Task and Delete Event Group
    vTaskDelete(createHWHandle);
    vTaskDelete(checkControlModeHandle);
    vTaskDelete(readTimeHandle);
    vTaskDelete(changeCurrentTimeHandle);
    vTaskDelete(changeAlarmTimeHandle);
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

void createAllHWTask(void *pvParameters){
    // Convert to Parts
    parts_t *part = (parts_t*)pvParameters; 

    // Initialize All Components
    initializeLCD(parts->lcd);
    initializeRTC(part->rtc);
    initializeSlideSwitch(part->slide_switch);
    initializeButtons(part->button);
    initializeBuzzer(parts->buzzer);

    // Initialize Time
    part->currentTime = (uint8_t*) malloc(4*sizeof(uint8_t));
    part->alarmTime = (uint8_t*) malloc(4*sizeof(uint8_t));

    part->currentTime[0] = 12; part->currentTime[1] = 0; part->currentTime[2] = 0;  part->currentTime[3] = 0;
    part->alarmTime[0] = 12; part->alarmTime[1] = 0; part->alarmTime[2] = 0; part->alarmTime[3] = 0;

    part->rtc->writeTime(
        part->currentTime[0], // hour = 0100 1100 -> 12AM
        part->currentTime[1], // minute = 0x00 -> 00
        part->currentTime[2], // Second = 0x00 -> 00
        (bool)part->currentTime[3] 
    ); // Initialize to 12:00:00 am

    // Set Event
    synchronizeSet(HW_INITIALIZE);
}

void checkControlMode(void *pvParameters){
    while(true){
        synchronizeWait(HW_INITIALIZE); // Do Not Start Until Hardware Is Intialized

        // Extract variables
        parts_t *part = (parts_t*)pvParameters;
        slide_switch_t *slideSwitch = part->slide_switch;
        
        // Choose Time Tasks
        switch (*(slideSwitch->mode(slideSwitch->onClockMode,slideSwitch->onTimeMode, slideSwitch->onAlarmMode)))
        {
        case CLOCK:
            xEventGroupClearBits(eventGroup,TIME_MODE_BIT|ALARM_MODE_BIT); // Clear Alarm and Time Bits
            synchronizeSet(CLOCK_MODE_BIT); // Read Clock Time Event
            break;
        case TIME:
            xEventGroupClearBits(eventGroup,CLOCK_MODE_BIT|ALARM_MODE_BIT); // Clear Alarm and Time Bits
            synchronizeSet(TIME_MODE_BIT); // Read Time Mode Set
            break;
        case ALARM:
            xEventGroupClearBits(eventGroup,CLOCK_MODE_BIT|TIME_MODE_BIT); // Clear Alarm and Time Bits
            synchronizeSet(ALARM_MODE_BIT); // Read Time Mode Set
            break;
        default:
            break;
        }
    }
}

void changeCurrentTime(void *pvParameters){
    parts_t *part = (parts_t*)pvParameters;
    button_t *button = part->button;
    lcd_t *lcd = parts->lcd;
    uint8_t timeIndex = 0;
    char time_str[6];
    
    // Change CurrentTime
    while(true){
        synchronizeWait(HW_INITIALIZE| TIME_MODE_BIT); // Update Clock Time
        if(button->gpioPin == SELECT_BUTTON && !button->pressed(button->gpioPin)){
            while(!button->pressed(button->gpioPin)); // Lock Task until Button is released
            timeIndex = (timeIndex == 4) ? 0: timeIndex+1;
            timeIndex++;
        }
        else if(button->gpioPin == UP_BUTTON && !button->pressed(button->gpioPin)){
            if(timeIndex == 3){
                part->currentTime[timeIndex] = (part->currentTime[timeIndex] == 1) ? 0 : 1; // Lock to AM or PM
            }
            else if(timeIndex == 0){
                part->currentTime[timeIndex] = (part->currentTime[timeIndex] == 12) ? 1 : part->currentTime[timeIndex] + 1; // Allow values from 1 to 12 for hours
            }
            else{
                part->currentTime[timeIndex] = (part->currentTime[timeIndex] == 59) ? 0 : part->currentTime[timeIndex] + 1; // Allow values from 0 to 59 for minutes or seconds
            }
        }
        else if(button->gpioPin == DOWN_BUTTON && !button->pressed(button->gpioPin)){
            if(timeIndex == 3){
                part->currentTime[timeIndex] = (part->currentTime[timeIndex] == 0) ? 1 : 0; // Lock to AM or PM
            }
            else if(timeIndex == 0){
                part->currentTime[timeIndex] = (part->currentTime[timeIndex] == 0) ? 12 : part->currentTime[timeIndex] - 1; // Allow values from 1 to 12 for hours
            }
            else{
                part->currentTime[timeIndex] = (part->currentTime[timeIndex] == 0) ? 59 : part->currentTime[timeIndex] - 1; // Allow values from 0 to 59 for minutes or seconds
            }
            parts->currentTime[timeIndex]--;
        }

        // Display Time to LCD
        snprintf(time_str, sizeof(time_str), "%02d:%02d", parts->currentTime[0], parts->currentTime[1]);
        lcd->display((void*)lcd,lcdCharacters,0,10,time_str);
        lcd->display((void*)lcd,lcdCharacters,0,20,((parts->currentTime[3] == 0)? "am" : "pm"));

        // Reset index
        if(timeIndex >=3){
            timeIndex = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Update every 20ms
    }
    
}

void changeAlarmTime(void *pvParameters){
    parts_t *part = (parts_t*)pvParameters;
    button_t *button = part->button;
    uint8_t timeIndex = 0;
    lcd_t *lcd = parts->lcd;
    uint8_t *alarmTime = (uint8_t*)malloc(3*sizeof(uint8_t));
    uint8_t timeIndex = 0;
    char time_str[6];
    
    // Change Time
     while(true){
        synchronizeWait(HW_INITIALIZE| ALARM_MODE_BIT); // Update Clock Time
        if(button->gpioPin == SELECT_BUTTON && !button->pressed(button->gpioPin)){
            while(!button->pressed(button->gpioPin)); // Lock Task until Button is released
            timeIndex = (timeIndex == 4) ? 0: timeIndex+1;
            timeIndex++;
        }
        else if(button->gpioPin == UP_BUTTON && !button->pressed(button->gpioPin)){
            if(timeIndex == 3){
                part->alarmTime[timeIndex] = (part->alarmTime[timeIndex] == 1) ? 0 : 1; // Lock to AM or PM
            }
            else if(timeIndex == 0){
                part->alarmTime[timeIndex] = (part->alarmTime[timeIndex] == 12) ? 1 : part->alarmTime[timeIndex] + 1; // Allow values from 1 to 12 for hours
            }
            else{
                part->alarmTime[timeIndex] = (part->alarmTime[timeIndex] == 59) ? 0 : part->alarmTime[timeIndex] + 1; // Allow values from 0 to 59 for minutes or seconds
            }
        }
        else if(button->gpioPin == DOWN_BUTTON && !button->pressed(button->gpioPin)){
            if(timeIndex == 3){
                part->alarmTime[timeIndex] = (part->alarmTime[timeIndex] == 0) ? 1 : 0; // Lock to AM or PM
            }
            else if(timeIndex == 0){
                part->alarmTime[timeIndex] = (part->alarmTime[timeIndex] == 0) ? 12 : part->alarmTime[timeIndex] - 1; // Allow values from 1 to 12 for hours
            }
            else{
                part->alarmTime[timeIndex] = (part->alarmTime[timeIndex] == 0) ? 59 : part->alarmTime[timeIndex] - 1; // Allow values from 0 to 59 for minutes or seconds
            }
            alarmTime[timeIndex]--;
        }

        // Display Time to LCD
        snprintf(time_str, sizeof(time_str), "%02d:%02d", alarmTime[0], alarmTime[1]);
        lcd->display((void*)lcd,lcdCharacters,1,10,time_str);
        lcd->display((void*)lcd,lcdCharacters,1,20,((alarmTime[3] == 0)? "am" : "pm"));

        // Reset index
        if(timeIndex >=3){
            timeIndex = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Update every 20ms
    }
    
}

void readClockTime(void *pvParameters){
   // Components
   parts_t *part = (parts_t*)pvParameters;
   real_time_clock_t *rtc = part->rtc;
   lcd_t *lcd = parts->lcd;
   char time_str[6];

   // Read Clock Time
   while(true){
    synchronizeWait(HW_INITIALIZE| CLOCK_MODE_BIT); // Do Not Read until Read Time Clock Time event is active
    synchronizeClear(READ_TIME_BIT);
    rtc->readTime(
        &part->currentTime[0],
        &part->currentTime[1],
        &part->currentTime[2],
        (bool*)(&part->currentTime[3])
    );

    // Display Time to LCD
    snprintf(time_str, sizeof(time_str), "%02d:%02d", parts->currentTime[0], parts->currentTime[1]);
    lcd->display((void*)lcd,lcdCharacters,0,10,time_str);
    lcd->display((void*)lcd,lcdCharacters,0,20,((parts->currentTime[3] == 0)? "am" : "pm"));

    synchronizeSet(READ_TIME_BIT);
    
    // Setup Alarm
    bool triggerAlarm = (part->currentTime[0] == part->alarmTime[0]) & (part->currentTime[3] == part->alarmTime[3]); // Check for Hour and AM & PM
    triggerAlarm &= (part->currentTime[1] == part->alarmTime[1]) || (part->currentTime[1] <= part->alarmTime[1] + 2); // Check for Minutes
    if(triggerAlarm){
        synchronizeSet(BUZZER_ON);
    }
    else{
        synchronizeClear(BUZZER_ON);
        part->buzzer->powerOn(BUZZER_MODE,BUZZER_CHANNEL,NONE); // Kill Alarm
    }
   }
}

void turnOnBuzzer(void *pvParameters){
    // Components
    parts_t *part = (parts_t*)pvParameters;
    buzzer_t *buzzer = part->buzzer;
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
            buzzer->powerOn(BUZZER_MODE,BUZZER_CHANNEL,(buzzer_power_t)powerLevel);
            vTaskDelay(pdMS_TO_TICKS(200));
            buzzer->powerOn(BUZZER_MODE,BUZZER_CHANNEL,NONE);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

