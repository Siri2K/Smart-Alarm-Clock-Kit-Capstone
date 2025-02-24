package com.example.sack;

import android.annotation.SuppressLint;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import java.util.ArrayList;

public class DatabaseHelper extends SQLiteOpenHelper {

    private static final String DATABASE_NAME = "Alarm_DB";
    private static final int DATABASE_VERSION = 2;

    // Table Names
    private static final String TABLE_HEARTBEAT = "Heartbeat_sensor";
    private static final String TABLE_USERS = "user_data";
    private static final String TABLE_ALARMS = "alarms";

    // Users Table Columns
    private static final String COLUMN_USER_ID = "user_id";
    private static final String COLUMN_FULL_NAME = "full_name";
    private static final String COLUMN_USERNAME = "username";
    private static final String COLUMN_PASSWORD = "password";
    private static final String COLUMN_SECURITY_QUESTION = "security_question";
    private static final String COLUMN_SECURITY_ANSWER = "security_answer";
    private static final String COLUMN_AGE = "age";
    private static final String COLUMN_Gender = "gender";
    private static final String COLUMN_CONDITION = "condition";
    // Heartbeat Table Columns
    private static final String COLUMN_SENSOR_ID = "sensor_id";
    private static final String COLUMN_USER_REF_ID = "user_id"; // Foreign Key
    private static final String COLUMN_SENSOR_DATA = "sensor_data";
    private static final String COLUMN_TIMESTAMP = "timestamp";

    // Alarms Table Columns
    private static final String COLUMN_ALARM_ID = "alarm_id";
    private static final String COLUMN_ALARM_TIME = "alarm_time";
    private static final String COLUMN_ALARM_LABEL = "alarm_label";
    private static final String COLUMN_ALARM_STATUS = "alarm_status"; // 0 = Disabled, 1 = Enabled
    private static final String COLUMN_REPEAT_DAYS = "repeat_days";


    // Users Table Creation
    private static final String CREATE_USERS_TABLE =
            "CREATE TABLE " + TABLE_USERS + " (" +
                    COLUMN_USER_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    COLUMN_FULL_NAME + " TEXT NOT NULL, " +  // Added space between column name and type
                    COLUMN_USERNAME + " TEXT NOT NULL, " +
                    COLUMN_PASSWORD + " TEXT NOT NULL, " +
                    COLUMN_SECURITY_QUESTION + " TEXT, " +
                    COLUMN_SECURITY_ANSWER + " TEXT, " +
                    COLUMN_AGE + " INTEGER, " +  // Added space between column name and type
                    COLUMN_Gender + " TEXT," + COLUMN_CONDITION + " TEXT);";   // Corrected "Text" to "TEXT"


    // Heartbeat Sensor Table Creation
    private static final String CREATE_HEARTBEAT_TABLE =
            "CREATE TABLE " + TABLE_HEARTBEAT + " (" +
                    COLUMN_SENSOR_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    COLUMN_USER_REF_ID + " INTEGER NOT NULL, " +
                    COLUMN_SENSOR_DATA + " REAL, " +
                    COLUMN_TIMESTAMP + " TIMESTAMP DEFAULT CURRENT_TIMESTAMP, " +
                    "FOREIGN KEY (" + COLUMN_USER_REF_ID + ") REFERENCES " + TABLE_USERS + "(" + COLUMN_USER_ID + "));";

    // Alarms Table Creation
    private static final String CREATE_ALARM_TABLE =
            "CREATE TABLE " + TABLE_ALARMS + " (" +
                    COLUMN_ALARM_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    COLUMN_USER_REF_ID + " INTEGER NOT NULL, " +
                    COLUMN_ALARM_TIME + " TEXT NOT NULL, " +
                    COLUMN_ALARM_LABEL + " TEXT, " +
                    COLUMN_ALARM_STATUS + " INTEGER DEFAULT 1, " +
                    "repeat_days TEXT, " +
                    "FOREIGN KEY (" + COLUMN_USER_REF_ID + ") REFERENCES " + TABLE_USERS + "(" + COLUMN_USER_ID + "));";





    public DatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL(CREATE_USERS_TABLE);
        db.execSQL(CREATE_HEARTBEAT_TABLE);
        db.execSQL(CREATE_ALARM_TABLE);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        if (oldVersion < 2) {
            // Add missing "full_name" column
            db.execSQL("ALTER TABLE " + TABLE_USERS + " ADD COLUMN " + COLUMN_FULL_NAME + " TEXT NOT NULL DEFAULT ''");
        }
        if (oldVersion < 3) {
            db.execSQL("ALTER TABLE " + TABLE_USERS + " ADD COLUMN " + COLUMN_AGE + " INTEGER DEFAULT 0");
            db.execSQL("ALTER TABLE " + TABLE_USERS + " ADD COLUMN " + COLUMN_Gender + " TEXT DEFAULT ''");
        }
        if (oldVersion < 4) {
            db.execSQL("ALTER TABLE " + TABLE_USERS + " ADD COLUMN " + COLUMN_CONDITION + " TEXT DEFAULT ''");
        }
        if (oldVersion < 5) {
            db.execSQL("ALTER TABLE " + TABLE_USERS + " ADD COLUMN " + COLUMN_SECURITY_QUESTION + " TEXT DEFAULT ''");
            db.execSQL("ALTER TABLE " + TABLE_USERS + " ADD COLUMN " + COLUMN_SECURITY_ANSWER + " TEXT DEFAULT ''");
        }
        if (oldVersion < 6) {
            // Fix: Add repeat_days column if missing
            db.execSQL("ALTER TABLE " + TABLE_ALARMS + " ADD COLUMN repeat_days TEXT DEFAULT ''");
        }
    }




    // Insert User
    public long insertUser(String fullName, String username, String password, String age, String gender, String securityQuestion, String securityAnswer) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put("full_name", fullName);
        values.put("username", username);
        values.put("password", password);
        values.put("age", age);
        values.put("gender", gender);
        values.put("security_question", securityQuestion);
        values.put("security_answer", securityAnswer);
        return db.insert("user_data", null, values);
    }


    // Validate User Credentials
    public boolean validateUser(String username, String password) {
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT * FROM " + TABLE_USERS + " WHERE " + COLUMN_USERNAME + "=? AND " + COLUMN_PASSWORD + "=?",
                new String[]{username, password});
        boolean exists = cursor.getCount() > 0;
        cursor.close();
        return exists;
    }

    // Insert Sensor Data
    public long insertSensorData(int userId, double sensorData) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(COLUMN_USER_REF_ID, userId);
        values.put(COLUMN_SENSOR_DATA, sensorData);
        return db.insert(TABLE_HEARTBEAT, null, values);
    }

    // Get Sensor Data for a User
    public Cursor getSensorDataByUserId(int userId) {
        SQLiteDatabase db = this.getReadableDatabase();
        return db.rawQuery("SELECT * FROM " + TABLE_HEARTBEAT + " WHERE " + COLUMN_USER_REF_ID + "=?", new String[]{String.valueOf(userId)});
    }

    // Insert Alarm
    public long insertAlarm(int userId, String time, String label, String repeatDays) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(COLUMN_USER_REF_ID, userId);
        values.put(COLUMN_ALARM_TIME, time);
        values.put(COLUMN_ALARM_LABEL, label);
        values.put(COLUMN_REPEAT_DAYS, repeatDays);
        values.put(COLUMN_ALARM_STATUS, 1);

        long result = db.insert(TABLE_ALARMS, null, values);

        if (result > 0) {
            System.out.println("DEBUG: Alarm inserted successfully in database - ID: " + result);
        } else {
            System.out.println("DEBUG: Alarm insertion failed.");
        }

        return result;
    }



    // Get All Alarms
    public ArrayList<AlarmModel> getAllAlarms() {
        ArrayList<AlarmModel> alarmList = new ArrayList<>();
        SQLiteDatabase db = this.getReadableDatabase();

        Cursor cursor = db.rawQuery("SELECT * FROM " + TABLE_ALARMS, null);

        if (cursor.moveToFirst()) {
            do {
                @SuppressLint("Range") int id = cursor.getInt(cursor.getColumnIndex(COLUMN_ALARM_ID));
                @SuppressLint("Range") int userId = cursor.getInt(cursor.getColumnIndex(COLUMN_USER_REF_ID));
                @SuppressLint("Range") String time = cursor.getString(cursor.getColumnIndex(COLUMN_ALARM_TIME));
                @SuppressLint("Range") String label = cursor.getString(cursor.getColumnIndex(COLUMN_ALARM_LABEL));
                @SuppressLint("Range") boolean isEnabled = cursor.getInt(cursor.getColumnIndex(COLUMN_ALARM_STATUS)) == 1;
                @SuppressLint("Range") String repeatDays = cursor.getString(cursor.getColumnIndex(COLUMN_REPEAT_DAYS));

                AlarmModel alarm = new AlarmModel(id, userId, time, label, isEnabled, repeatDays);
                alarmList.add(alarm);

                // Debugging: Log retrieved alarm
                System.out.println("DEBUG: Retrieved Alarm - ID: " + id + ", Time: " + time + ", Repeat Days: " + repeatDays);
            } while (cursor.moveToNext());
        } else {
            System.out.println("DEBUG: No alarms found in database!");
        }

        cursor.close();
        return alarmList;
    }




    // Update Alarm
    public boolean updateAlarm(int alarmId, long newTime, String newLabel) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(COLUMN_ALARM_TIME, newTime);
        values.put(COLUMN_ALARM_LABEL, newLabel);
        int rowsAffected = db.update(TABLE_ALARMS, values, COLUMN_ALARM_ID + "=?", new String[]{String.valueOf(alarmId)});
        return rowsAffected > 0;
    }

    // Enable or Disable Alarm
    public void updateAlarmStatus(int alarmId, boolean isEnabled) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(COLUMN_ALARM_STATUS, isEnabled ? 1 : 0);

        db.update(TABLE_ALARMS, values, COLUMN_ALARM_ID + "=?", new String[]{String.valueOf(alarmId)});
        db.close();
    }


    // Delete Alarm
    public boolean deleteAlarm(int alarmId) {
        SQLiteDatabase db = this.getWritableDatabase();
        int rowsDeleted = db.delete(TABLE_ALARMS, COLUMN_ALARM_ID + "=?", new String[]{String.valueOf(alarmId)});
        return rowsDeleted > 0;
    }

    // Get Security Question
    public String getSecurityQuestion(String username) {
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT " + COLUMN_SECURITY_QUESTION + " FROM " + TABLE_USERS + " WHERE " + COLUMN_USERNAME + "=?",
                new String[]{username});

        if (cursor.moveToFirst()) {
            String question = cursor.getString(0);
            cursor.close();
            return question;
        }
        cursor.close();
        return null; // User not found
    }

    // Get Security Answer for a given username
    public String getSecurityAnswer(String username) {
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT " + COLUMN_SECURITY_ANSWER + " FROM " + TABLE_USERS + " WHERE " + COLUMN_USERNAME + "=?",
                new String[]{username});

        if (cursor.moveToFirst()) {
            String answer = cursor.getString(0);
            cursor.close();
            return answer;
        }
        cursor.close();
        return null; // User not found
    }

    // Verify Security Answer
    public boolean verifySecurityAnswer(String username, String answer) {
        if (username == null || answer == null) {
            return false; // Prevent null values
        }

        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT 1 FROM " + TABLE_USERS + " WHERE " + COLUMN_USERNAME + "=? AND " + COLUMN_SECURITY_ANSWER + "=?",
                new String[]{username, answer});

        boolean exists = cursor.moveToFirst();
        cursor.close();
        return exists;
    }

    // Reset Password
    public boolean resetPassword(String username, String newPassword) {
        if (username == null || newPassword == null || newPassword.isEmpty()) {
            return false; // Prevent null or empty password updates
        }

        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(COLUMN_PASSWORD, newPassword);

        int rowsUpdated = db.update(TABLE_USERS, values, COLUMN_USERNAME + "=?", new String[]{username});
        return rowsUpdated > 0;
    }
    public int getUserIdByUsername(String username) {
        SQLiteDatabase db = this.getReadableDatabase();
        int userId = -1; // Default value if user is not found

        Cursor cursor = db.rawQuery("SELECT user_id FROM user_data WHERE username=?", new String[]{username});
        if (cursor.moveToFirst()) {
            userId = cursor.getInt(0); // Get the user ID
        }
        cursor.close();
        return userId;
    }


}
