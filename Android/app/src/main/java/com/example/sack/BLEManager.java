package com.example.sack;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.os.Handler;
import android.util.Log;

import java.util.List;
import java.util.UUID;

public class BLEManager {
    private static final String TAG = "BLEManager";
    private static final long SCAN_PERIOD = 10000; // 10 seconds scan period

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothLeScanner bluetoothLeScanner;
    private BluetoothGatt bluetoothGatt;
    private String targetDeviceName;
    private BLEConnectionListener connectionListener;
    private Context context;
    private Handler handler = new Handler();

    public BLEManager(Context context, String targetDeviceName) {
        this.context = context;
        this.targetDeviceName = targetDeviceName;

        final BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        bluetoothAdapter = bluetoothManager.getAdapter();
        bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner();
    }

    public void setBLEConnectionListener(BLEConnectionListener listener) {
        this.connectionListener = listener;
    }

    public void startScan() {
        handler.postDelayed(() -> {
            if (bluetoothLeScanner != null) {
                bluetoothLeScanner.stopScan(scanCallback);
            }
        }, SCAN_PERIOD);
        Log.e(TAG, "supposed to start callback" );

        bluetoothLeScanner.startScan(scanCallback);
        Log.e(TAG, "supposed to finish callback" );
    }

    private ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            Log.e(TAG, "onScanResult called congrats" );

            super.onScanResult(callbackType, result);
            BluetoothDevice device = result.getDevice();
            String deviceName = device.getName();
            if (deviceName != null && deviceName.equals(targetDeviceName)) {
                Log.e(TAG, "connecting congrats" );

                bluetoothLeScanner.stopScan(scanCallback);
                connectToDevice(device);
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            super.onScanFailed(errorCode);
            Log.e(TAG, "Scan failed with error code: " + errorCode);
        }
    };

    private void connectToDevice(BluetoothDevice device) {
        if (bluetoothGatt != null) {
            bluetoothGatt.close();
            bluetoothGatt = null;
        }
        bluetoothGatt = device.connectGatt(context, false, gattCallback);
    }

    private BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, "Connected to GATT server.");
                if (connectionListener != null) {
                    connectionListener.onConnected(gatt.getDevice().getAddress());
                }
                gatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.i(TAG, "Disconnected from GATT server.");
                if (connectionListener != null) {
                    connectionListener.onDisconnected(gatt.getDevice().getAddress());
                }
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.i(TAG, "Services discovered.");
                List<BluetoothGattService> services = gatt.getServices();
                for (BluetoothGattService service : services) {
                    Log.i(TAG, "Service UUID: " + service.getUuid().toString());
                }
            } else {
                Log.e(TAG, "Service discovery failed with status: " + status);
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicRead(gatt, characteristic, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.i(TAG, "Characteristic read: " + characteristic.getUuid().toString());
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicWrite(gatt, characteristic, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.i(TAG, "Characteristic written: " + characteristic.getUuid().toString());
            }
        }
    };

    public void disconnect() {
        if (bluetoothGatt != null) {
            bluetoothGatt.disconnect();
            bluetoothGatt.close();
            bluetoothGatt = null;
        }
    }

    public interface BLEConnectionListener {
        void onConnected(String deviceAddress);
        void onDisconnected(String deviceAddress);
        void onConnectionFailed(String deviceAddress, int status);
    }
}