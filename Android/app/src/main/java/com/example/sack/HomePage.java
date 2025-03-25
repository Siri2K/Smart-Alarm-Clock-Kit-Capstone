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
import com.github.mikephil.charting.formatter.ValueFormatter;
import com.google.android.material.bottomnavigation.BottomNavigationView;
import com.github.mikephil.charting.charts.LineChart;

import java.sql.Date;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.HashMap;
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
    private boolean useDummyData = false; // Set to false when using actual data

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

        if (getIntent().getBooleanExtra("UPDATE_BEDTIME", false)) {
            saveAndSendBedtime();
        }
        dbHelper.insertHeartbeatData(1, 72, 22, 30, 0);
        dbHelper.insertHeartbeatData(1, 80, 23, 0, 2);
        dbHelper.insertHeartbeatData(1, 65, 23, 30, 3);
        dbHelper.insertHeartbeatData(1, 85, 0, 30, 1);
        dbHelper.insertHeartbeatData(1, 72.5, 1, 0, 2 );
        dbHelper.insertHeartbeatData(1, 70, 3, 0, 3);
        dbHelper.insertHeartbeatData(1, 95, 4, 0, 2);
        dbHelper.insertHeartbeatData(1, 85, 6, 30, 0);
        LineChart sleepChart = findViewById(R.id.sleepChart);
        List<SleepStageEntry> dummySleepStages = Arrays.asList(
                new SleepStageEntry(0, 22 * 60 + 30),  // 22:30 - Wake
                new SleepStageEntry(2, 23 * 60 + 0),   // 23:00 - Light
                new SleepStageEntry(3, 23 * 60 + 30),  // 23:30 - Deep
                new SleepStageEntry(1, 0 * 60 + 30),   // 00:30 - REM
                new SleepStageEntry(2, 1 * 60 + 0),    // 01:00 - Light
                new SleepStageEntry(3, 2 * 60 + 0),    // 02:00 - Deep
                new SleepStageEntry(1, 3 * 60 + 0),    // 03:00 - REM
                new SleepStageEntry(2, 4 * 60 + 0),    // 04:00 - Light
                new SleepStageEntry(0, 6 * 60 + 30)    // 06:30 - Wake
        );
        displaySleepStages(dummySleepStages, sleepChart);


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
            xAxis.setValueFormatter(new ValueFormatter() {
                private final SimpleDateFormat sdf = new SimpleDateFormat("HH:mm", Locale.getDefault());

                @Override
                public String getFormattedValue(float value) {
                    int hour = ((int) value) % 24; // 24 = 00:00, 25 = 01:00, etc.
                    Calendar calendar = Calendar.getInstance();
                    calendar.set(Calendar.HOUR_OF_DAY, hour);
                    calendar.set(Calendar.MINUTE, 0);
                    return sdf.format(calendar.getTime());
                }
            });
            xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
            xAxis.setGranularity(1f);
            xAxis.setLabelCount(hourLabels.size(), true);
            xAxis.setTextColor(Color.parseColor("#FF6B6B"));
            xAxis.setAxisMinimum(20f);  // 22:00
            xAxis.setAxisMaximum(35f);
            // Customize Y-Axis
            YAxis yAxisLeft = lineChart.getAxisLeft();
            yAxisLeft.setTextColor(Color.parseColor("#FF6B6B"));
            YAxis yAxisRight = lineChart.getAxisRight();
            yAxisRight.setTextColor(Color.parseColor("#FF6B6B"));

            lineChart.invalidate();
        }
    }
    public static class SleepStageEntry {
        public int stage;
        public float timeInMinutes;

        public SleepStageEntry(int stage, float timeInMinutes) {
            this.stage = stage;
            this.timeInMinutes = timeInMinutes;
        }
    }

    private void displaySleepStages(List<SleepStageEntry> stages, LineChart sleepChart) {
        List<Entry> entries = new ArrayList<>();

        for (int i = 0; i < stages.size(); i++) {
            SleepStageEntry current = stages.get(i);
            int stageValue = current.stage;

            float xStart = current.timeInMinutes;
            if (xStart < 1320) xStart += 1440; // shift early morning to next day
            xStart -= 1320; // normalize 22:00 to 0

            float xEnd;
            if (i + 1 < stages.size()) {
                xEnd = stages.get(i + 1).timeInMinutes;
                if (xEnd < 1320) xEnd += 1440;
                xEnd -= 1320;
            } else {
                xEnd = xStart + 30;
            }

            entries.add(new Entry(xStart, stageValue));
            entries.add(new Entry(xEnd, stageValue));
        }

        LineDataSet dataSet = new LineDataSet(entries, "Sleep Cycle");
        dataSet.setDrawValues(false);
        dataSet.setDrawCircles(false);
        dataSet.setColor(Color.CYAN);
        dataSet.setLineWidth(2f);

        LineData lineData = new LineData(dataSet);
        sleepChart.setData(lineData);

        XAxis x_axis = sleepChart.getXAxis();
        x_axis.setAxisMinimum(0);       // 22:00
        x_axis.setAxisMaximum(540);     // 07:00 (540 minutes after 22:00)
        x_axis.setTextColor(Color.parseColor("#FF6B6B"));
        x_axis.setGranularity(60f);
        x_axis.setLabelCount(8, true);

        // Format X-axis to HH:mm
        x_axis.setValueFormatter(new ValueFormatter() {
            private final SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm", Locale.getDefault());

            @Override
            public String getFormattedValue(float value) {
                Calendar calendar = Calendar.getInstance();
                calendar.set(Calendar.HOUR_OF_DAY, 22);
                calendar.set(Calendar.MINUTE, 0);
                calendar.set(Calendar.SECOND, 0);
                calendar.set(Calendar.MILLISECOND, 0);

                // Add the offset (in minutes) from 22:00
                calendar.add(Calendar.MINUTE, (int) value);
                return timeFormat.format(calendar.getTime());
            }
        });


        // Customize Y-axis labels to show text instead of numbers
        YAxis yAxis = sleepChart.getAxisLeft();
        yAxis.setGranularity(1f);
        yAxis.setTextColor(Color.parseColor("#FF6B6B"));
        yAxis.setValueFormatter(new ValueFormatter() {
            @Override
            public String getFormattedValue(float value) {
                switch ((int) value) {
                    case 0: return "Wake";
                    case 1: return "Light";
                    case 2: return "Deep";
                    case 3: return "REM";
                    default: return "";
                }
            }
        });

        sleepChart.getAxisRight().setEnabled(false);
        sleepChart.getXAxis().setPosition(XAxis.XAxisPosition.BOTTOM);
        sleepChart.getDescription().setEnabled(false);
        sleepChart.invalidate(); // Refresh chart
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
            int adjustedHour;
            if (hour >= 23) {
                adjustedHour = hour; // 22–23 remain as-is
            } else {
                adjustedHour = hour + 24; // 0–6 become 24–30
            }
            entries.add(new Entry(adjustedHour, avgHeartRate));
            Calendar calendar = Calendar.getInstance();
            calendar.set(Calendar.HOUR_OF_DAY, hour);
            calendar.set(Calendar.MINUTE, 0);
            SimpleDateFormat sdf = new SimpleDateFormat("HH:mm", Locale.getDefault());
            hourLabels.add(sdf.format(calendar.getTime()));
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

        SharedPreferences prefs = this.getSharedPreferences("MyPreferences", MODE_PRIVATE);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putString("AVERAGE_BEDTIME", averageBedtime);
        editor.apply();

        // Debugging log
        Log.d("Bedtime", "Optimal bedtime saved: " + optimalBedtime);
    }
    public void updateSleepUI() {
        SharedPreferences prefs = this.getSharedPreferences("MyPreferences", MODE_PRIVATE);
        String averageBedtime = prefs.getString("AVERAGE_BEDTIME", "No bedtime recorded");

        optimalBedTime.setText(averageBedtime);
        Log.d("HomePage", "Displaying bedtime: " + averageBedtime);
    }
    protected void onResume() {
        super.onResume();
        updateSleepUI();
    }
}
