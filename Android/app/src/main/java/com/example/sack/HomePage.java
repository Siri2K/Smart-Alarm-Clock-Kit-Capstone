package com.example.sack;

import android.annotation.SuppressLint;
import android.database.Cursor;
import android.graphics.Color;
import android.os.Bundle;
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

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

public class HomePage extends AppCompatActivity {
    @SuppressLint("SetTextI18n")
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.homepage);
        TextView welcomeMessage = findViewById(R.id.welcomeMessage);
        BottomNavigationView bottomNavigationView = findViewById(R.id.bottom_navigation);
        NavigationBar.setupNavigation(this, bottomNavigationView);
        LineChart lineChart = findViewById(R.id.lineChart);
        String username = getIntent().getStringExtra("USERNAME");
        DatabaseHelper dbHelper = new DatabaseHelper(this);
        if (username != null && !username.isEmpty()) {
            welcomeMessage.setText("Welcome " + username);
            int userId = dbHelper.getUserIdByUsername(username);
            if (userId != -1) {
                insertDummyData(dbHelper, userId);
                // Display the data on the graph
                displayHeartbeatDataOnGraph(userId, dbHelper, lineChart);
            }
        } else {
            welcomeMessage.setText("Welcome User");
        }

    }

    private void insertDummyData(DatabaseHelper dbHelper, int userId) {
        long currentTime = System.currentTimeMillis(); // Current timestamp

        // Simulate 12 hours of data (one entry every 10 minutes)
        for (int i = 0; i < 72; i++) { // 12 hours * 6 readings per hour
            long timestamp = currentTime - ((72 - i) * 10 * 60 * 1000); // Every 10 minutes
            double heartRate = 60 + Math.random() * 30; // Simulate random heart rate between 60-90 BPM

            dbHelper.insertSensorData(userId, heartRate, timestamp);
        }

        System.out.println("DEBUG: Inserted 72 dummy heartbeat records.");
    }


    // Display heartbeat data on the graph
    private void displayHeartbeatDataOnGraph(int userId, DatabaseHelper dbHelper, LineChart lineChart) {
        Cursor cursor = dbHelper.getSensorDataByUserId(userId);

        Map<Long, List<Double>> hourlyData = new TreeMap<>();
        if (cursor != null && cursor.moveToFirst()) {
            int sensorDataIndex = cursor.getColumnIndex("sensor_data");
            int timestampIndex = cursor.getColumnIndex("timestamp");

            if (sensorDataIndex != -1 && timestampIndex != -1) {
                do {
                    double sensorData = cursor.getDouble(sensorDataIndex);
                    long timestamp = cursor.getLong(timestampIndex);

                    // Convert timestamp to hour block
                    long hour = (timestamp / (1000 * 60 * 60)) * (1000 * 60 * 60); // Round to hour

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
        for (Map.Entry<Long, List<Double>> entry : hourlyData.entrySet()) {
            long hour = entry.getKey();
            List<Double> values = entry.getValue();
            int avgHeartRate = (int) Math.round(values.stream().mapToDouble(Double::doubleValue).average().orElse(0.0));

            entries.add(new Entry(hourCounter,avgHeartRate));
            hourLabels.add(String.valueOf(hourCounter)); // X-axis labels 1 to 12

            hourCounter++;
            if (hourCounter > 12) break;
        }

        if (!entries.isEmpty()) {
            LineDataSet dataSet = new LineDataSet(entries, "Avg Heart Rate per Hour (bpm)");
            dataSet.setColor(Color.BLUE);
            dataSet.setLineWidth(2f);
            dataSet.setDrawValues(false);
            dataSet.setDrawCircles(true);
            dataSet.setDrawVerticalHighlightIndicator(true);

            LineData lineData = new LineData(dataSet);
            lineData.setValueFormatter(new IntegerValueFormatter());
            lineChart.setData(lineData);
            Legend legend = lineChart.getLegend();
            legend.setTextColor(Color.WHITE);
            //Customize style to X and Y Axes
            XAxis xAxis = lineChart.getXAxis();
            xAxis.setTextColor(Color.parseColor("#FF6B6B"));
            xAxis.setAxisLineColor(Color.parseColor("#FF6B6B"));
            xAxis.setGridColor(Color.DKGRAY);
            xAxis.setLabelCount(12, true);
            xAxis.setGranularity(1f);
            xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
            xAxis.setValueFormatter(new IndexAxisValueFormatter(hourLabels)); // Dynamically assign labels from 1 to 12

            YAxis leftAxis = lineChart.getAxisLeft();
            leftAxis.setValueFormatter(new IntegerValueFormatter());
            leftAxis.setTextColor(Color.parseColor("#FF6B6B"));
            leftAxis.setAxisLineColor(Color.parseColor("#FF6B6B"));
            leftAxis.setGridColor(Color.DKGRAY);

            YAxis rightAxis = lineChart.getAxisRight();
            rightAxis.setTextColor(Color.parseColor("#FF6B6B"));
            rightAxis.setAxisLineColor(Color.parseColor("#FF6B6B"));
            rightAxis.setGridColor(Color.DKGRAY);
            System.out.println("Hour Labels: " + hourLabels);
            System.out.println("Entries: " + entries);
            // Refresh the chart
            lineChart.getDescription().setEnabled(false);
            lineChart.invalidate();
        } else {
            System.out.println("DEBUG: No valid data to display on the graph.");
        }
    }


}

