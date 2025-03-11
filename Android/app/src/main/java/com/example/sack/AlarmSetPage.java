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
import androidx.recyclerview.widget.ItemTouchHelper;

import com.google.android.material.bottomnavigation.BottomNavigationView;
import com.google.android.material.floatingactionbutton.FloatingActionButton;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

public class AlarmSetPage extends AppCompatActivity {
    private TextView tvAlarmDate, tvNoAlarms; // Added tvNoAlarms for empty state
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
        tvNoAlarms = findViewById(R.id.tv_no_alarms); // Get empty state message
        recyclerAlarmList = findViewById(R.id.recycler_alarm_list);
        btnAddAlarm = findViewById(R.id.btn_add_alarm);
        BottomNavigationView bottomNavigationView = findViewById(R.id.bottom_navigation);
        NavigationBar.setupNavigation(this, bottomNavigationView);
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

        // Check if alarm list is empty
        if (alarmList.isEmpty()) {
            tvNoAlarms.setVisibility(View.VISIBLE); // Show "Create your first alarm now!"
            recyclerAlarmList.setVisibility(View.GONE); // Hide RecyclerView
            System.out.println("DEBUG: No alarms found, showing empty state message.");
        } else {
            tvNoAlarms.setVisibility(View.GONE); // Hide empty message
            recyclerAlarmList.setVisibility(View.VISIBLE); // Show RecyclerView
            for (AlarmModel alarm : alarmList) {
                System.out.println("DEBUG: Loaded Alarm - Time: " + alarm.getTime() + ", Repeat: " + alarm.getRepeatDays());
            }
        }

        // Update RecyclerView
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
        loadAlarms();  // Refresh alarms on return to this screen
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
                databaseHelper.deleteAlarm(alarmList.get(position).getId()); // Remove from DB
                alarmList.remove(position); // Remove from list
                alarmAdapter.notifyItemRemoved(position);

                // Check if the list is empty after deleting an alarm
                if (alarmList.isEmpty()) {
                    tvNoAlarms.setVisibility(View.VISIBLE);
                    recyclerAlarmList.setVisibility(View.GONE);
                }

                Toast.makeText(AlarmSetPage.this, "Alarm Deleted", Toast.LENGTH_SHORT).show();
            }
        };

        new ItemTouchHelper(itemTouchHelperCallback).attachToRecyclerView(recyclerAlarmList);
    }
}
