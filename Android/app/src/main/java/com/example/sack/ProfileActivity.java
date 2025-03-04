package com.example.sack;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.InputType;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import com.google.android.material.bottomnavigation.BottomNavigationView;

public class ProfileActivity extends AppCompatActivity {
    private EditText etFullName, etUsername, etAge, etPassword, etSecurityQuestion, etSecurityAnswer;
    private Button btnEditProfile, btnSaveChanges, btnCancelEdit, btnLogout;
    private CheckBox cbShowPassword;
    private DatabaseHelper dbHelper;
    private int userId;
    private String securityQuestion, securityAnswer;
    private String originalFullName, originalUsername, originalAge, originalPassword, originalSecurityQuestion, originalSecurityAnswer; // Store original values

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.profileactivity);
        BottomNavigationView bottomNavigationView = findViewById(R.id.bottom_navigation);
        NavigationBar.setupNavigation(this, bottomNavigationView);

        dbHelper = new DatabaseHelper(this);

        // Retrieve logged-in user
        SharedPreferences sharedPreferences = getSharedPreferences("MyPreferences", MODE_PRIVATE);
        String username = sharedPreferences.getString("USERNAME", "default_username"); // Default if not found
        if (username == null || username.isEmpty()) {
            Toast.makeText(this, "Username is missing", Toast.LENGTH_SHORT).show();
            return;
        }
        userId = dbHelper.getUserIdByUsername(username);

        // Initialize UI elements
        etFullName = findViewById(R.id.et_full_name);
        etUsername = findViewById(R.id.et_username);
        etAge = findViewById(R.id.et_age);
        etPassword = findViewById(R.id.et_password);
        etSecurityQuestion = findViewById(R.id.et_security_question);
        etSecurityAnswer = findViewById(R.id.et_security_answer);
        btnEditProfile = findViewById(R.id.btn_edit_profile);
        btnSaveChanges = findViewById(R.id.btn_save_changes);
        btnCancelEdit = findViewById(R.id.btn_cancel_edit);
        btnLogout = findViewById(R.id.btn_logout);
        cbShowPassword = findViewById(R.id.cb_show_password);

        // Load user details
        loadUserProfile(username);

        // Disable editing initially
        setEditingEnabled(false);

        // Show/Hide password functionality
        cbShowPassword.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) {
                etPassword.setInputType(InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
            } else {
                etPassword.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
            }
            etPassword.setSelection(etPassword.getText().length()); // Keep cursor at end
        });

        // Edit profile button enables editing
        btnEditProfile.setOnClickListener(v -> enableEditing());

        // Save changes after security question verification
        btnSaveChanges.setOnClickListener(v -> promptSecurityQuestion());

        // Cancel button resets fields and disables editing
        btnCancelEdit.setOnClickListener(v -> cancelEditing());

        // Logout button
        btnLogout.setOnClickListener(v -> showLogoutDialog());
    }

    private void loadUserProfile(String username) {
        UserModel user = dbHelper.getUserProfile(username);
        if (user != null) {
            originalFullName = user.getFullName();
            originalUsername = user.getUsername();
            originalAge = String.valueOf(user.getAge());
            originalPassword = user.getPassword();
            originalSecurityQuestion = user.getSecurityQuestion();
            originalSecurityAnswer = user.getSecurityAnswer();

            etFullName.setText(originalFullName);
            etUsername.setText(originalUsername);
            etAge.setText(originalAge);
            etPassword.setText(originalPassword);
            etSecurityQuestion.setText(originalSecurityQuestion);
            etSecurityAnswer.setText(originalSecurityAnswer);
        }
    }

    private void enableEditing() {
        etFullName.setEnabled(true);
        etUsername.setEnabled(true);
        etAge.setEnabled(true);
        etPassword.setEnabled(true);
        etSecurityQuestion.setEnabled(true);
        etSecurityAnswer.setEnabled(true);

        btnEditProfile.setVisibility(View.GONE);
        btnSaveChanges.setVisibility(View.VISIBLE);
        btnCancelEdit.setVisibility(View.VISIBLE);
    }

    private void setEditingEnabled(boolean enabled) {
        etFullName.setEnabled(enabled);
        etUsername.setEnabled(enabled);
        etAge.setEnabled(enabled);
        etPassword.setEnabled(enabled);
        etSecurityQuestion.setEnabled(enabled);
        etSecurityAnswer.setEnabled(enabled);

        if (!enabled) {
            btnSaveChanges.setVisibility(View.GONE);
            btnCancelEdit.setVisibility(View.GONE);
            btnEditProfile.setVisibility(View.VISIBLE);
        }
    }

    private void cancelEditing() {
        etFullName.setText(originalFullName);
        etUsername.setText(originalUsername);
        etAge.setText(originalAge);
        etPassword.setText(originalPassword);
        etSecurityQuestion.setText(originalSecurityQuestion);
        etSecurityAnswer.setText(originalSecurityAnswer);

        setEditingEnabled(false);
    }

    private void promptSecurityQuestion() {
        // Retrieve the correct security question for the logged-in user
        securityQuestion = dbHelper.getSecurityQuestion(originalUsername); // Fetch from database
        securityAnswer = dbHelper.getSecurityAnswer(originalUsername); // Fetch correct answer

        if (securityQuestion == null || securityAnswer == null) {
            Toast.makeText(this, "Error: Security question not found!", Toast.LENGTH_SHORT).show();
            return;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Security Verification");

        // Display the user's actual security question
        builder.setMessage(securityQuestion);

        // Input field for the security answer
        final EditText input = new EditText(this);
        input.setHint("Enter your security answer");
        input.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD); // Hide input
        builder.setView(input);

        builder.setPositiveButton("Verify", (dialog, which) -> {
            String answer = input.getText().toString().trim();

            if (answer.equalsIgnoreCase(securityAnswer)) {
                updateProfile(); // ✅ If correct, allow the user to update profile
            } else {
                Toast.makeText(this, "Incorrect security answer!", Toast.LENGTH_SHORT).show();
            }
        });

        builder.setNegativeButton("Cancel", (dialog, which) -> dialog.dismiss());

        // Show the dialog
        builder.show();
    }



    private void updateProfile() {
        String fullName = etFullName.getText().toString().trim();
        String username = etUsername.getText().toString().trim();
        String ageText = etAge.getText().toString().trim();
        String password = etPassword.getText().toString().trim();
        String newSecurityQuestion = etSecurityQuestion.getText().toString().trim();
        String newSecurityAnswer = etSecurityAnswer.getText().toString().trim();

        if (!ageText.matches("\\d+")) {
            etAge.setError("Age must be a valid number");
            return;
        }

        int age = Integer.parseInt(ageText);
        if (age <= 0 || age > 100) {
            etAge.setError("Please enter a valid age");
            return;
        }

        if (newSecurityQuestion.isEmpty() || newSecurityAnswer.isEmpty()) {
            Toast.makeText(this, "Security question and answer cannot be empty!", Toast.LENGTH_SHORT).show();
            return;
        }

        if (dbHelper.updateUserProfile(userId, fullName, username, age, password, newSecurityQuestion, newSecurityAnswer)) {
            Toast.makeText(this, "Profile updated successfully!", Toast.LENGTH_SHORT).show();

            originalFullName = fullName;
            originalUsername = username;
            originalAge = ageText;
            originalPassword = password;
            originalSecurityQuestion = newSecurityQuestion;
            originalSecurityAnswer = newSecurityAnswer;

            setEditingEnabled(false);
        } else {
            Toast.makeText(this, "Failed to update profile!", Toast.LENGTH_SHORT).show();
        }
    }

    private void showLogoutDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Log Out")
                .setMessage("Are you sure you want to log out?")
                .setPositiveButton("Yes", (dialog, which) -> logoutUser())
                .setNegativeButton("No", (dialog, which) -> dialog.dismiss())
                .show();
    }

    private void logoutUser() {
        SharedPreferences sharedPreferences = getSharedPreferences("MyPreferences", MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.clear();
        editor.apply();

        Intent intent = new Intent(ProfileActivity.this, MainActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        startActivity(intent);
        finish();
    }
}
