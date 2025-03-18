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
        String username = sharedPreferences.getString("LOGGED_IN_USERNAME", "default_username");
        dbHelper = new DatabaseHelper(this);
        userId = dbHelper.getUserIdByUsername(username);
        dbHelper.printAllTables();
        if (username != null && !username.isEmpty()) {
            welcomeMessage.setText("Welcome " + username);

            if (userId != -1) {
                // Display the data on the graph
                displayHeartbeatDataOnGraph(userId, dbHelper, lineChart);
            }
        } else {
            welcomeMessage.setText("Welcome User");
        }
    }

    // Display heartbeat data on the graph
    private void displayHeartbeatDataOnGraph(int userId, DatabaseHelper dbHelper, LineChart lineChart) {
        Cursor cursor = dbHelper.getSensorDataByUserId(userId);

        Map<Integer, List<Double>> hourlyData = new TreeMap<>();
        if (cursor != null && cursor.moveToFirst()) {
            int bpmIndex = cursor.getColumnIndex("sensor_data");
            int hourIndex = cursor.getColumnIndex("hour");
            int minuteIndex = cursor.getColumnIndex("minute");

            if (bpmIndex != -1 && hourIndex != -1 && minuteIndex != -1) {
                do {
                    double bpm = cursor.getDouble(bpmIndex);
                    int hour = cursor.getInt(hourIndex);
                //    int minute = cursor.getInt(minuteIndex);

                    // Group data by hour
                    hourlyData.putIfAbsent(hour, new ArrayList<>());
                    hourlyData.get(hour).add(bpm);

                } while (cursor.moveToNext());
            }
            cursor.close();
        }

        ArrayList<Entry> entries = new ArrayList<>();
        ArrayList<String> hourLabels = new ArrayList<>();

        for (Map.Entry<Integer, List<Double>> entry : hourlyData.entrySet()) {
            int hour = entry.getKey();
            List<Double> values = entry.getValue();
            int avgHeartRate = (int) Math.round(values.stream().mapToDouble(Double::doubleValue).average().orElse(0.0));

            // Use actual hour as x-value (convert to float for correct chart scaling)
            entries.add(new Entry(hour, avgHeartRate));
            hourLabels.add(String.valueOf(hour)); // Label for x-axis
        }

        if (!entries.isEmpty()) {
            LineDataSet dataSet = new LineDataSet(entries, "Avg Heart Rate per Hour (bpm)");
            dataSet.setColor(Color.RED);
            dataSet.setLineWidth(2f);
            dataSet.setDrawCircles(true);

            LineData lineData = new LineData(dataSet);
            lineChart.setData(lineData);

            XAxis xAxis = lineChart.getXAxis();
            xAxis.setValueFormatter(new IndexAxisValueFormatter(hourLabels)); // Show actual hours
            xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
            xAxis.setGranularity(1f);  // Ensures each tick represents an hour
            xAxis.setLabelCount(hourLabels.size(), true);

            lineChart.invalidate(); // Refresh chart
        }
    }
    public void updateGraph() {
        runOnUiThread(() -> {
            Log.d("GraphDebug", "Updating Graph...");
            displayHeartbeatDataOnGraph(userId, dbHelper, lineChart);
        });
    }
}

