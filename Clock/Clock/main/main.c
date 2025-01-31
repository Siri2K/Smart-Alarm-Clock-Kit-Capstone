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

/* Function Prototypes */
// Control Task
void ControlTask(void *pvParameters);

// Computation Tasks
void createAllHWTask(void *pvParameters);
void checkControlMode(void *pvParameters);
void readTime(void *pvParameters);
void changeCurrentTime(void *pvParameters);
void changeAlarmTime(void *pvParameters);

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
    eTaskState taskState;

    /* Create All Tasks */
    // Initalize All HW
    xTaskCreatePinnedToCore(createAllHWTask,"HardWare Initialization", sizeof(parts_t),parts, configMAX_PRIORITIES -1, &createHWHandle,1);
    
    // Create All Functionality tasks
    xTaskCreatePinnedToCore(checkControlMode,"Check Clock Mode", sizeof(parts_t),parts, tskIDLE_PRIORITY + 3,&checkControlModeHandle,1);
    xTaskCreatePinnedToCore(readTime,"Read Current Time", sizeof(parts_t),parts, tskIDLE_PRIORITY,&readTimeHandle,1);
    xTaskCreatePinnedToCore(changeCurrentTime,"Change Curent Time", sizeof(parts_t),parts, tskIDLE_PRIORITY + 1,&changeCurrentTimeHandle,1);
    xTaskCreatePinnedToCore(changeAlarmTime,"Change Alarm Time", sizeof(parts_t),parts, tskIDLE_PRIORITY + 1,&changeAlarmTimeHandle,1);

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

void createAllHWTask(void *pvParameters){
    // Convert to Parts
    parts_t *part = (parts_t*)pvParameters; 

    // Initialize All Components
    // initializeLCD(parts->lcd);
    // initializeRTC(parts->rtc);
    initializeSlideSwitch(parts->slide_switch);
    initializeButtons(parts->button);
    // initializeBuzzer(parts->buzzer);

    // Set Event
    synchronizeSet(HW_INITIALIZE);
}

void checkControlMode(void *pvParameters){
    while(true){
        synchronizeWait(HW_INITIALIZE); // Do Not Start Until Hardware Is Intialized

        // Extract variables
        parts_t *part = (parts_t*)pvParameters;
        slide_switch_t *slideSwitch = parts->slide_switch;
        
        // Choose Time Tasks
        switch (*(slideSwitch->mode(slideSwitch->onClockMode,slideSwitch->onTimeMode, slideSwitch->onAlarmMode)))
        {
        case CLOCK:
            // xEventGroupClearBits(eventGroup,TIME_MODE_BIT|ALARM_MODE_BIT); // Clear Alarm and Time Bits
            // synchronizeSet(CLOCK_MODE_BIT); // Read Clock Time Event
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

void readTime(void *pvParameters){
    while(true){
        synchronizeWait(HW_INITIALIZE| CLOCK_MODE_BIT); // Do Not Read until Read Time Clock Time event is active
        /* Read RTC values */
    }
}

void changeCurrentTime(void *pvParameters){
    parts_t *part = (parts_t*)pvParameters;
    button_t *button = part->button;
    uint8_t timeIndex = 0;
    
    // Change CurrentTime
    while(true){
        synchronizeWait(HW_INITIALIZE| TIME_MODE_BIT); // Update Clock Time
        for(int i=0;i<3;i++){
            if(button->gpioPin == SELECT_BUTTON && !button->pressed(button->gpioPin)){
                while(!button->pressed(button->gpioPin)); // Lock Task until Button is released
                timeIndex++;
            }
            else if(button->gpioPin == UP_BUTTON && !button->pressed(button->gpioPin)){
                parts->currentTime[timeIndex]++;
            }
            else if(button->gpioPin == DOWN_BUTTON && !button->pressed(button->gpioPin)){
                parts->currentTime[timeIndex]--;
            }

            if(timeIndex >= 3){
                break;
            }
        }
        if(timeIndex >= 3){
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Update every 20s
    }
    
}

void changeAlarmTime(void *pvParameters){
    parts_t *part = (parts_t*)pvParameters;
    button_t *button = part->button;
    uint8_t timeIndex = 0;
    uint8_t *currentTime = (uint8_t*)malloc(3*sizeof(uint8_t));
    
    // Change Time
    while(true){
        synchronizeWait(HW_INITIALIZE| ALARM_MODE_BIT); // Update Clock Time
        for(int i=0;i<3;i++){
            if(button->gpioPin == SELECT_BUTTON && !button->pressed(button->gpioPin)){
                while(!button->pressed(button->gpioPin)); // Lock Task until Button is released
                timeIndex++;
            }
            else if(button->gpioPin == UP_BUTTON && !button->pressed(button->gpioPin)){
                currentTime[timeIndex]++;
            }
            else if(button->gpioPin == DOWN_BUTTON && !button->pressed(button->gpioPin)){
                currentTime[timeIndex]--;
            }

            if(timeIndex >= 3){
                break;
            }
        }
        if(timeIndex >= 3){
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Update every 20s
    }
}
