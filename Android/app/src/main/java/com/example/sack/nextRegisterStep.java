package com.example.sack;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class nextRegisterStep extends AppCompatActivity {
    protected EditText passwordInput;
    protected EditText securityQuestion;
    protected EditText securityAnswer;
    protected Button RegisterButton;
    private DatabaseHelper dbHelper;
    private String name, username, age, gender; // Data from the first activity
    @Override
    protected void onCreate( Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.nextregisterstep);
        // Initialize DatabaseHelper
        dbHelper = new DatabaseHelper(this);

        // Retrieve data from the first activity
        name = getIntent().getStringExtra("full_name");
        username = getIntent().getStringExtra("username");
        age = getIntent().getStringExtra("age");
        gender = getIntent().getStringExtra("gender");
        setupUI();
    }
    private void setupUI() {
        passwordInput = findViewById(R.id.passwordInput);
        securityQuestion = findViewById(R.id.securityQuestion);
        securityAnswer = findViewById(R.id.securityAnswer);
        RegisterButton = findViewById(R.id.RegisterButton);

        // Handle Register Button Click
        RegisterButton.setOnClickListener(v -> saveUserToUserDatabase());
    }

    private void saveUserToUserDatabase() {
        // Retrieve input values
        String password = passwordInput.getText().toString().trim();
        String Question = securityQuestion.getText().toString().trim();
        String Answer = securityAnswer.getText().toString().trim();

        // Validate inputs
        if (password.isEmpty() || Question.isEmpty() || Answer.isEmpty()) {
            Toast.makeText(this, "Please fill out all fields", Toast.LENGTH_SHORT).show();
            return;
        }

        // Save to database
        long result = dbHelper.insertUser(name, username, password, age, gender, Question, Answer);
        if (result != -1) {
            Toast.makeText(this, "User registered successfully!", Toast.LENGTH_SHORT).show();
            // Navigate to SignInPage
            Intent intent = new Intent(nextRegisterStep.this, MainActivity.class);
            startActivity(intent);
            finish(); // Go back to the previous activity or close
        } else {
            Toast.makeText(this, "Failed to register user", Toast.LENGTH_SHORT).show();
        }
    }
}
