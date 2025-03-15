/* Headers */
#include "Component/Battery/battery.h"
#include "Component/Button/button.h"
#include "Component/RGB/rgb.h"
#include "Component/BLE/ble.h"
#include "Component/Accelerometer/accelerometer.h"
#include "Component/ECG/ecg.h"


/* Definitions */
// Event Bits
#define EVT_PARTS_INITIALIZED   BIT(0)
#define EVT_BUTTON_SHORT        BIT(1)
#define EVT_BUTTON_LONG         BIT(2)
#define EVT_ECG_DATA            BIT(3)
#define EVT_ACCEL_DATA          BIT(4)
#define EVT_BLE_CONNECTED       BIT(5)


// Stack Size
#define STACK_SIZE 192 // XIAO worked with 512 bytes
#define TASK_COUNT 7


/* Global Variables */
// Threads Stacks
K_THREAD_STACK_DEFINE(initialize_all_parts_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(button_control_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(display_battery_levels_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(read_ecg_data_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(read_accelerometer_data_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(connect_to_clock_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(send_data_to_clock_stack, STACK_SIZE);

// Event group for synchronization
static struct k_event task_events;

// Threads
static struct thread_context {
    struct k_thread data;
    k_tid_t id;
} threads[TASK_COUNT];


/* Prototypes */
// Control Task
void controlTask();

// Computation Tasks
void initializeAllParts(void *p1, void *p2, void *p3);      // Initialize All HW Parts
void buttonControl(void *p1, void *p2, void *p3);           // Choose Press REsult : Short = Battery Check, Long = Connect to BLE
void displayBatteryLevels(void *p1, void *p2, void *p3);    // Display Battery Levels to User
void readECGData(void *p1, void *p2, void *p3);             // Get BPM from ECG device
void readAccelerometerData(void *p1, void *p2, void *p3);   // Get VX, VY, VZ from Accelerometer
void connectToClock(void *p1, void *p2, void *p3);          // Establish BLE Connection to Clock
void sendDataToClock(void *p1, void *p2, void *p3);         // Send Data to Clock


int main(){
    controlTask();
    return 0;
}

void controlTask(){
    // Initialize tasks Events
    k_event_init(&task_events);

    // Clear All Events
    k_event_clear(
        &task_events, 
        EVT_PARTS_INITIALIZED
    );
    k_event_clear(
        &task_events, 
        EVT_BUTTON_SHORT|EVT_BUTTON_LONG
    );
    k_event_clear(
        &task_events, 
        EVT_ECG_DATA| EVT_ACCEL_DATA
    );
    k_event_clear(
        &task_events, 
        EVT_BLE_CONNECTED
    );

    // Create All Threads
    static const struct {
        k_thread_entry_t entry;
        k_thread_stack_t *stack;
        size_t stack_size;
    } thread_configs[] = {
        {initializeAllParts, initialize_all_parts_stack, STACK_SIZE},
        {buttonControl, button_control_stack, STACK_SIZE},
        {displayBatteryLevels, display_battery_levels_stack, STACK_SIZE},
        {readECGData, read_ecg_data_stack, STACK_SIZE},
        {readAccelerometerData, read_accelerometer_data_stack, STACK_SIZE},
        {connectToClock, connect_to_clock_stack, STACK_SIZE},
        {sendDataToClock, send_data_to_clock_stack, STACK_SIZE}
    };

    
    for(int i=0; i<sizeof(thread_configs)/sizeof(thread_configs[0]);i++){
        threads[i].id = k_thread_create(
            &threads[i+1].data,
            thread_configs[i].stack,
            thread_configs[i].stack_size,
            thread_configs[i].entry,
            NULL,
            NULL,
            NULL,
            0,
            0,
            K_NO_WAIT
        );
    }
}

void initializeAllParts(void *p1, void *p2, void *p3){
    // Status
    int8_t status = 0; 

    status = initializeRGB();
    if (status != 0) {
        return;
    }

    status = initializeButton();
    if (status != 0) {
        return;
    }

    status = initializeBattery();
    if (status != 0) {
        return;
    }

    status = (int8_t)initializeECG();
    if (status != 0) {
        return;
    }

    status = (int8_t)initializeAccelerometer();
    if (status != 0) {
        return;
    }

    status = initializeBLE();
    if (status != 0) {
        return;
    }

    k_event_set(&task_events, EVT_PARTS_INITIALIZED);
    k_msleep(10); 
    return;
}

void buttonControl(void *p1, void *p2, void *p3){ // Choose Between Display Battery levels or Connect to BLE
    // Wait for HW to be initialized
    k_event_wait(&task_events, EVT_PARTS_INITIALIZED, false, K_FOREVER);

    while(true){
        // Calculate Press Time
        int64_t duration = calculatePressTime();
        if(duration > 50 && duration < BUTTON_PRESSED_MAX){ // Less than 10s
            k_event_set(&task_events, EVT_BUTTON_SHORT);
            k_yield();
        }
        else if(duration > 50 && duration >= BUTTON_PRESSED_MAX){
            k_event_set(&task_events, EVT_BUTTON_LONG);
            k_yield();
        }
    }
}

void displayBatteryLevels(void *p1, void *p2, void *p3){
    while(true){
        // Wait for HW to be initialized
        k_event_wait(&task_events, EVT_BUTTON_SHORT, true, K_FOREVER); // Wait for a Short Press

        // Status
        int16_t batteryPercentage = readBatteryChargePercentage();
        k_msleep(50);

        // Read Battery Percentages
        if(batteryPercentage < BATTERY_MIN || batteryPercentage < BATTERY_LOW){
            turnOnRGB(RED);
        }
        else if(batteryPercentage < BATTERY_MID){
            turnOnRGB(YELLOW);
        }
        else{
            turnOnRGB(GREEN);
        }
        
        // Turn off after 2s
        k_sleep(K_SECONDS(2));
        turnOffRGB();
        k_yield();
    }
}

void connectToClock(void *p1, void *p2, void *p3){
    while(true){
        // Wait for HW to be initialized
        k_event_wait(&task_events, EVT_BUTTON_LONG, true, K_FOREVER); // Wait for a Long Press

        // Variables
        int8_t status = 0;

        /* Check if Already Connected */
        if(connectedBLE == 1){
            k_yield();
            continue;
        }
        else{
            k_event_clear(&task_events, EVT_BLE_CONNECTED);
        }

        //Connect to Clock
        status = startScanningBLE();
        if(status == 0){
            k_event_set(&task_events, EVT_BLE_CONNECTED);
            k_yield();
        } 
    }
}

void sendDataToClock(void *p1, void *p2, void *p3){
    k_event_wait(&task_events, EVT_BLE_CONNECTED, false, K_FOREVER); // Wait for a Long Press
    while(true){
        uint8_t data[4] = {
            collectedData.bpm,
            collectedData.vx,
            collectedData.vy,
            collectedData.vz,
        };
        
        if(connectedBLE == 1){
            sendBLEData(current_conn, data);
        }
        k_sleep(K_SECONDS(2));  // Wait for 2 seconds
    }
    
}

void readECGData(void *p1, void *p2, void *p3){
    // Wait for HW to be initialized
    k_event_wait(&task_events, EVT_PARTS_INITIALIZED | EVT_BLE_CONNECTED, false, K_FOREVER);
    
    while(true){
        collectedData.bpm = getBPM(); // Read ECG Data
        k_event_set(&task_events, EVT_ECG_DATA);
        k_msleep(10);
    }
}

void readAccelerometerData(void *p1, void *p2, void *p3){
    // Wait for HW to be initialized
    k_event_wait(&task_events, EVT_PARTS_INITIALIZED | EVT_BLE_CONNECTED | !EVT_ACCEL_DATA, false, K_FOREVER);
    
    while(true){
        readXYZ(&accelerometerData); // Read Accelerometer Data

        collectedData.vx = accelerometerData.vx;
        collectedData.vy = accelerometerData.vy;
        collectedData.vz = accelerometerData.vz;

        k_event_set(&task_events, EVT_ACCEL_DATA);
        k_msleep(10);
    }
}