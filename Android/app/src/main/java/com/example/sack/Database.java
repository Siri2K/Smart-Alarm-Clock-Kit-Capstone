package com.example.sack;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class Database extends SQLiteOpenHelper {

    private static final String DATABASE_NAME = "SACK_LOCAL";
    private static final int DATABASE_VERSION = 1;

    //Table Names
    private static final String Table_Heartbeat = "Heartbeat_sensor";
    private static final String Table_Userid = "user_data";

    //Userid Table Columns
    private static final String COLUMN_User_ID = "user_id";
    private static final String COLUMN_USERNAME = "username";
    private static final String COLUMN_PWD = "password";
    private static final String COLUMN_SECURITY_QUESTION = "security_question";
    private static final String COLUMN_SECURITY_ANSWER = "security_answer";

    //Sensor Data Columns
    private static final String COLUMN_USER_REF_ID = "user_id"; //Foreign
    private static final String COLUMN_Sensor_Data = "sensor_data";
    private static final String COLUMN_TIMESTAMP = "timestamp";

    // User Table Creation
    private static final String CREATE_USERS_TABLE =
            "CREATE TABLE " + Table_Userid + " (" +
                    COLUMN_User_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    COLUMN_USERNAME + "TEXT NOT NULL, " +
                    COLUMN_PWD + " TEXT NOT NULL," +
                    COLUMN_SECURITY_QUESTION + " TEXT, " +
                    COLUMN_SECURITY_ANSWER + " TEXT);";

    //Sensor Data Creation
    private static final String CREATE_SENSOR_TABLE =
            "CREATE TABLE " + Table_Heartbeat + " (" +
                    COLUMN_Sensor_Data + " INTEGER, " +
                    COLUMN_TIMESTAMP + " TIMESTAMP DEFAULT CURRENT_TIMESTAMP, " +
                    "FOREIGN KEY (" + COLUMN_USER_REF_ID + ") REFERENCES " + Table_Userid + "(" + COLUMN_User_ID + "));";

    public Database(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase sqLiteDatabase) {
        sqLiteDatabase.execSQL(CREATE_USERS_TABLE);
        sqLiteDatabase.execSQL(CREATE_SENSOR_TABLE);
    }

    @Override
    public void onUpgrade(SQLiteDatabase sqLiteDatabase, int oldVersion, int newVersion) {
        sqLiteDatabase.execSQL("DROP TABLE IF EXISTS " + Table_Userid);
        sqLiteDatabase.execSQL("DROP TABLE IF EXISTS " + Table_Heartbeat);
        onCreate(sqLiteDatabase);

    }
    // Insert User
    public long insertUser(String username, String password) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(COLUMN_USERNAME, username);
        values.put(COLUMN_PWD, password); // Store hashed password for security
        return db.insert(Table_Userid, null, values);
    }
    // Check User Login Credentials if exist
    public boolean validateUser(String username, String password) {
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT * FROM " + Table_Userid + " WHERE " + COLUMN_USERNAME + "=? AND " + COLUMN_PWD + "=?",
                new String[]{username, password});
        boolean exists = cursor.getCount() > 0;
        cursor.close();
        return exists;
    }
    //Inserting user's related sensor data
    public long insertSensorData(int userId, double sensor_data, String securityQuestion, String securityAnswer) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(COLUMN_USER_REF_ID, userId);
        values.put(COLUMN_Sensor_Data, sensor_data);
        values.put(COLUMN_SECURITY_QUESTION, securityQuestion);
        values.put(COLUMN_SECURITY_ANSWER, securityAnswer);
        return db.insert(Table_Heartbeat, null, values);
    }
    // Get Security Question for a User
    public String getSecurityQuestion(String username) {
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT " + COLUMN_SECURITY_QUESTION + " FROM " + Table_Userid + " WHERE " + COLUMN_USERNAME + "=?", new String[]{username});
        if (cursor.moveToFirst()) {
            String question = cursor.getString(0);
            cursor.close();
            return question;
        }
        cursor.close();
        return null;
    }
    // Verify Security Answer
    public boolean verifySecurityAnswer(String username, String answer) {
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT " + COLUMN_SECURITY_ANSWER + " FROM " + Table_Userid + " WHERE " + COLUMN_USERNAME + "=? AND " + COLUMN_SECURITY_ANSWER + "=?", new String[]{username, answer});
        boolean exists = cursor.getCount() > 0;
        cursor.close();
        return exists;
    }
    // Reset Password
    public boolean resetPassword(String username, String newPassword) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(COLUMN_PWD, newPassword);
        int rowsAffected = db.update(Table_Userid, values, COLUMN_USERNAME + "=?", new String[]{username});
        return rowsAffected > 0;
    }
    // Get Sensor Data for a Specific User
    public Cursor getSensorDataByUserId(int userId) {
        SQLiteDatabase db = this.getReadableDatabase();
        return db.rawQuery("SELECT * FROM " + Table_Heartbeat + " WHERE " + COLUMN_USER_REF_ID + "=?", new String[]{String.valueOf(userId)});
    }

}
