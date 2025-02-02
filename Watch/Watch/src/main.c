/* Headers */
#include "Component/Battery/battery.h"
#include "Component/Button/button.h"
#include "Component/RGB/rgb.h"
#include "Component/Accelerometer/accelerometer.h"
#include "Component/ECG/ecg.h"

/* Defnitions */
// Stack Size
#define STACK_SIZE 1024

// Events
#define ALL_PARTS_INITIALIZED BIT(0)
#define BUTTON_SHORT_PRESSED BIT(1)
#define BUTTON_LONG_PRESSED BIT(2)

#define ECG_DATA_READ BIT(4)

/* Global Variables */
// Data
accelerometer_data_t *accelerometer; // Accelerometer
uint8_t* bpm; // HeartRate

// Threads
K_THREAD_STACK_DEFINE(control_task_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(intialize_all_parts_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(button_control_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(display_battery_levels_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(read_ecg_stack, STACK_SIZE);

struct k_thread *control_task_data;
struct k_thread *intialize_all_parts_data;
struct k_thread *button_control_data;
struct k_thread *display_battery_levels_data;
struct k_thread *read_ecg_data;

k_tid_t control_task_id;
k_tid_t intialize_all_parts_id;
k_tid_t button_control_id;
k_tid_t display_battery_levels_id;
k_tid_t read_ecg_id;

// Event group for synchronization
K_EVENT_DEFINE(task_events);

/* Prototypes */
// Control task
void controlTask();

// Computation Tasks
void intializeAllParts();
void buttonControl();
void displayBatteryLevels();
void readECG();

// Main
int main(void)
{   
    // Initialize BPM
    bpm = (uint8_t*)malloc(sizeof(uint8_t));

    /* Create Control Task*/
    control_task_id = k_thread_create(
        control_task_data,
        control_task_stack,
        STACK_SIZE,
        (k_thread_entry_t)controlTask,
        NULL,
        NULL,
        NULL,
        K_PRIO_PREEMPT(0),
        0,
        K_NO_WAIT
    );
}

// Functions
void controlTask(){
    /* Create Computation Tasks*/
    intialize_all_parts_id = k_thread_create(
        intialize_all_parts_data,
        intialize_all_parts_stack,
        STACK_SIZE,
        (k_thread_entry_t)intializeAllParts,
        NULL,
        NULL,
        NULL,
        K_PRIO_PREEMPT(1),
        0,
        K_NO_WAIT
    );

    k_event_wait(&task_events, ALL_PARTS_INITIALIZED, false, K_FOREVER);
    
    button_control_id = k_thread_create(
        button_control_data,
        button_control_stack,
        STACK_SIZE,
        (k_thread_entry_t)buttonControl,
        NULL,
        NULL,
        NULL,
        K_PRIO_PREEMPT(2),
        0,
        K_NO_WAIT
    );
    
    display_battery_levels_id = k_thread_create(
        display_battery_levels_data,
        display_battery_levels_stack,
        STACK_SIZE,
        (k_thread_entry_t)displayBatteryLevels,
        NULL,
        NULL,
        NULL,
        K_PRIO_PREEMPT(2),
        0,
        K_NO_WAIT
    );

    read_ecg_id = k_thread_create(
        read_ecg_data,
        read_ecg_stack,
        STACK_SIZE,
        (k_thread_entry_t)readECG,
        NULL,
        NULL,
        NULL,
        K_PRIO_PREEMPT(2),
        0,
        K_NO_WAIT
    );
}

void intializeAllParts(){
    initializeButton();
    initializeBattery();
    initializeRGB();
    initializeAccelerometer(accelerometer);
    initializeECG();
    k_event_post(&task_events, ALL_PARTS_INITIALIZED);
}

void buttonControl(){
    while(true){
        k_event_clear(&task_events, BUTTON_LONG_PRESSED | BUTTON_SHORT_PRESSED);
        if(pressed()){
            if(calculatePressTime() > 10000){ // Check if button is held for 10s
            k_event_post(&task_events, BUTTON_LONG_PRESSED);
            }
            else{
                k_event_post(&task_events, BUTTON_SHORT_PRESSED);
            }
        }
    }
   
}

void displayBatteryLevels(){
    rgb_colors_t color;

    // Configure LED based on battery 
    while(true){
        switch (readbatteryCharge()){
        case BATTERY_MIN:
            color = RED;
            break;
        case BATTERY_LOW:
            color = RED;
            break;
        case BATTERY_MID:
            color = YELLOW;
            break;
        case BATTERY_HIGH:
            color = GREEN;
            break;
        case BATTERY_MAX:
            color = GREEN;
            break;
        default:
            color = GREEN;
            break;
        }
        turnOnRGB(color);
        k_sleep(K_MSEC(2000)); // 2 Second Sleep
        turnOffRGB();
    }
    
}

void readECG(){
    // Calculate BPM
    k_event_clear(&task_events, ECG_DATA_READ);
    *bpm = calculateBPM(getFIFODataSamples()); // Get BPM Data
    k_event_post(&task_events, ECG_DATA_READ);
}
