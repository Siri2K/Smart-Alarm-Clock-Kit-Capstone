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
    private EditText etFullName, etUsername, etAge, etPassword;
    private Button btnEditProfile, btnSaveChanges, btnCancelEdit;
    private CheckBox cbShowPassword;
    private DatabaseHelper dbHelper;
    private int userId;
    private String securityQuestion, securityAnswer;
    private String originalFullName, originalUsername, originalAge, originalPassword; // Store original values

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
            return; // Or handle the error accordingly
        }
        userId = dbHelper.getUserIdByUsername(username);

        // Initialize UI elements
        etFullName = findViewById(R.id.et_full_name);
        etUsername = findViewById(R.id.et_username);
        etAge = findViewById(R.id.et_age);
        etPassword = findViewById(R.id.et_password);
        btnEditProfile = findViewById(R.id.btn_edit_profile);
        btnSaveChanges = findViewById(R.id.btn_save_changes);
        btnCancelEdit = findViewById(R.id.btn_cancel_edit);
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
    }

    private void loadUserProfile(String username) {
        UserModel user = dbHelper.getUserProfile(username);
        if (user != null) {
            originalFullName = user.getFullName();
            originalUsername = user.getUsername();
            originalAge = String.valueOf(user.getAge());
            originalPassword = user.getPassword();

            etFullName.setText(originalFullName);
            etUsername.setText(originalUsername);
            etAge.setText(originalAge);
            etPassword.setText(originalPassword); // Initially hidden
            securityQuestion = user.getSecurityQuestion();
            securityAnswer = user.getSecurityAnswer();
        }
    }

    private void enableEditing() {
        etFullName.setEnabled(true);
        etUsername.setEnabled(true);
        etAge.setEnabled(true);
        etPassword.setEnabled(true);

        btnEditProfile.setVisibility(View.GONE);
        btnSaveChanges.setVisibility(View.VISIBLE);
        btnCancelEdit.setVisibility(View.VISIBLE);
    }

    private void setEditingEnabled(boolean enabled) {
        etFullName.setEnabled(enabled);
        etUsername.setEnabled(enabled);
        etAge.setEnabled(enabled);
        etPassword.setEnabled(enabled);

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

        setEditingEnabled(false);
    }

    private void promptSecurityQuestion() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Security Verification");

        // Input field for the security answer
        final EditText input = new EditText(this);
        input.setHint(securityQuestion);
        builder.setView(input);

        builder.setPositiveButton("Verify", (dialog, which) -> {
            String answer = input.getText().toString().trim();
            if (answer.equalsIgnoreCase(securityAnswer)) {
                updateProfile();
            } else {
                Toast.makeText(this, "Incorrect security answer!", Toast.LENGTH_SHORT).show();
            }
        });

        builder.setNegativeButton("Cancel", (dialog, which) -> dialog.cancel());
        builder.show();
    }

    private void updateProfile() {
        String fullName = etFullName.getText().toString().trim();
        String username = etUsername.getText().toString().trim();
        String ageText = etAge.getText().toString().trim();
        String password = etPassword.getText().toString().trim();

        // Validate Age Input - Ensure it's only numbers
        if (!ageText.matches("\\d+")) {
            etAge.setError("Age must be a valid number");
            return;
        }

        int age = Integer.parseInt(ageText);
        if (age <= 0 || age > 100) {
            etAge.setError("Please enter a valid age");
            return;
        }

        // If validation passes, proceed with saving
        if (dbHelper.updateUserProfile(userId, fullName, username, age, password)) {
            Toast.makeText(this, "Profile updated successfully!", Toast.LENGTH_SHORT).show();

            // Update stored values after saving
            originalFullName = fullName;
            originalUsername = username;
            originalAge = ageText;
            originalPassword = password;

            setEditingEnabled(false);
        } else {
            Toast.makeText(this, "Failed to update profile!", Toast.LENGTH_SHORT).show();
        }
    }
}
