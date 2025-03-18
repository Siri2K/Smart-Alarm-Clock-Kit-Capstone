package com.example.sack;

import android.annotation.SuppressLint;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.github.mikephil.charting.components.Legend;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.formatter.IndexAxisValueFormatter;
import com.google.android.material.bottomnavigation.BottomNavigationView;
import com.github.mikephil.charting.charts.LineChart;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.TreeMap;

public class HomePage extends AppCompatActivity {
    private int userId; // Make userId a class-level variable
    private DatabaseHelper dbHelper;
    private LineChart lineChart;
    @SuppressLint("SetTextI18n")
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.homepage);
        TextView welcomeMessage = findViewById(R.id.welcomeMessage);
        BottomNavigationView bottomNavigationView = findViewById(R.id.bottom_navigation);
        NavigationBar.setupNavigation(this, bottomNavigationView);
        lineChart = findViewById(R.id.lineChart);
        SharedPreferences sharedPreferences = getSharedPreferences("MyPreferences", MODE_PRIVATE);
        String username = sharedPreferences.getString("USERNAME", "default_username"); // Default if not found
        dbHelper = new DatabaseHelper(this);
        userId = dbHelper.getUserIdByUsername(username);
        if (username != null && !username.isEmpty()) {
            welcomeMessage.setText("Welcome " + username);

            if (userId != -1) {
                // Display the data on the graph
                displayHeartbeatDataOnGraph(userId, dbHelper, lineChart);
            }
        } else {
            welcomeMessage.setText("Welcome User");
        }
        File databasePath = getDatabasePath("Alarm_DB");
        Log.d("DatabaseDebug", "Database Path: " + databasePath.getAbsolutePath());
    }

    public void insertActualData(DatabaseHelper dbHelper, int userId, int bpm, int hour, int minute) {
        // Generate actual timestamp using ESP-provided hour & minute
        String formattedTimestamp = generateTimestampFromESP(hour, minute);

        // Insert the actual BPM and timestamp into the database
        dbHelper.insertSensorData(userId, bpm, formattedTimestamp);
        dbHelper.printAllTables();
    }


    // Display heartbeat data on the graph
    private void displayHeartbeatDataOnGraph(int userId, DatabaseHelper dbHelper, LineChart lineChart) {
        Cursor cursor = dbHelper.getSensorDataByUserId(userId);

        // TreeMap keeps the data sorted by timestamp
        Map<String, List<Double>> hourlyData = new TreeMap<>();
        if (cursor != null && cursor.moveToFirst()) {
            int sensorDataIndex = cursor.getColumnIndex("sensor_data");
            int timestampIndex = cursor.getColumnIndex("timestamp");

            if (sensorDataIndex != -1 && timestampIndex != -1) {
                do {
                    double sensorData = cursor.getDouble(sensorDataIndex);
                    String timestamp = cursor.getString(timestampIndex);

                    // Extract only hour from the timestamp (HH:00 format)
                    String hour = formatTimestampToHour(timestamp);

                    // Group data by hour
                    hourlyData.putIfAbsent(hour, new ArrayList<>());
                    hourlyData.get(hour).add(sensorData);

                } while (cursor.moveToNext());
            }
            cursor.close();
        }

        ArrayList<Entry> entries = new ArrayList<>();
        ArrayList<String> hourLabels = new ArrayList<>();
        int hourCounter = 1; // Start from hour 1

        for (Map.Entry<String, List<Double>> entry : hourlyData.entrySet()) {
            String hour = entry.getKey(); // Hour in HH:00 format
            List<Double> values = entry.getValue();

            // Calculate average BPM for the hour
            int avgHeartRate = (int) Math.round(values.stream().mapToDouble(Double::doubleValue).average().orElse(0.0));

            // Add data points for the graph
            entries.add(new Entry(hourCounter, avgHeartRate));
            hourLabels.add(String.valueOf(Integer.parseInt(hour)));

            hourCounter++;
            if (hourCounter > 12) break; // Show only the latest 12 hours
        }

        if (!entries.isEmpty()) {
            LineDataSet dataSet = new LineDataSet(entries, "Avg Heart Rate per Hour (bpm)");
            dataSet.setColor(Color.RED);
            dataSet.setLineWidth(2f);
            dataSet.setDrawValues(false);
            dataSet.setDrawCircles(true);
            dataSet.setDrawVerticalHighlightIndicator(true);

            LineData lineData = new LineData(dataSet);
            lineData.setValueFormatter(new IntegerValueFormatter());
            lineChart.setData(lineData);
            System.out.println("DEBUG: Hour Labels: " + hourLabels);
            XAxis xAxis = lineChart.getXAxis();
            xAxis.setTextColor(Color.WHITE);
            xAxis.setAxisLineColor(Color.RED);
            xAxis.setGridColor(Color.DKGRAY);
            xAxis.setLabelCount(hourLabels.size(), true);
            xAxis.setGranularity(1f);
            xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
            xAxis.setValueFormatter(new IndexAxisValueFormatter(hourLabels)); // Ensure formatted labels
            xAxis.setAvoidFirstLastClipping(true);
            xAxis.setLabelRotationAngle(0);



            // Customize the Y-axis
            YAxis leftAxis = lineChart.getAxisLeft();
            leftAxis.setValueFormatter(new IntegerValueFormatter());
            leftAxis.setTextColor(Color.WHITE);
            leftAxis.setAxisLineColor(Color.RED);
            leftAxis.setGridColor(Color.DKGRAY);

            YAxis rightAxis = lineChart.getAxisRight();
            rightAxis.setEnabled(false);

            // Customize legend and appearance
            Legend legend = lineChart.getLegend();
            legend.setTextColor(Color.WHITE);

            System.out.println("Hour Labels: " + hourLabels);
            System.out.println("Entries: " + entries);

            // Refresh the chart
            lineChart.getDescription().setEnabled(false);
            lineChart.invalidate();
        } else {
            System.out.println("DEBUG: No valid data to display on the graph.");
        }
    }
    private String formatTimestampToHour(String timestamp) {
        try {
            SimpleDateFormat inputFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault());
            SimpleDateFormat outputFormat = new SimpleDateFormat("H", Locale.getDefault());

            Date date = inputFormat.parse(timestamp);
            return outputFormat.format(date); // Returns formatted hour like "08:00"
        } catch (Exception e) {
            e.printStackTrace();
            return timestamp; // Return original timestamp if parsing fails
        }
    }



    private String generateTimestampFromESP(int hour, int minute) {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault());
        Calendar calendar = Calendar.getInstance();

        // Use the current date but replace the hour and minute with the ESP values
        calendar.set(Calendar.HOUR_OF_DAY, hour);
        calendar.set(Calendar.MINUTE, minute);
        calendar.set(Calendar.SECOND, 0); // Set seconds to 0 for consistency

        return sdf.format(calendar.getTime()); // Return formatted timestamp
    }
    public void updateGraph() {
        runOnUiThread(() -> {
            displayHeartbeatDataOnGraph(userId, dbHelper, lineChart);
        });
    }
    public int getUserId() {
        return userId;
    }
}

