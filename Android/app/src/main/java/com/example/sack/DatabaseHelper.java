package com.example.sack;

import android.annotation.SuppressLint;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

public class DatabaseHelper extends SQLiteOpenHelper {


    private static final String DATABASE_NAME = "Alarm_DB";
    private static final int DATABASE_VERSION = 6;

    // Table Names
    private static final String TABLE_HEARTBEAT = "Heartbeat_sensor";
    private static final String TABLE_USERS = "user_data";
    private static final String TABLE_ALARMS = "alarms";
    private static final String TABLE_WIFI = "wifi_settings";
    private static final String TABLE_BULB = "bulb_settings";
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
    private static final String COLUMN_ALARM_SOUND = "alarm_sound";
    private static final String COLUMN_ALARM_LABEL = "alarm_label";
    private static final String COLUMN_ALARM_STATUS = "alarm_status"; // 0 = Disabled, 1 = Enabled
    private static final String COLUMN_REPEAT_DAYS = "repeat_days";
    private static final String COLUMN_VIBRATE = "vibrate";

    // WiFi Table Columns
    private static final String COLUMN_WIFI_ID = "wifi_id";
    private static final String COLUMN_SSID = "ssid";
    private static final String COLUMN_WIFI_PASSWORD = "password";

    // Bulb Table Columns
    private static final String COLUMN_BULB_ID = "bulb_id";
    private static final String COLUMN_MAC_ADDRESS = "mac_address";
    private static final String COLUMN_SKU = "sku";
    private static final String COLUMN_API_KEY = "api_key";

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
                    COLUMN_REPEAT_DAYS + " TEXT DEFAULT '', " +
                    COLUMN_ALARM_SOUND + " TEXT DEFAULT 'Default', " +
                    COLUMN_VIBRATE + " INTEGER DEFAULT 0, " +
                    "FOREIGN KEY (" + COLUMN_USER_REF_ID + ") REFERENCES " + TABLE_USERS + "(" + COLUMN_USER_ID + "));";

    // Create WiFi Settings Table
    private static final String CREATE_WIFI_TABLE =
            "CREATE TABLE " + TABLE_WIFI + " (" +
                    COLUMN_WIFI_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    COLUMN_SSID + " TEXT NOT NULL, " +
                    COLUMN_WIFI_PASSWORD + " TEXT NOT NULL);";

    // Create Bulb Settings Table
    private static final String CREATE_BULB_TABLE =
            "CREATE TABLE " + TABLE_BULB + " (" +
                    COLUMN_BULB_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    COLUMN_MAC_ADDRESS + " TEXT NOT NULL, " +
                    COLUMN_SKU + " TEXT NOT NULL, " +
                    COLUMN_API_KEY + " TEXT NOT NULL);";




    public DatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL(CREATE_USERS_TABLE);
        db.execSQL(CREATE_HEARTBEAT_TABLE);
        db.execSQL(CREATE_ALARM_TABLE);
        db.execSQL(CREATE_WIFI_TABLE);
        db.execSQL(CREATE_BULB_TABLE);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        if (oldVersion < 2) {
            db.execSQL("ALTER TABLE " + TABLE_USERS + " ADD COLUMN " + COLUMN_FULL_NAME + " TEXT NOT NULL DEFAULT ''");
            db.execSQL("ALTER TABLE " + TABLE_ALARMS + " ADD COLUMN " + COLUMN_REPEAT_DAYS + " TEXT DEFAULT ''");
            db.execSQL("ALTER TABLE " + TABLE_ALARMS + " ADD COLUMN " + COLUMN_ALARM_SOUND + " TEXT DEFAULT 'Default'"); // ✅ Fix: Space added before TEXT
            db.execSQL("ALTER TABLE " + TABLE_ALARMS + " ADD COLUMN " + COLUMN_VIBRATE + " INTEGER DEFAULT 0"); // ✅ Fix: Space added before INTEGER
            db.execSQL("ALTER TABLE " + TABLE_ALARMS + " ADD COLUMN " + COLUMN_ALARM_SOUND + " TEXT DEFAULT 'Default'");
            db.execSQL("ALTER TABLE " + TABLE_ALARMS + " ADD COLUMN " + COLUMN_VIBRATE + " INTEGER DEFAULT 0");
        }
        if (oldVersion < 8) { //Ensure new columns are always created
            db.execSQL("ALTER TABLE " + TABLE_ALARMS + " ADD COLUMN " + COLUMN_ALARM_SOUND + "TEXT DEFAULT 'Default'");
            db.execSQL("ALTER TABLE " + TABLE_ALARMS + " ADD COLUMN " + COLUMN_VIBRATE + "INTEGER DEFAULT 0");
            db.execSQL("DROP TABLE IF EXISTS " + TABLE_WIFI);
            db.execSQL("DROP TABLE IF EXISTS " + TABLE_BULB);
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
    public long insertSensorData(int userId, int bpm, String timestamp) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put("user_id", userId);
        values.put("sensor_data", bpm);
        values.put("timestamp", timestamp);

        long result = db.insert("Heartbeat_sensor", null, values);

        if (result != -1) {
            Log.d("DatabaseHelper", "BPM inserted successfully: " + bpm + " at " + timestamp);
        } else {
            Log.e("DatabaseHelper", "Failed to insert BPM.");
        }

        return result;
    }




    // Get Sensor Data for a User
    public Cursor getSensorDataByUserId(int userId) {
        SQLiteDatabase db = this.getReadableDatabase();
        return db.rawQuery("SELECT * FROM " + TABLE_HEARTBEAT + " WHERE " + COLUMN_USER_REF_ID + "=?", new String[]{String.valueOf(userId)});
    }

    // Insert Alarm with Custom Sound & Vibration
    public long insertAlarm(int userId, String time, String label, String repeatDays, String alarmSound, boolean vibrate) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(COLUMN_USER_REF_ID, userId);
        values.put(COLUMN_ALARM_TIME, time);
        values.put(COLUMN_ALARM_LABEL, label);
        values.put(COLUMN_REPEAT_DAYS, repeatDays);
        values.put(COLUMN_ALARM_SOUND, alarmSound); //Store selected alarm sound
        values.put(COLUMN_VIBRATE, vibrate ? 1 : 0); //Store vibration setting
        values.put(COLUMN_ALARM_STATUS, 1);

        long result = db.insert(TABLE_ALARMS, null, values);

        if (result > 0) {
            System.out.println("DEBUG: Alarm inserted successfully - ID: " + result +
                    ", Time: " + time + ", Sound: " + alarmSound + ", Vibrate: " + vibrate);
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

    public UserModel getUserProfile(String username) {
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT * FROM user_data WHERE username=?", new String[]{username});
        if (cursor.moveToFirst()) {
            @SuppressLint("Range") UserModel user = new UserModel(
                    cursor.getString(cursor.getColumnIndex("full_name")),
                    cursor.getString(cursor.getColumnIndex("username")),
                    cursor.getInt(cursor.getColumnIndex("age")),
                    cursor.getString(cursor.getColumnIndex("password")),
                    cursor.getString(cursor.getColumnIndex("security_question")),
                    cursor.getString(cursor.getColumnIndex("security_answer"))
            );
            cursor.close();
            return user;
        }
        cursor.close();
        return null;
    }


    public boolean updateUserProfile(int userId, String fullName, String username, int age, String password, String securityQuestion, String securityAnswer) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put("full_name", fullName);
        values.put("username", username);
        values.put("age", age);
        values.put("password", password);
        values.put("security_question", securityQuestion);
        values.put("security_answer", securityAnswer);

        return db.update("user_data", values, "user_id=?", new String[]{String.valueOf(userId)}) > 0;
    }

    public String getAlarmData(int userId) {
        SQLiteDatabase db = this.getReadableDatabase();
        List<String> alarmTimes = new ArrayList<>();

        // Fetch all alarm times for the user
        Cursor cursor = db.rawQuery("SELECT " + COLUMN_ALARM_TIME +
                        " FROM " + TABLE_ALARMS +
                        " WHERE " + COLUMN_USER_REF_ID +
                        " ORDER BY " + COLUMN_ALARM_TIME + " ASC",
                new String[]{String.valueOf(userId)});

        while (cursor.moveToNext()) {
            @SuppressLint("Range") String alarmTime = cursor.getString(cursor.getColumnIndex(COLUMN_ALARM_TIME));
            alarmTimes.add(alarmTime);
        }
        cursor.close();

        int alarmCount = alarmTimes.size();

        // If no alarms exist, send a default response
        if (alarmCount == 0) {
            return "00:00,0"; // No alarms set
        }

        // Convert the list of alarms to a single string (comma-separated)
        return String.join(",", alarmTimes) + "," + alarmCount;
    }
    // Insert or Update WiFi credentials
    public void saveOrUpdateWiFiSettings(String ssid, String password) {
        SQLiteDatabase db = this.getWritableDatabase();

        // Check if WiFi settings exist (Only one WiFi entry allowed)
        Cursor cursor = db.rawQuery("SELECT * FROM " + TABLE_WIFI, null);
        boolean exists = cursor.moveToFirst();
        cursor.close();

        ContentValues values = new ContentValues();
        values.put(COLUMN_SSID, ssid);
        values.put(COLUMN_WIFI_PASSWORD, password);

        if (exists) {
            // Update existing entry
            db.update(TABLE_WIFI, values, null, null);
        } else {
            // Insert new entry
            db.insert(TABLE_WIFI, null, values);
        }
        db.close();
    }

    // Retrieve saved WiFi settings
    public String[] getWiFiSettings() {
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT * FROM " + TABLE_WIFI, null);
        String[] wifiData = null;

        if (cursor.moveToFirst()) {
            @SuppressLint("Range") String ssid = cursor.getString(cursor.getColumnIndex(COLUMN_SSID));
            @SuppressLint("Range") String password = cursor.getString(cursor.getColumnIndex(COLUMN_WIFI_PASSWORD));
            wifiData = new String[]{ssid, password};
        }
        cursor.close();
        return wifiData;
    }



    // Insert or Update Bulb Information
    public void saveOrUpdateBulbSettings(String macAddress, String sku, String apiKey) {
        SQLiteDatabase db = this.getWritableDatabase();

        // Check if Bulb settings exist (Only one bulb entry allowed)
        Cursor cursor = db.rawQuery("SELECT * FROM " + TABLE_BULB, null);
        boolean exists = cursor.moveToFirst();
        cursor.close();

        ContentValues values = new ContentValues();
        values.put(COLUMN_MAC_ADDRESS, macAddress);
        values.put(COLUMN_SKU, sku);
        values.put(COLUMN_API_KEY, apiKey);

        if (exists) {
            // Update existing entry
            db.update(TABLE_BULB, values, null, null);
        } else {
            // Insert new entry
            db.insert(TABLE_BULB, null, values);
        }
        db.close();
    }

    // Retrieve saved Bulb settings
    public String[] getBulbSettings() {
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT * FROM " + TABLE_BULB, null);
        String[] bulbData = null;

        if (cursor.moveToFirst()) {
            @SuppressLint("Range") String macAddress = cursor.getString(cursor.getColumnIndex(COLUMN_MAC_ADDRESS));
            @SuppressLint("Range") String sku = cursor.getString(cursor.getColumnIndex(COLUMN_SKU));
            @SuppressLint("Range") String apiKey = cursor.getString(cursor.getColumnIndex(COLUMN_API_KEY));
            bulbData = new String[]{macAddress, sku, apiKey};
        }
        cursor.close();
        return bulbData;
    }
    public void printHeartbeatTable() {
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT * FROM Heartbeat_sensor", null);

        if (cursor != null && cursor.moveToFirst()) {
            do {
                @SuppressLint("Range") int userId = cursor.getInt(cursor.getColumnIndex("user_id"));
                @SuppressLint("Range") int bpm = cursor.getInt(cursor.getColumnIndex("sensor_data"));
                @SuppressLint("Range") String timestamp = cursor.getString(cursor.getColumnIndex("timestamp"));

                Log.d("DatabaseDebug", "UserID: " + userId + " | BPM: " + bpm + " | Timestamp: " + timestamp);
            } while (cursor.moveToNext());
            cursor.close();
        } else {
            Log.d("DatabaseDebug", "No data found in Heartbeat_sensor table.");
        }
    }

}