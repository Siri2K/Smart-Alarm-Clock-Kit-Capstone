package com.example.sack;

import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.Manifest;
import android.content.pm.PackageManager;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.google.android.material.bottomnavigation.BottomNavigationView;

import java.util.regex.Pattern;


public class ConnectToDevice extends AppCompatActivity {
    private EditText deviceAddressInput;
    private TextView statusTextView;
    private BLEManager bleManager;
    private static final int REQUEST_BLUETOOTH_PERMISSIONS = 1;
    private static final Pattern MAC_ADDRESS_PATTERN =
            Pattern.compile("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$");

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.connecttodevice);

        bleManager = new BLEManager(this);
        setupUI();
    }

    private void setupUI() {
        deviceAddressInput = findViewById(R.id.deviceAddressInput);
        statusTextView = findViewById(R.id.statusTextView);
        Button connectButton = findViewById(R.id.connectButton);
        BottomNavigationView bottomNavigationView = findViewById(R.id.bottom_navigation);
        NavigationBar.setupNavigation(this, bottomNavigationView);
        connectButton.setOnClickListener(v -> {
            String macAddress = deviceAddressInput.getText().toString().trim();
            if (!isValidMacAddress(macAddress)) {
                Toast.makeText(this, "Invalid MAC address! Please check and try again.", Toast.LENGTH_SHORT).show();
                statusTextView.setText("Invalid MAC address!");
                return;
            }

            statusTextView.setText("Connecting to " + macAddress + "...");
            checkPermissions(macAddress);
        });

        // Set up BLE connection callbacks
        bleManager.setConnectionCallback(new BLEManager.ConnectionCallback() {
            @Override
            public boolean onConnected() {
                updateStatus("Connected to ESP32!");
                return true;
            }

            @Override
            public void onDisconnected() {
                updateStatus("Disconnected from ESP32");
            }
        });
    }

    public void updateStatus(String message) {
        runOnUiThread(() -> statusTextView.setText(message));
    }

    private void checkPermissions(String macAddress) {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.BLUETOOTH_SCAN}, REQUEST_BLUETOOTH_PERMISSIONS);
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
}
