package com.example.sack;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.floatingactionbutton.FloatingActionButton;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

import androidx.recyclerview.widget.ItemTouchHelper;
import androidx.recyclerview.widget.RecyclerView;

public class AlarmSetPage extends AppCompatActivity {
    private TextView tvAlarmDate;
    private RecyclerView recyclerAlarmList;
    private FloatingActionButton btnAddAlarm;
    private AlarmAdapter alarmAdapter;
    private DatabaseHelper databaseHelper;
    private ArrayList<AlarmModel> alarmList;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.alarmsetpage);

        // Initialize UI elements
        tvAlarmDate = findViewById(R.id.tv_alarm_date);
        recyclerAlarmList = findViewById(R.id.recycler_alarm_list);
        btnAddAlarm = findViewById(R.id.btn_add_alarm);

        // Set current date dynamically
        updateCurrentDate();

        // Initialize database and load alarms
        databaseHelper = new DatabaseHelper(this);
        alarmList = new ArrayList<>();
        alarmAdapter = new AlarmAdapter(this, alarmList);
        recyclerAlarmList.setLayoutManager(new LinearLayoutManager(this));
        recyclerAlarmList.setAdapter(alarmAdapter);

        // Load alarms from database
        loadAlarms();

        // Floating Button to Add New Alarm
        btnAddAlarm.setOnClickListener(v -> {
            Intent intent = new Intent(AlarmSetPage.this, TimeSetPage.class);
            startActivityForResult(intent, 1);
        });

        recyclerAlarmList = findViewById(R.id.recycler_alarm_list);
        alarmList = databaseHelper.getAllAlarms();
        alarmAdapter = new AlarmAdapter(this, alarmList);
        recyclerAlarmList.setAdapter(alarmAdapter);

        // Enable swipe-to-delete
        enableSwipeToDelete();
    }

    private void updateCurrentDate() {
        SimpleDateFormat sdf = new SimpleDateFormat("EEEE, dd MMM - HH:mm", Locale.getDefault());
        String currentDateAndTime = sdf.format(new Date());
        tvAlarmDate.setText(currentDateAndTime);
    }

    private void loadAlarms() {
        alarmList.clear();  // Clear old alarms
        alarmList.addAll(databaseHelper.getAllAlarms());  // Fetch latest alarms

        if (alarmList.isEmpty()) {
            System.out.println("DEBUG: No alarms found in the database.");
        } else {
            for (AlarmModel alarm : alarmList) {
                System.out.println("DEBUG: Loaded Alarm - Time: " + alarm.getTime() + ", Repeat: " + alarm.getRepeatDays());
            }
        }

        // Force RecyclerView update by setting a new adapter
        alarmAdapter = new AlarmAdapter(this, alarmList);
        recyclerAlarmList.setAdapter(alarmAdapter);
        alarmAdapter.notifyDataSetChanged();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 1 && resultCode == RESULT_OK) {
            System.out.println("DEBUG: onActivityResult triggered, refreshing alarms...");
            loadAlarms();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        System.out.println("DEBUG: onResume triggered, refreshing alarms...");
        loadAlarms();  // Force UI refresh when returning to the page
    }

    private void enableSwipeToDelete() {
        ItemTouchHelper.SimpleCallback itemTouchHelperCallback = new ItemTouchHelper.SimpleCallback(0, ItemTouchHelper.LEFT) {
            @Override
            public boolean onMove(RecyclerView recyclerView, RecyclerView.ViewHolder viewHolder, RecyclerView.ViewHolder target) {
                return false; // No move action
            }

            @Override
            public void onSwiped(RecyclerView.ViewHolder viewHolder, int direction) {
                int position = viewHolder.getAdapterPosition();
                alarmAdapter.removeAlarm(position); // Remove from UI & DB
                Toast.makeText(AlarmSetPage.this, "Alarm Deleted", Toast.LENGTH_SHORT).show();
            }
        };

        new ItemTouchHelper(itemTouchHelperCallback).attachToRecyclerView(recyclerAlarmList);
    }
}
