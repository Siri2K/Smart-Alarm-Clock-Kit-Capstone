package com.example.sack;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.Button;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import java.util.ArrayList;

public class TimeSetPage extends AppCompatActivity {

    private TimePicker timePicker;
    private TextView tvSelectedDays;
    private Switch switchAlarmSound, switchVibrate;
    private Button btnSave, btnCancel;
    private ArrayList<String> selectedDays;
    private DatabaseHelper databaseHelper;
    private int userId = 1; // Replace with actual logged-in user ID

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.timesetpage);

        // Initialize UI components
        timePicker = findViewById(R.id.time_picker);
        tvSelectedDays = findViewById(R.id.tv_selected_days);
        switchAlarmSound = findViewById(R.id.switch_alarm_sound);
        switchVibrate = findViewById(R.id.switch_vibrate);
        btnSave = findViewById(R.id.btn_save);
        btnCancel = findViewById(R.id.btn_cancel_set);
        databaseHelper = new DatabaseHelper(this);
        selectedDays = new ArrayList<>();

        // Ensure TimePicker uses spinner mode and AM/PM selection
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            timePicker.setIs24HourView(false); // Ensure AM/PM is available
            timePicker.setDescendantFocusability(TimePicker.FOCUS_BLOCK_DESCENDANTS); // Prevent manual input
        }

        // Apply white text color manually on startup
        applyTimePickerTextColor(timePicker);

        // 🔥 Add listener to dynamically fix colors when scrolling
        timePicker.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                applyTimePickerTextColor(timePicker); // Reapply colors on layout updates
            }
        });

        // Setup click listeners for day selection
        setupDaySelection();

        // Handle Cancel Button
        btnCancel.setOnClickListener(v -> finish());

        // Handle Save Button
        btnSave.setOnClickListener(v -> saveAlarm());
    }

    /**
     * Recursively applies white text color to TimePicker elements.
     */
    private void applyTimePickerTextColor(ViewGroup viewGroup) {
        for (int i = 0; i < viewGroup.getChildCount(); i++) {
            View child = viewGroup.getChildAt(i);
            if (child instanceof TextView) {
                ((TextView) child).setTextColor(Color.WHITE);  // Force white text
            } else if (child instanceof ViewGroup) {
                applyTimePickerTextColor((ViewGroup) child);  // Recursively apply to subviews
            }
        }
    }

    private void setupDaySelection() {
        int[] dayIds = {R.id.btn_sun, R.id.btn_mon, R.id.btn_tue, R.id.btn_wed, R.id.btn_thu, R.id.btn_fri, R.id.btn_sat};
        String[] dayLabels = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

        for (int i = 0; i < dayIds.length; i++) {
            TextView dayView = findViewById(dayIds[i]);
            String day = dayLabels[i];

            dayView.setOnClickListener(v -> {
                if (selectedDays.contains(day)) {
                    selectedDays.remove(day);
                    dayView.setTextColor(getResources().getColor(android.R.color.darker_gray));
                } else {
                    selectedDays.add(day);
                    dayView.setTextColor(getResources().getColor(android.R.color.holo_red_light));
                }
                updateSelectedDaysText();
            });
        }
    }

    private void updateSelectedDaysText() {
        tvSelectedDays.setText(String.join(", ", selectedDays));
    }

    private void saveAlarm() {
        int hour = timePicker.getHour();
        int minute = timePicker.getMinute();
        String time = String.format("%02d:%02d", hour, minute);
        String repeatDays = selectedDays.isEmpty() ? "None" : String.join(", ", selectedDays);

        long result = databaseHelper.insertAlarm(userId, time, "Alarm", repeatDays);

        if (result > 0) {
            System.out.println("DEBUG: Alarm successfully saved - Time: " + time + ", Repeat: " + repeatDays);
            Toast.makeText(this, "Alarm Saved!", Toast.LENGTH_SHORT).show();

            // Ensure Activity Result is returned properly
            Intent returnIntent = new Intent();
            setResult(RESULT_OK, returnIntent);
            finish();
        } else {
            System.out.println("DEBUG: Failed to save alarm.");
            Toast.makeText(this, "Failed to save alarm", Toast.LENGTH_SHORT).show();
        }
    }
}
