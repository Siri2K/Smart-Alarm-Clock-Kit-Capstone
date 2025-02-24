package com.example.sack;

import android.annotation.SuppressLint;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.List;

public class AlarmAdapter extends RecyclerView.Adapter<AlarmAdapter.AlarmViewHolder> {
    private Context context;
    private ArrayList<AlarmModel> alarmList;
    private DatabaseHelper databaseHelper;

    public AlarmAdapter(Context context, ArrayList<AlarmModel> alarmList) {
        this.context = context;
        this.alarmList = alarmList;
        this.databaseHelper = new DatabaseHelper(context);
    }

    @NonNull
    @Override
    public AlarmViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(context).inflate(R.layout.alarm_item, parent, false);
        return new AlarmViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull AlarmViewHolder holder, int position) {
        AlarmModel alarm = alarmList.get(position);

        // Set the alarm time
        holder.tvAlarmTime.setText(alarm.getTime());

        // Set repeat days
        holder.tvRepeatDays.setText(alarm.getRepeatDays());

        // Set switch state
        holder.switchAlarm.setChecked(alarm.isEnabled());

        // Handle alarm toggle
        holder.switchAlarm.setOnCheckedChangeListener((buttonView, isChecked) -> {
            alarm.setEnabled(isChecked);
            databaseHelper.updateAlarmStatus(alarm.getId(), isChecked);
        });
    }

    @Override
    public int getItemCount() {
        return alarmList.size();
    }

    public static class AlarmViewHolder extends RecyclerView.ViewHolder {
        TextView tvAlarmTime, tvRepeatDays;
        Switch switchAlarm;

        public AlarmViewHolder(@NonNull View itemView) {
            super(itemView);
            tvAlarmTime = itemView.findViewById(R.id.tv_alarm_time);
            tvRepeatDays = itemView.findViewById(R.id.tv_repeat_days);
            switchAlarm = itemView.findViewById(R.id.switch_alarm);
        }
    }

    @SuppressLint("NotifyDataSetChanged")
    public void updateAlarms(List<AlarmModel> newAlarms) {
        this.alarmList = new ArrayList<>(newAlarms); // Replace with a new list
        notifyDataSetChanged(); // Force update
    }

    public void removeAlarm(int position) {
        if (position >= 0 && position < alarmList.size()) {
            AlarmModel alarmToDelete = alarmList.get(position);
            databaseHelper.deleteAlarm(alarmToDelete.getId()); // Remove from database
            alarmList.remove(position); // Remove from list
            notifyItemRemoved(position); // Notify adapter
        }
    }

}
