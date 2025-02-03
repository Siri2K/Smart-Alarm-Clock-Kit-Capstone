package com.example.sack;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TimePicker;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import java.util.Calendar;

public class AlarmSetPage extends AppCompatActivity {
    private TimePicker timePicker;
    private EditText alarmLabel;
    private Button updateAlarmButton;
    private Database dbHelper;
    private int alarmId;
    private long oldAlarmTime;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        dbHelper = new Database(this);
        timePicker = findViewById(R.id.timePicker);
        alarmLabel = findViewById(R.id.alarmLabel);
        updateAlarmButton = findViewById(R.id.updateAlarmButton);

        // Get alarm details from Intent
        alarmId = getIntent().getIntExtra("ALARM_ID", -1);
        oldAlarmTime = getIntent().getLongExtra("ALARM_TIME", 0);
        String oldLabel = getIntent().getStringExtra("ALARM_LABEL");

        // Set old label
        alarmLabel.setText(oldLabel);

        // Convert old time
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(oldAlarmTime);
        timePicker.setHour(calendar.get(Calendar.HOUR_OF_DAY));
        timePicker.setMinute(calendar.get(Calendar.MINUTE));

        updateAlarmButton.setOnClickListener(v -> {
            int newHour = timePicker.getHour();
            int newMinute = timePicker.getMinute();
            String newLabel = alarmLabel.getText().toString().trim();

            Calendar newCalendar = Calendar.getInstance();
            newCalendar.set(Calendar.HOUR_OF_DAY, newHour);
            newCalendar.set(Calendar.MINUTE, newMinute);
            newCalendar.set(Calendar.SECOND, 0);

            long newAlarmTime = newCalendar.getTimeInMillis();

            // Update alarm in database
            boolean updated = dbHelper.updateAlarm(alarmId, newAlarmTime, newLabel);
            if (updated) {
                cancelOldAlarm(alarmId);
                setNewAlarm(alarmId, newAlarmTime);
                Toast.makeText(this, "Alarm updated successfully!", Toast.LENGTH_SHORT).show();
                finish();
            } else {
                Toast.makeText(this, "Error updating alarm!", Toast.LENGTH_SHORT).show();
            }
        });
    }

    private void cancelOldAlarm(int alarmId) {
        AlarmManager alarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        Intent intent = new Intent(this, AlarmReceiver.class);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(this, alarmId, intent, PendingIntent.FLAG_UPDATE_CURRENT);
        if (alarmManager != null) {
            alarmManager.cancel(pendingIntent);
        }
    }

    private void setNewAlarm(int alarmId, long newAlarmTime) {
        AlarmManager alarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        Intent intent = new Intent(this, AlarmReceiver.class);
        intent.putExtra("ALARM_ID", alarmId);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(this, alarmId, intent, PendingIntent.FLAG_UPDATE_CURRENT);

        if (alarmManager != null) {
            alarmManager.setExact(AlarmManager.RTC_WAKEUP, newAlarmTime, pendingIntent);
        }
    }

}
