

package com.example.sack;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

public class HomePage extends AppCompatActivity implements BLEManager.BLEConnectionListener {
    private BLEManager bleManager;
    private static final String TARGET_DEVICE = "CLOCK-BLE";
    private TextView connectionStatus;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.homepage);

        connectionStatus = findViewById(R.id.connectionStatus); // Ensure you have a TextView with this id in your XML

        // Check for BLE permissions on Android 12 and above.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN)
                    != PackageManager.PERMISSION_GRANTED ||
                    ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION)
                            != PackageManager.PERMISSION_GRANTED ||
                    ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT)
                            != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT},
                        1);
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, 2);
            }
        }

        bleManager = new BLEManager(this, TARGET_DEVICE);
        bleManager.setBLEConnectionListener(this);

        Button bleButton = findViewById(R.id.buttonBLE);
        bleButton.setOnClickListener(v -> {
            bleManager.startScan();
        });
    }

    @Override
    public void onConnected(String deviceAddress) {
        runOnUiThread(() -> connectionStatus.setText("Connected: " + deviceAddress));
    }

    @Override
    public void onDisconnected(String deviceAddress) {
        runOnUiThread(() -> connectionStatus.setText("Disconnected: " + deviceAddress));
    }

    @Override
    public void onConnectionFailed(String deviceAddress, int status) {
        runOnUiThread(() -> connectionStatus.setText("Connection failed: " + deviceAddress + " (Status: " + status + ")"));
    }
}
