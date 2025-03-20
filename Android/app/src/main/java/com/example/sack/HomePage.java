package com.example.sack;

import android.annotation.SuppressLint;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Color;
import android.icu.text.SimpleDateFormat;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.formatter.IndexAxisValueFormatter;
import com.google.android.material.bottomnavigation.BottomNavigationView;
import com.github.mikephil.charting.charts.LineChart;

import java.sql.Date;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.TreeMap;
import java.util.Random;

public class HomePage extends AppCompatActivity {
    private int userId;
    private DatabaseHelper dbHelper;
    private TextView optimalBedTime;
    private LineChart lineChart;
    private boolean useDummyData = true; // Set to false when using actual data

    @SuppressLint("SetTextI18n")
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.homepage);

        TextView welcomeMessage = findViewById(R.id.welcomeMessage);
        lineChart = findViewById(R.id.lineChart);
        optimalBedTime = findViewById(R.id.optimalBedtime);
        BottomNavigationView bottomNavigationView = findViewById(R.id.bottom_navigation);
        NavigationBar.setupNavigation(this, bottomNavigationView);

        SharedPreferences sharedPreferences = getSharedPreferences("MyPreferences", MODE_PRIVATE);
        String username = sharedPreferences.getString("LOGGED_IN_USERNAME", "default_username");

        dbHelper = new DatabaseHelper(this);
        userId = dbHelper.getUserIdByUsername(username);
//        dbHelper.insertHeartbeatData(1, 72, 1, 30);
//        dbHelper.insertHeartbeatData(1, 80, 1, 30);
//        dbHelper.insertHeartbeatData(1, 65, 1, 30);
//        dbHelper.insertHeartbeatData(1, 85, 2, 30);
//        dbHelper.insertHeartbeatData(1, 72.5, 2, 30);
//        dbHelper.insertHeartbeatData(1, 70, 3, 30);
//        dbHelper.insertHeartbeatData(1, 95, 3, 30);


        if (username != null && !username.isEmpty()) {
            welcomeMessage.setText("Welcome " + username);

            if (userId != -1) {
                displayHeartbeatDataOnGraph(userId);
                //updateSleepUI();
            }
        } else {
            welcomeMessage.setText("Welcome User");
        }
    }

    // Display heartbeat data on the graph (with dummy mode toggle)
    private void displayHeartbeatDataOnGraph(int userId) {
        ArrayList<Entry> entries;
        ArrayList<String> hourLabels = new ArrayList<>();

        if (useDummyData) {
            entries = generateDummyData(hourLabels);
        } else {
            entries = getActualHeartbeatData(userId, hourLabels);
        }

        if (!entries.isEmpty()) {
            LineDataSet dataSet = new LineDataSet(entries, "Avg Heart Rate per Hour (bpm)");
            dataSet.setColor(Color.BLUE);
            dataSet.setLineWidth(2f);
            dataSet.setDrawCircles(true);

            LineData lineData = new LineData(dataSet);
            lineChart.setData(lineData);

            XAxis xAxis = lineChart.getXAxis();
            xAxis.setValueFormatter(new IndexAxisValueFormatter(hourLabels));
            xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
            xAxis.setGranularity(1f);
            xAxis.setLabelCount(hourLabels.size(), true);
            xAxis.setTextColor(Color.parseColor("#FF6B6B"));

            // Customize Y-Axis
            YAxis yAxisLeft = lineChart.getAxisLeft();
            yAxisLeft.setTextColor(Color.parseColor("#FF6B6B"));
            YAxis yAxisRight = lineChart.getAxisRight();
            yAxisRight.setTextColor(Color.parseColor("#FF6B6B"));

            lineChart.invalidate();
        }
    }

    // Generate Dummy Data for Testing
    private ArrayList<Entry> generateDummyData(ArrayList<String> hourLabels) {
        ArrayList<Entry> dummyEntries = new ArrayList<>();
        Random random = new Random();

        for (int i = 0; i < 10; i++) { // Generate 10 random hourly values
            int hour = i + 1;
            int bpm = random.nextInt(40) + 60; // Random heart rate between 60-100

            dummyEntries.add(new Entry(hour, bpm));
            hourLabels.add(String.valueOf(hour)); // Store labels for X-axis
        }

        return dummyEntries;
    }

    // Fetch Actual Data from Database
    private ArrayList<Entry> getActualHeartbeatData(int userId, ArrayList<String> hourLabels) {
        Cursor cursor = dbHelper.getSensorDataByUserId(userId);
        Map<Integer, List<Double>> hourlyData = new TreeMap<>();

        if (cursor != null && cursor.moveToFirst()) {
            int bpmIndex = cursor.getColumnIndex("sensor_data");
            int hourIndex = cursor.getColumnIndex("hour");

            if (bpmIndex != -1 && hourIndex != -1) {
                do {
                    double bpm = cursor.getDouble(bpmIndex);
                    int hour = cursor.getInt(hourIndex);

                    hourlyData.putIfAbsent(hour, new ArrayList<>());
                    hourlyData.get(hour).add(bpm);
                } while (cursor.moveToNext());
            }
            cursor.close();
        }

        ArrayList<Entry> entries = new ArrayList<>();
        for (Map.Entry<Integer, List<Double>> entry : hourlyData.entrySet()) {
            int hour = entry.getKey();
            List<Double> values = entry.getValue();
            int avgHeartRate = (int) Math.round(values.stream().mapToDouble(Double::doubleValue).average().orElse(0.0));

            entries.add(new Entry(hour, avgHeartRate));
            hourLabels.add(String.valueOf(hour));
        }

        return entries;
    }

    public void updateGraph() {
        runOnUiThread(() -> {
            Log.d("GraphDebug", "Updating Graph...");
            displayHeartbeatDataOnGraph(userId);
        });
    }
    private void saveAndSendBedtime() {
        if (userId == -1) {
            Log.e("Bedtime", "User ID not found! Cannot save bedtime.");
            return;
        }

        // Get the current time and add 10 minutes
        long currentTimeMillis = System.currentTimeMillis();
        long optimalTimeMillis = currentTimeMillis + (10 * 60 * 1000); // Add 10 minutes

        // Format the bedtime as HH:mm
        SimpleDateFormat timeFormat = new SimpleDateFormat("hh:mm a", Locale.getDefault());
        String optimalBedtime = timeFormat.format(new Date(optimalTimeMillis));

        // Save bedtime in the database
        dbHelper.insertSleepTime(userId, optimalBedtime);

        // Retrieve the updated average bedtime
        String averageBedtime = dbHelper.getAverageBedtime(userId);

        SharedPreferences prefs = getSharedPreferences("MyPreferences", MODE_PRIVATE);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putString("AVERAGE_BEDTIME", averageBedtime);
        editor.apply();

        // Debugging log
        Log.d("Bedtime", "Optimal bedtime saved: " + optimalBedtime);
    }
    private void updateSleepUI() {
        SharedPreferences prefs = getSharedPreferences("MyPreferences", MODE_PRIVATE);
        String averageBedtime = prefs.getString("AVERAGE_BEDTIME", "No bedtime recorded");

        optimalBedTime.setText(averageBedtime);
        Log.d("HomePage", "Displaying bedtime: " + averageBedtime);
    }

}
