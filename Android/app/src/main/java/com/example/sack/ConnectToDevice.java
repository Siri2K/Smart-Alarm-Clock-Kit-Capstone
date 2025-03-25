package com.example.sack;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.google.android.material.bottomnavigation.BottomNavigationView;

import java.util.regex.Pattern;

public class ConnectToDevice extends AppCompatActivity {
    private EditText deviceAddressInput;
    private TextView statusTextView;
    private BLEManager bleManager;
    private DatabaseHelper dbHelper;
    private BluetoothAdapter bluetoothAdapter;
    private static final int REQUEST_BLUETOOTH_PERMISSIONS = 1;
    private static final int REQUEST_ENABLE_BLUETOOTH = 2;

    // MAC address validation pattern
    private static final Pattern MAC_ADDRESS_PATTERN =
            Pattern.compile("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$");

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.connecttodevice);
        setupUI();
    }

    private void setupUI() {
        deviceAddressInput = findViewById(R.id.deviceAddressInput);
        statusTextView = findViewById(R.id.statusTextView);
        Button connectButton = findViewById(R.id.connectButton);
        BottomNavigationView bottomNavigationView = findViewById(R.id.bottom_navigation);
        NavigationBar.setupNavigation(this, bottomNavigationView);

        bleManager = BLEManager.getInstance(this);
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        dbHelper = new DatabaseHelper(this);
        // Restore the saved status message
        String savedStatus = getSharedPreferences("AppPrefs", MODE_PRIVATE)
                .getString("status_message", "Status: Waiting");
        statusTextView.setText(savedStatus);
        dbHelper.clearHeartbeatTable();
        connectButton.setOnClickListener(v -> {
            String macAddress = deviceAddressInput.getText().toString().trim();

            if (!isValidMacAddress(macAddress)) {
                Toast.makeText(this, "Invalid MAC address! Please check and try again.", Toast.LENGTH_SHORT).show();
                updateStatus("Invalid MAC address!");
                return;
            }

            if (!isBluetoothEnabled()) {
                requestBluetoothEnable();
                return;
            }

            updateStatus("Connecting to " + macAddress + "...");
            checkPermissions(macAddress);
        });

        // Set up BLE connection callbacks
        bleManager.setConnectionCallback(new BLEManager.ConnectionCallback() {
            @Override
            public boolean onConnected() {
                updateStatus("Connected to ESP32!");
                Intent intent = new Intent(ConnectToDevice.this, HomePage.class);
                intent.putExtra("UPDATE_BEDTIME", true); // Signal HomePage to update bedtime
                startActivity(intent);
                return true;
            }

            @Override
            public void onDisconnected() {
                updateStatus("Disconnected from ESP32");
            }
        });
    }


    public void updateStatus(String message) {
        runOnUiThread(() -> {
            statusTextView.setText(message);

            // Save the status in SharedPreferences
            getSharedPreferences("AppPrefs", MODE_PRIVATE)
                    .edit()
                    .putString("status_message", message)
                    .apply();
        });
    }


    private void checkPermissions(String macAddress) {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED ||
                ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {

            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT},
                    REQUEST_BLUETOOTH_PERMISSIONS);
        } else {
            connectToDevice(macAddress);
        }
    }

    private void connectToDevice(String macAddress) {
        try {
            bleManager.connectToDevice(macAddress);
        } catch (Exception e) {
            updateStatus("Connection failed! Check MAC address.");
            Toast.makeText(this, "Connection failed! Please check the MAC address.", Toast.LENGTH_SHORT).show();
        }
    }

    private boolean isValidMacAddress(String macAddress) {
        return MAC_ADDRESS_PATTERN.matcher(macAddress).matches();
    }

    private boolean isBluetoothEnabled() {
        return bluetoothAdapter != null && bluetoothAdapter.isEnabled();
    }

    private void requestBluetoothEnable() {
        Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        startActivityForResult(enableBtIntent, REQUEST_ENABLE_BLUETOOTH);
    }
}
