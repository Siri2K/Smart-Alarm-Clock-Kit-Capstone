package com.example.sack;

import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Locale;
import java.util.UUID;

import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothManager;
import android.content.SharedPreferences;
import android.util.Log;
import android.widget.Toast;


public class BLEManager {
    private static final String TAG = "BLEManager";
    private static final String DEVICE_NAME = "MAXM86161_Sensor";

    private static final UUID SERVICE_UUID =
            UUID.fromString("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    private static final UUID CHARACTERISTIC_UUID =
            UUID.fromString("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
    private static final UUID CLIENT_CHARACTERISTIC_CONFIG =
            UUID.fromString("00002902-0000-1000-8000-00805F9B34FB");

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothGatt bluetoothGatt;
    private BluetoothGattCharacteristic characteristic;
    private DataCallback dataCallback;
    private ConnectionCallback connectionCallback;
    private Context context;
    private DatabaseHelper dbHelper;

    public interface DataCallback {
        void onDataReceived(int bpm, int hour, int minute);
    }
    public interface ConnectionCallback {
        boolean onConnected();

        void onDisconnected();
    }
    private static BLEManager instance;

    public static BLEManager getInstance(Context context) {
        if (instance == null) {
            instance = new BLEManager(context);
        }
        return instance;
    }
    public BLEManager(Context context) {
        dbHelper = new DatabaseHelper(context);
        this.context = context;
        BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager != null) {
            bluetoothAdapter = bluetoothManager.getAdapter();
        }
        instance = this;
    }
    public void setDataCallback(DataCallback callback) {

        this.dataCallback = callback;
    }
    public void setConnectionCallback(ConnectionCallback callback) {
        this.connectionCallback = callback;
    }
    public void connectToDevice(String deviceAddress) {
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(deviceAddress);
        bluetoothGatt = device.connectGatt(context, false, gattCallback);
    }
    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.d(TAG, "Connected to GATT server.");
                bluetoothGatt.discoverServices();
                if (connectionCallback != null) {
                    connectionCallback.onConnected();
                }
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.d(TAG, "Disconnected from GATT server.");
                if (connectionCallback != null) {
                    connectionCallback.onDisconnected();
                }
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                BluetoothGattService service = gatt.getService(SERVICE_UUID);
                if (service != null) {
                    characteristic = service.getCharacteristic(CHARACTERISTIC_UUID);
                    if (characteristic != null) {
                        gatt.setCharacteristicNotification(characteristic, true);

                        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(CLIENT_CHARACTERISTIC_CONFIG);
                        if (descriptor != null) {
                            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                            gatt.writeDescriptor(descriptor);
                        } else {
                            Log.w(TAG, "Descriptor not found for notifications.");
                        }
                    } else {
                        Log.w(TAG, "Characteristic not found: " + CHARACTERISTIC_UUID.toString());
                    }
                } else {
                    Log.w(TAG, "Service not found: " + SERVICE_UUID.toString());
                }
            } else {
                Log.w(TAG, "onServicesDiscovered received: " + status);
            }
        }
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            if (CHARACTERISTIC_UUID.equals(characteristic.getUuid())) {
                byte[] rawData = characteristic.getValue();
                if (rawData != null) {
                    String receivedData = new String(rawData, StandardCharsets.UTF_8);
                    Log.d(TAG, "Data received from ESP: " + receivedData);

                    if (receivedData.matches("\\d{2,3}, \\d{1,2}, \\d{1,2}")) {
                        String[] parts = receivedData.split(",");
                        if (parts.length == 3) { // Expected format: "BPM,Hour,Minute"
                            try {
                                int bpm = (int) Double.parseDouble(parts[0]); // Convert BPM to int
                                int hour = Integer.parseInt(parts[1]);
                                int minute = Integer.parseInt(parts[2]);
                                Log.d(TAG, "Parsed Data - BPM: " + bpm + ", Hour: " + hour + ", Minute: " + minute);

                                // Save BPM & timestamp to database
                                SharedPreferences sharedPreferences = context.getSharedPreferences("MyPreferences", Context.MODE_PRIVATE);
                                String loggedInUsername = sharedPreferences.getString("LOGGED_IN_USERNAME", null);
                                int userId = dbHelper.getUserIdByUsername(loggedInUsername);

                                if (userId != -1) {
                                    dbHelper.insertSensorData(userId, bpm, hour, minute);
                                    Log.d(TAG, "BPM saved to database: " + bpm + " at " + hour + ":" + minute);
                                } else {
                                    Log.e(TAG, "Failed to get user ID. BPM not saved.");
                                }

                                // Update UI
                                if (dataCallback != null) {
                                    dataCallback.onDataReceived(bpm, hour, minute);
                                }
                            } catch (Exception e) {
                                Log.e(TAG, "Error decoding BLE data", e);
                            }
                        } else {
                            Log.e(TAG, "Invalid data format after cleaning: " + receivedData);
                        }
                    }
                }
            }
        };
        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.d(TAG, "ESP is ready");
            } else {
                Log.e(TAG, "Failed to send data to ESP. Status: " + status);
            }
        }
    };
    public void sendDataToESP(String ssid, String password, String macAddress, String sku, String apiKey) {
        if (characteristic == null || bluetoothGatt == null) {
            Log.e(TAG, "BLE characteristic not found.");
            return;
        }
        String dataToSend = ssid + "," + password + "," + macAddress + "," + sku + "," + apiKey;
        characteristic.setValue(dataToSend.getBytes(StandardCharsets.UTF_8));
        bluetoothGatt.writeCharacteristic(characteristic);
        Log.d(TAG, "Sent WiFi & Bulb data: " + dataToSend);
    }

    public void sendAlarmDataToESP(DatabaseHelper dbHelper, int userId) {
        String alarmData = dbHelper.getAlarmData(userId);

        if (characteristic != null && bluetoothGatt != null) {
            characteristic.setValue(alarmData.getBytes(StandardCharsets.UTF_8));
            bluetoothGatt.writeCharacteristic(characteristic);
            Log.d(TAG, "Sent Alarm Data: " + alarmData);
        } else {
            Log.e(TAG, "BLE characteristic not found or not connected.");
        }
    }

    public void disconnect() {
        if (bluetoothGatt != null) {
            bluetoothGatt.disconnect();
            bluetoothGatt.close();
            bluetoothGatt = null;
        }
    }

    private void returnToast(String message) {
        if (context instanceof Activity) {
            ((Activity) context).runOnUiThread(() -> Toast.makeText(context, message, Toast.LENGTH_LONG).show());
        }
    }
    public boolean isConnected() {
        if (bluetoothGatt == null) {
            Log.e("BLEManager", "BLE connection check failed: bluetoothGatt is null.");
            return false;
        }
        int connectionState = bluetoothGatt.getConnectionState(bluetoothGatt.getDevice());
        Log.d("BLEManager", "BLE connection state: " + connectionState);
        return connectionState == BluetoothProfile.STATE_CONNECTED;
    }
    private String generateTimestampFromESP(int hour, int minute) {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault());
        Calendar calendar = Calendar.getInstance();

        // Use current date but replace hour and minute with ESP values
        calendar.set(Calendar.HOUR_OF_DAY, hour);
        calendar.set(Calendar.MINUTE, minute);
        calendar.set(Calendar.SECOND, 0); // Set seconds to 0

        return sdf.format(calendar.getTime()); // Return formatted timestamp
    }

}
