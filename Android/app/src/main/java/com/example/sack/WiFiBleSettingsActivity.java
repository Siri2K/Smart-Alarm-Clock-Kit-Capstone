package com.example.sack;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.util.List;

public class WiFiBleSettingsActivity extends AppCompatActivity {

    private EditText etSSID, etPassword, etMacAddress, etSKU, etApiKey;
    private Button btnUpdate, btnClear, btnFetchDevices;
    private RecyclerView recyclerView;
    private DeviceAdapter deviceAdapter;
    private DatabaseHelper dbHelper;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wifiblesettings); // Ensure this matches your XML file name

        dbHelper = new DatabaseHelper(this);

        // Initialize UI components
        etSSID = findViewById(R.id.et_ssid);
        etPassword = findViewById(R.id.et_password);
        etMacAddress = findViewById(R.id.et_mac_address);
        etSKU = findViewById(R.id.et_sku);
        etApiKey = findViewById(R.id.et_api_key);
        btnUpdate = findViewById(R.id.btn_update);
        btnClear = findViewById(R.id.btn_clear);
        btnFetchDevices = findViewById(R.id.btn_fetch_devices);
        recyclerView = findViewById(R.id.recyclerView);

        // Setup RecyclerView
        recyclerView.setLayoutManager(new LinearLayoutManager(this));

        // Load saved WiFi and Bulb information
        loadSavedData();

        // Update button click listener
        btnUpdate.setOnClickListener(v -> saveData());

        // Clear button click listener
        btnClear.setOnClickListener(v -> clearFields());

        // Fetch devices from Govee API when the button is clicked
        btnFetchDevices.setOnClickListener(v -> fetchDevices());
    }

    /** ======================== Load Existing Data ======================== **/
    private void loadSavedData() {
        // Load WiFi settings from the database
        String[] wifiData = dbHelper.getWiFiSettings();
        if (wifiData != null) {
            etSSID.setText(wifiData[0]);  // Load SSID
            etPassword.setText(wifiData[1]);  // Load Password
        }

        // Load Bulb settings from the database
        String[] bulbData = dbHelper.getBulbSettings();
        if (bulbData != null) {
            etMacAddress.setText(bulbData[0]);  // Load MAC Address
            etSKU.setText(bulbData[1]);  // Load SKU
            etApiKey.setText(bulbData[2]);  // Load API Key
        }
    }

    /** ======================== Save or Update Data ======================== **/
    private void saveData() {
        String ssid = etSSID.getText().toString().trim();
        String password = etPassword.getText().toString().trim();
        String macAddress = etMacAddress.getText().toString().trim();
        String sku = etSKU.getText().toString().trim();
        String apiKey = etApiKey.getText().toString().trim();

        // Validate inputs
        if (ssid.isEmpty() || password.isEmpty()) {
            Toast.makeText(this, "Please enter WiFi SSID and Password", Toast.LENGTH_SHORT).show();
            return;
        }
        if (macAddress.isEmpty() || sku.isEmpty() || apiKey.isEmpty()) {
            Toast.makeText(this, "Please enter all Bulb details", Toast.LENGTH_SHORT).show();
            return;
        }

        // Save to database
        dbHelper.saveOrUpdateWiFiSettings(ssid, password);
        dbHelper.saveOrUpdateBulbSettings(macAddress, sku, apiKey);

        // Show confirmation
        Toast.makeText(this, "Settings Updated Successfully!", Toast.LENGTH_SHORT).show();
    }

    /** ======================== Clear Fields ======================== **/
    private void clearFields() {
        etSSID.setText("");
        etPassword.setText("");
        etMacAddress.setText("");
        etSKU.setText("");
        etApiKey.setText("");
    }

    /** ======================== Fetch Govee Devices ======================== **/
    private void fetchDevices() {
        String apiKey = etApiKey.getText().toString().trim();
        if (apiKey.isEmpty()) {
            Toast.makeText(this, "Enter API Key first", Toast.LENGTH_SHORT).show();
            return;
        }

        // Fetch devices from Govee API
        GoveeAPIHelper.fetchDevices(apiKey, devices -> {
            if (devices != null) {
                deviceAdapter = new DeviceAdapter(devices);
                recyclerView.setAdapter(deviceAdapter);
            } else {
                Toast.makeText(this, "Failed to fetch devices", Toast.LENGTH_SHORT).show();
            }
        });
    }
}
