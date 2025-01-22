// Header
// crossing the casme book
// hard to isolate a mouvement {need extra reference}
#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

// Setup status led
#define LED_RED 22
#define LED_GREEN 23
#define LED_BLUE 24

// Arduino genearal variables
unsigned long startTime;

// Constants for gyro sensitivity based on expected range settings
float gyroSensitivity = 0.0175; // Adjusted to match the sensitivity for LSM9DS1 in degrees per second per LSB

// Global variables for accelerometer, gyroscope data, and calculated angles
float RateYaw, RateRoll, RatePitch;
float AccX, AccY, AccZ;
float Roll = 0, Pitch = 0, Yaw = 0;
float AngleRoll = 0, AnglePitch = 0, AngleYaw = 0;
uint32_t LoopTimer;
float gz_bias = 0, gx_bias = 0, gy_bias = 0;

// Calibration variable
int sampleCounter = 0;
int numSamples = 100;
float sumX = 0, sumY = 0, sumZ = 0;

// IMU state manager variable
bool isMeasuring = false;
bool isCalibrating = false;
bool isConnected = false;
bool isReseting = false;
bool isSerialMonitoringActive = false;
bool isImuInitialized = false;
bool isBleInitialized = false;

// Ble transfer variable
BLEService imuService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEStringCharacteristic dataCharacteristic("19B10005-E8F2-537E-4F6C-D104768A1214", BLENotify | BLEWrite, 512);

// Arduino debug variables
String bleAddress;
String bleCommand = "None";
String bleDebugMessage = "None";
String bleStatus = "";

void checkBleInput(BLEDevice &central)
{
    // Check the address
    bleAddress = String(central.address());

    // Check the commands receive
    // bleCommand = "None";
    if (dataCharacteristic.written())
    {
        bleCommand = dataCharacteristic.value();
    }
}

void sendBleOutput()
{
    int YawInt = (int)AngleYaw;
    int PitchInt = (int)AnglePitch;
    int RollInt = (int)AngleRoll;

    String data = String(YawInt) + "," + String(PitchInt) + "," + String(RollInt) + "," + bleStatus;
    // update ble

    if (bleStatus != "")
    {
        bleStatus = "";
    }
    dataCharacteristic.writeValue(data);
}

void startCalibrateImu()
{
    sampleCounter++;
    isCalibrating = true;
    bleDebugMessage += " Started callibration -";
    bleStatus = " Started callibration ";
}

void continueCalibrateImu()
{
    // Update acceleration
    float xGyro, yGyro, zGyro;
    IMU.readGyroscope(xGyro, yGyro, zGyro);
    sumX += xGyro * gyroSensitivity;
    sumY += yGyro * gyroSensitivity;
    sumZ += zGyro * gyroSensitivity;
    sampleCounter++;
    delay(2);

    // Update bias
    gx_bias = sumX / sampleCounter;
    gy_bias = sumY / sampleCounter;
    gz_bias = sumZ / sampleCounter;
}

void finishCalibrateImu()
{
    // initiate and handle the callibration unprevented calibration termination
    gx_bias = sumX / sampleCounter;
    gy_bias = sumY / sampleCounter;
    gz_bias = sumZ / sampleCounter;
    sampleCounter = 0;
    sumX = 0, sumY = 0, sumZ = 0;
    isCalibrating = false;
    Roll = 0, Pitch = 0, Yaw = 0;
    bleDebugMessage += " Finished callibration -";
    bleStatus = " Finished callibration ";

    if (!isMeasuring)
    {
        isMeasuring = true;
        bleDebugMessage += " Started measuring -";
        bleStatus = " Started measuring ";
    }
}

void resetMeasuring()
{
    // Set values to zero
    Roll = 0, Pitch = 0, Yaw = 0;
    bleDebugMessage += " Resetted Angles -";
    bleStatus = " Resetted Angles ";
}

void readIMU()
{
    float xGyro, yGyro, zGyro;

    IMU.readAcceleration(AccX, AccY, AccZ);
    IMU.readGyroscope(xGyro, yGyro, zGyro);

    RateYaw = zGyro * gyroSensitivity;
    RateRoll = xGyro * gyroSensitivity;
    RatePitch = yGyro * gyroSensitivity;

    // Continuous roll, pitch, and yaw update based on gyroscope data, corrected for bias
    float gx_dps = RateRoll - gx_bias;             // Calculate degrees per second
    float gy_dps = RatePitch - gy_bias;            // Calculate degrees per second
    float gz_dps = RateYaw - gz_bias;              // Calculate degrees per second
    float dt = (micros() - LoopTimer) / 1000000.0; // Calculate elapsed time in seconds

    Roll += gx_dps * dt;
    Pitch += gy_dps * dt;
    Yaw += gz_dps * dt;

    AngleRoll = 900 * Roll;
    AnglePitch = 900 * Pitch;
    AngleYaw = 900 * Yaw;
}

void printImuMeasurement()
{
    // Prepare data to print
    String bleAddressPrinter = "Connected to central: " + String(bleAddress);
    String AngleRollPrinter = " | Roll Angle [°]: " + String(AngleRoll);
    String AnglePitchPrinter = " | Pitch Angle [°]: " + String(AnglePitch);
    String AngleYawPrinter = " | Yaw Angle [°]: " + String(AngleYaw);
    String bleCommandPrinter = " | Ble Command: " + String(bleCommand);
    String bleDebugMessagePrinter = " |Ble Debug: " + String(bleDebugMessage);

    // Concatonate the message
    String imuDataPrinter = bleAddressPrinter + AngleRollPrinter + AnglePitchPrinter + AngleYawPrinter + bleCommandPrinter + bleDebugMessagePrinter;

    // Printint the entire data
    if (isSerialMonitoringActive)
    {
        Serial.println(imuDataPrinter);
    }
}

void setup()
{
    // Initialize LED pins
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);

    // Turn off all LED colors initially
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);

    // Initialize serial communication
    Serial.begin(9600);
    startTime = millis();
    isSerialMonitoringActive = false;
    while (!isSerialMonitoringActive && millis() - startTime < 5000)
    {
        if (Serial)
        {
            isSerialMonitoringActive = true;
            digitalWrite(LED_BLUE, HIGH);
        }
        else
        {
            digitalWrite(LED_BLUE, LOW);
        }
        delay(10);
    }

    // Initialize the IMU with timeout
    startTime = millis();
    isImuInitialized = false;
    while (!isImuInitialized && millis() - startTime < 5000)
    {
        if (IMU.begin())
        {
            isImuInitialized = true;
            digitalWrite(LED_RED, HIGH);
        }
        else
        {
            digitalWrite(LED_RED, LOW);
            if (isSerialMonitoringActive)
            {
                Serial.println("Failed to initialize IMU!");
            }
        }
        delay(10);
    }

    // Initialize the BLE with timeout
    startTime = millis();
    isBleInitialized = false;
    while (!isBleInitialized && millis() - startTime < 5000)
    {
        if (BLE.begin())
        {
            isBleInitialized = true;
            digitalWrite(LED_RED, HIGH);
        }
        else
        {
            digitalWrite(LED_RED, LOW);
            if (isSerialMonitoringActive)
            {
                Serial.println("Failed to start BLE!");
            }
        }
        delay(10);
    }

    if (isImuInitialized && isBleInitialized)
    {
        // Setup BLE if both IMU and BLE are initialized
        BLE.setLocalName("IMU Sensor");
        BLE.setAdvertisedService(imuService);
        imuService.addCharacteristic(dataCharacteristic);
        BLE.addService(imuService);
        dataCharacteristic.writeValue("0,0,0,");

        BLE.advertise();

        if (isSerialMonitoringActive)
        {
            Serial.println("IMU initialized successfully.");
            Serial.println("Bluetooth device active, waiting for connections...");
        }

        // Turn off all LEDs to indicate success
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_BLUE, HIGH);
    }
    else
    {
        // If initialization fails, indicate by keeping the red LED on
        digitalWrite(LED_RED, LOW); // Red LED for failure
        if (isSerialMonitoringActive)
        {
            Serial.println("Initialization failed!");
        }
    }
}

void loop()
{
    // Handle the connection status
    BLEDevice central = BLE.central();
    if (central.connected())
    {
        if (!isConnected)
        {
            isConnected = true;
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_RED, HIGH);
            digitalWrite(LED_BLUE, HIGH);

            if (isSerialMonitoringActive)
            {
                Serial.println("Connected to device");
            }
        }

        // Check if new accelerometer and gyroscope data is available
        if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable())
        {
            LoopTimer = micros(); // Update LoopTimer at the start of data handling

            if (!isMeasuring)
            {
                // Start calibration if not measuring
                if (sampleCounter == 0)
                {
                    startCalibrateImu();
                }
                else if ((sampleCounter <= numSamples) && isCalibrating)
                {
                    continueCalibrateImu();
                }
                else if ((sampleCounter >= numSamples) && isCalibrating)
                {
                    finishCalibrateImu();
                }
            }
            else
            {
                checkBleInput(central);

                // Execute BLE command
                if (bleCommand == "Reset data")
                {
                    bleDebugMessage += " Command received [" + String(bleCommand) + "] -";
                    bleStatus = " Command received [" + String(bleCommand) + "] ";
                    resetMeasuring();
                    bleDebugMessage += "Reset data done -";
                    bleStatus = "Reset";
                    bleCommand = "None";
                }

                readIMU();
                sendBleOutput();
                printImuMeasurement();
            }
        }
    }
    else if (isConnected)
    {
        isConnected = false;
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_BLUE, HIGH);
        bleDebugMessage += "Disconnected from device -";
        bleStatus = "Disconnected";
        if (isSerialMonitoringActive)
        {
            Serial.println("Disconnected from device");
        }
    }

    // Maintain a loop timing of 40 milliseconds
    while (micros() - LoopTimer < 40000)
        ;
}