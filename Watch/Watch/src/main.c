/* Headers */
#include "Component/Battery/battery.h"
#include "Component/Button/button.h"
#include "Component/RGB/rgb.h"
#include "Component/Accelerometer/accelerometer.h"
#include "Component/ECG/ecg.h"
#include "Services/ble.h"

/* Definitions */
// Event Bits
#define EVT_PARTS_INITIALIZED   BIT(0)
#define EVT_BUTTON_SHORT        BIT(1)
#define EVT_BUTTON_LONG         BIT(2)
#define EVT_ECG_DATA            BIT(3)
#define EVT_ACCEL_DATA          BIT(4)
#define EVT_BLE_CONNECTED       BIT(5)
#define EVT_BLE_DATA_SENT       BIT(6)

// Stack Size
#define STACK_SIZE 512

// Number of Parts
#define PART_COUNT 5

/* Global Variables */
collected_data_t collectedData;

// Threads Stacks
K_THREAD_STACK_DEFINE(control_task_stack, STACK_SIZE);

K_THREAD_STACK_DEFINE(initialize_all_parts_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(button_control_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(display_battery_levels_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(read_ecg_data_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(read_accelerometer_data_stack, STACK_SIZE);

// Event group for synchronization
K_EVENT_DEFINE(task_events);

// Threads
static struct thread_context {
    struct k_thread data;
    k_tid_t id;
} threads[6];

// BLE Address
/*
    - Clock Address is found on the ESP32 microcontroller itself
    - The MAC Address on the ESP32 needs to be used in big-endian
*/
static const bt_addr_le_t clockAddress = {
    .type = BT_ADDR_LE_RANDOM, 
    .a.val = {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56} // MAC : 56:34:12:EF:CD:AB
};

/* Prototypes */
// Control Task
void controlTask();

// Computation Tasks
void initializeAllParts();      // Initialize All HW Parts
void buttonControl();           // Choose Press REsult : Short = Battery Check, Long = Connect to BLE
void displayBatteryLevels();    // Display Battery Levels to User
void connectToClock();          // Establish BLE COnnection to Clock
void readECGData();             // Get BPM from ECG device
void readAccelerometerData();   // Get VX, VY, VZ from Accelerometer
void sendDataToClock();         // Send Data to Clock

int main(){
    k_event_init(&task_events);
    threads[0].id = k_thread_create(
        &threads[0].data,
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
    return 0;
}

void controlTask(){
    // Create HW
    threads[1].id = k_thread_create(
        &threads[1].data,
        initialize_all_parts_stack,
        STACK_SIZE,
        (k_thread_entry_t)initializeAllParts,
        NULL,
        NULL,
        NULL,
        K_PRIO_PREEMPT(0),
        0,
        K_NO_WAIT
    );

    // Create remaining threads
   static const struct {
    k_thread_entry_t entry;
    k_thread_stack_t *stack;
    size_t stack_size;
    } thread_configs[] = {
        {buttonControl, button_control_stack, STACK_SIZE/2},
        {displayBatteryLevels, display_battery_levels_stack, STACK_SIZE/2},
        {readECGData, read_ecg_data_stack, STACK_SIZE},
        {readAccelerometerData, read_accelerometer_data_stack, STACK_SIZE/2}
    };

    for(int i=0; i<sizeof(thread_configs)/sizeof(thread_configs[0]);i++){
        threads[i].id = k_thread_create(
            &threads[i+2].data,
            thread_configs[i].stack,
            thread_configs[i].stack_size,
            thread_configs[i].entry,
            NULL,
            NULL,
            NULL,
            K_PRIO_PREEMPT(2),
            0,
            K_NO_WAIT
        );
    }
}

void initializeAllParts(){
    // Status
    uint8_t status = 0;
    uint8_t i = 0; 

    // Initialize All Parts
    k_event_clear(&task_events, EVT_PARTS_INITIALIZED|EVT_BUTTON_SHORT|EVT_BUTTON_LONG|EVT_ECG_DATA|EVT_ACCEL_DATA);
    while(++i<10){ // Try 10 Attempts 
        status = initializeButton() || initializeBattery() ||initializeRGB() || initializeECG() || initializeAccelerometer(collectedData.accelerometer);
        if(status == 0){
            k_event_post(&task_events, EVT_PARTS_INITIALIZED);
            k_thread_abort(threads[0].id);
            break;
        }
    }
}

void buttonControl(){ // Choose Between Display Battery levels or Connect to BLE
    k_event_clear(&task_events, EVT_BUTTON_SHORT | EVT_BUTTON_LONG);
    while(true){
        // Wait for HW to be initialized
        k_event_wait(&task_events, EVT_PARTS_INITIALIZED, false, K_FOREVER);
        
        // Calculate Press Time
        int64_t pressTime = calculatePressTime();
        if(pressTime > 1 && pressTime < BUTTON_PRESSED_MAX){ // Less than 10s
            k_event_post(&task_events, EVT_BUTTON_SHORT);
            k_event_clear(&task_events, EVT_BUTTON_LONG);
        }
        else if(pressTime > 1 && pressTime >= BUTTON_PRESSED_MAX){
            k_event_post(&task_events, EVT_BUTTON_LONG);
            k_event_clear(&task_events, EVT_BUTTON_SHORT);
        }
    }
}

void displayBatteryLevels(){
    k_event_clear(&task_events, EVT_BUTTON_SHORT);
    while(true){
       // Wait for HW to be initialized
       k_event_wait(&task_events, EVT_PARTS_INITIALIZED | EVT_BUTTON_SHORT, false, K_FOREVER); // Wait for a Short Press
       uint8_t status = 0;
       uint8_t batteryPercentage = readBatteryChargePercentage();

       // Read Battery Percentages
       if(batteryPercentage < BATTERY_MIN || batteryPercentage < BATTERY_LOW){
        status = turnOnRGB(RED);
       }
       else if(batteryPercentage < BATTERY_MID){
        status = turnOnRGB(YELLOW);
       }
       else{
        status = turnOnRGB(GREEN);
       }

       // Turn off after 2s
       k_sleep(K_SECONDS(2));
       turnOffRGB();
       k_event_clear(&task_events, EVT_BUTTON_SHORT);
    }
}

void connectToClock(){
    k_event_clear(&task_events, EVT_BUTTON_LONG | EVT_BLE_CONNECTED);

    // Variables
    int8_t status = 0;
    struct bt_conn *conn;

    while(true){
        // Wait for HW to be initialized
        k_event_wait(&task_events, EVT_PARTS_INITIALIZED | EVT_BUTTON_LONG, false, K_FOREVER); // Wait for a Long Press
        
        // Enable and Connect to Clock
        status = (int8_t)bt_enable(NULL);
        status = (int8_t)bt_conn_cb_register(&connCallbacks);
        status = (int8_t)bt_conn_le_create(&clockAddress, &param, BT_LE_CONN_PARAM_DEFAULT, &conn);
        
        if(conn){
            connection = conn;
        }
        
        if(status == 0){
            k_event_set(&task_events, EVT_BLE_CONNECTED);
            k_event_clear(&task_events,EVT_BUTTON_LONG);
        } 
    }
}

void readECGData(){
    k_event_clear(&task_events, EVT_ECG_DATA);
    while(true){
        // Wait for HW to be initialized
        k_event_wait(&task_events, EVT_PARTS_INITIALIZED | EVT_BLE_CONNECTED | !EVT_ECG_DATA, false, K_FOREVER);
        collectedData.bpm = getBPM(); // Read ECG Data
        k_event_set(&task_events, EVT_ECG_DATA);
    }
}

void readAccelerometerData(){
    k_event_clear(&task_events, EVT_ACCEL_DATA);
    while(true){
        // Wait for HW to be initialized
        k_event_wait(&task_events, EVT_PARTS_INITIALIZED | EVT_BLE_CONNECTED | !EVT_ACCEL_DATA, false, K_FOREVER);
        readXYZ(collectedData.accelerometer); // Read Accelerometer Data
        k_event_set(&task_events, EVT_ACCEL_DATA);
    }
}


void sendDataToClock(){
    k_event_clear(&task_events, EVT_PARTS_INITIALIZED |EVT_BLE_CONNECTED | EVT_ECG_DATA | EVT_ACCEL_DATA);
    while(true){
        // Wait for HW to be initialized
        k_event_wait(&task_events, EVT_PARTS_INITIALIZED | EVT_BLE_CONNECTED | EVT_ECG_DATA | EVT_ACCEL_DATA | !EVT_BLE_DATA_SENT, false, K_FOREVER);
        notifyData(collectedData); // Transmit Data
        k_sleep(K_SECONDS(5));
        k_event_set(&task_events, EVT_BLE_DATA_SENT);
    }
}