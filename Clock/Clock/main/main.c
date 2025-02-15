/* Components Headers */
#include "Button.h"
#include "Buzzer.h"
#include "LCD.h"
#include "RealTimeClock.h"
#include "SlideSwitch.h"

/* Services Headers */
#include "BLE.h"

/* FreeRTOS Header */
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* Defnitions */
// Hardware
#define EVT_HW_INITIALIZE (EventBits_t)(1<<0) // HW Initialized

// Clock Modes
#define EVT_CLOCK_MODE (EventBits_t)(1<<1) // Slide_Switch on Clock Mode
#define EVT_TIME_MODE (EventBits_t)(1<<2) // Slide_Switch on Time Mode
#define EVT_ALARM_MODE (EventBits_t)(1<<3) // Slide_Switch on Alarm Mode

// Sensors
#define EVT_READ_TIME (EventBits_t)(1<<4) // Slide_Switch on Alarm Mode

// BLE
#define EVT_BLE_INITIALIZE (EventBits_t)(1<<5) // Initialize BLE

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
TaskHandle_t changeCurrentTimeHandle, changeAlarmTimeHandle; // SlideSwitch Handle
TaskHandle_t readClockTimeHandle; // RTC Handle
TaskHandle_t setupBLEHandle; // BLE handle

/* Function Prototypes */
// Control Task
void ControlTask(void *pvParameters);

// Computation Tasks
void createAllHWTask(void *pvParameters);
void checkControlMode(void *pvParameters);
void changeCurrentTime(void *pvParameters);
void changeAlarmTime(void *pvParameters);
void readClockTime(void *pvParameters);
void setupBLE(void *pvParameters);

// Helper Function
EventBits_t synchronizeWait(const EventBits_t uxBitsToSet);
EventBits_t synchronizeSet(const EventBits_t uxBitsToSet);
EventBits_t synchronizeClear(const EventBits_t uxBitsToSet);

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
    
    // Clear All Bits
    synchronizeClear(EVT_HW_INITIALIZE|EVT_CLOCK_MODE|EVT_TIME_MODE|EVT_ALARM_MODE|EVT_READ_TIME|EVT_BLE_INITIALIZE);

    /* Create All Tasks */
    // Initalize All HW
    xTaskCreatePinnedToCore(createAllHWTask,"HardWare Initialization", sizeof(parts_t),part, configMAX_PRIORITIES -1, &createHWHandle,1);
    
    // Create All Functionality tasks
    xTaskCreatePinnedToCore(setupBLE,"Setup BLE", sizeof(parts_t),part, tskIDLE_PRIORITY + 4,&setupBLEHandle,0);


    xTaskCreatePinnedToCore(checkControlMode,"Check Clock Mode", sizeof(parts_t),part, tskIDLE_PRIORITY + 3,&checkControlModeHandle,1);
    xTaskCreatePinnedToCore(readClockTime,"Read Current Time", sizeof(parts_t),part, tskIDLE_PRIORITY,&readClockTimeHandle,1);
    xTaskCreatePinnedToCore(changeCurrentTime,"Change Curent Time", sizeof(parts_t),part, tskIDLE_PRIORITY + 1,&changeCurrentTimeHandle,1);
    xTaskCreatePinnedToCore(changeAlarmTime,"Change Alarm Time", sizeof(parts_t),part, tskIDLE_PRIORITY + 1,&changeAlarmTimeHandle,1);

    // Execute based on Events
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

void createAllHWTask(void *pvParameters){
    // Convert to Parts
    parts_t *part = (parts_t*)pvParameters; 

    // Initialize All Components
    // initializeLCD(parts->lcd);
    initializeRTC(part->rtc);
    initializeSlideSwitch(part->slide_switch);
    initializeButtons(part->button);
    // initializeBuzzer(parts->buzzer);

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
    synchronizeSet(EVT_HW_INITIALIZE);
    vTaskDelete(createHWHandle);
}

void checkControlMode(void *pvParameters){
    while(true){
        synchronizeWait(EVT_HW_INITIALIZE); // Do Not Start Until Hardware Is Intialized

        // Extract variables
        parts_t *part = (parts_t*)pvParameters;
        slide_switch_t *slideSwitch = part->slide_switch;
        
        // Choose Time Tasks
        switch (*(slideSwitch->mode(slideSwitch->onClockMode,slideSwitch->onTimeMode, slideSwitch->onAlarmMode)))
        {
        case CLOCK:
            xEventGroupClearBits(eventGroup,EVT_TIME_MODE|EVT_ALARM_MODE); // Clear Alarm and Time Bits
            synchronizeSet(EVT_CLOCK_MODE); // Read Clock Time Event
            break;
        case TIME:
            xEventGroupClearBits(eventGroup,EVT_CLOCK_MODE|EVT_ALARM_MODE); // Clear Alarm and Time Bits
            synchronizeSet(EVT_TIME_MODE); // Read Time Mode Set
            break;
        case ALARM:
            xEventGroupClearBits(eventGroup,EVT_CLOCK_MODE|EVT_TIME_MODE); // Clear Alarm and Time Bits
            synchronizeSet(EVT_ALARM_MODE); // Read Time Mode Set
            break;
        default:
            break;
        }
    }
    vTaskDelete(checkControlModeHandle);
}

void changeCurrentTime(void *pvParameters){
    parts_t *part = (parts_t*)pvParameters;
    button_t *button = part->button;
    uint8_t timeIndex = 0;
    
    // Change CurrentTime
    while(true){
        synchronizeWait(EVT_HW_INITIALIZE| EVT_TIME_MODE); // Update Clock Time
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

        // Reset index
        if(timeIndex >=3){
            timeIndex = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Update every 20ms
    }
    vTaskDelete(changeCurrentTimeHandle);
}

void changeAlarmTime(void *pvParameters){
    parts_t *part = (parts_t*)pvParameters;
    button_t *button = part->button;
    uint8_t timeIndex = 0;
    uint8_t *alarmTime = (uint8_t*)malloc(3*sizeof(uint8_t));
    
    // Change Time
    while(true){
        synchronizeWait(EVT_HW_INITIALIZE| EVT_ALARM_MODE); // Update Clock Time
        if(button->gpioPin == SELECT_BUTTON && !button->pressed(button->gpioPin)){
            while(!button->pressed(button->gpioPin)); // Lock Task until Button is released
            timeIndex++;
        }
        else if(button->gpioPin == UP_BUTTON && !button->pressed(button->gpioPin)){
            alarmTime[timeIndex]++;
        }
        else if(button->gpioPin == DOWN_BUTTON && !button->pressed(button->gpioPin)){
            alarmTime[timeIndex]--;
        }

        // Reset index
        if(timeIndex >=3){
            timeIndex = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Update every 20ms
    }
    vTaskDelete(changeAlarmTimeHandle);
}

void readClockTime(void *pvParameters){
   // Components
   parts_t *part = (parts_t*)pvParameters;
   real_time_clock_t *rtc = part->rtc;

   // Read Clock Time
   while(true){
    synchronizeWait(EVT_HW_INITIALIZE| EVT_CLOCK_MODE); // Do Not Read until Read Time Clock Time event is active
    synchronizeClear(EVT_READ_TIME);
    rtc->readTime(
        &part->currentTime[0],
        &part->currentTime[1],
        &part->currentTime[2],
        (bool*)(&part->currentTime[3])
    );
    synchronizeSet(EVT_READ_TIME);
   }
   vTaskDelete(readClockTimeHandle);
}

void setupBLE(void *pvParameters){
    int i=0;
    while(++i<=10){
        initializeBLE();
        synchronizeSet(EVT_BLE_INITIALIZE);
    }
    vTaskDelete(setupBLEHandle);
}
