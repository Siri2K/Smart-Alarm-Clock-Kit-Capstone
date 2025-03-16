package com.example.sack;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;

public class GoveeAPIHelper {

    private static final String API_URL = "https://developer-api.govee.com/v1/devices";
    private static final String TAG = "GoveeAPI";

    public interface DeviceCallback {
        void onResponse(List<String> devices);
    }

    public static void fetchDevices(String apiKey, DeviceCallback callback) {
        Executors.newSingleThreadExecutor().execute(() -> {
            try {
                URL url = new URL(API_URL);
                HttpURLConnection connection = (HttpURLConnection) url.openConnection();
                connection.setRequestMethod("GET");
                connection.setRequestProperty("Govee-API-Key", apiKey);
                connection.setRequestProperty("Content-Type", "application/json");

                int responseCode = connection.getResponseCode();
                if (responseCode == HttpURLConnection.HTTP_OK) {
                    BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
                    StringBuilder response = new StringBuilder();
                    String line;
                    while ((line = reader.readLine()) != null) {
                        response.append(line);
                    }
                    reader.close();

                    List<String> devices = parseDeviceResponse(response.toString());
                    new Handler(Looper.getMainLooper()).post(() -> callback.onResponse(devices));

                } else {
                    Log.e(TAG, "Error fetching devices: " + responseCode);
                    new Handler(Looper.getMainLooper()).post(() -> callback.onResponse(null));
                }

                connection.disconnect();

            } catch (Exception e) {
                Log.e(TAG, "Exception: " + e.getMessage());
                new Handler(Looper.getMainLooper()).post(() -> callback.onResponse(null));
            }
        });
    }

    private static List<String> parseDeviceResponse(String json) {
        List<String> devices = new ArrayList<>();
        try {
            JSONObject jsonObject = new JSONObject(json);

            //The key to access is "devices" inside "data"
            JSONObject dataObject = jsonObject.getJSONObject("data");
            JSONArray deviceArray = dataObject.getJSONArray("devices");

            for (int i = 0; i < deviceArray.length(); i++) {
                JSONObject device = deviceArray.getJSONObject(i);
                String deviceName = device.getString("deviceName");
                String model = device.getString("model");
                String macAddress = device.getString("device");

                // Combine details into one string for display
                String formattedDevice = deviceName + " (" + model + ") - " + macAddress;
                devices.add(formattedDevice);
            }
        } catch (Exception e) {
            Log.e(TAG, "Error parsing JSON: " + e.getMessage());
        }
        return devices;
    }

}
