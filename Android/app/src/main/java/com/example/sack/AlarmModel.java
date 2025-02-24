package com.example.sack;

public class AlarmModel {
    private int id;
    private int userId;
    private String time;
    private String label;
    private boolean isEnabled;
    private String repeatDays; // New field for repeat days (e.g., "S M T W T F S")

    // Updated Constructor
    public AlarmModel(int id, int userId, String time, String label, boolean isEnabled, String repeatDays) {
        this.id = id;
        this.userId = userId;
        this.time = time;
        this.label = label;
        this.isEnabled = isEnabled;
        this.repeatDays = repeatDays; // Store repeat days
    }

    // Getters
    public int getId() {
        return id;
    }

    public int getUserId() {
        return userId;
    }

    public String getTime() {
        return time;
    }

    public String getLabel() {
        return label;
    }

    public boolean isEnabled() {
        return isEnabled;
    }

    public String getRepeatDays() {
        return repeatDays;
    } // New Getter

    // Setters
    public void setId(int id) {
        this.id = id;
    }

    public void setUserId(int userId) {
        this.userId = userId;
    }

    public void setTime(String time) {
        this.time = time;
    }

    public void setLabel(String label) {
        this.label = label;
    }

    public void setEnabled(boolean enabled) {
        isEnabled = enabled;
    }

    public void setRepeatDays(String repeatDays) {
        this.repeatDays = repeatDays;
    } // New Setter
}
