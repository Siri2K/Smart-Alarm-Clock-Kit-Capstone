package com.example.sack;

import android.content.Intent;
import android.os.Bundle;
import android.text.InputType;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

public class ResetPwd extends AppCompatActivity {
    private EditText usernameEdit, securityQuestionInput, answerInput, newPasswordInput;
    private Button resetButtonR;
    private DatabaseHelper dbHelper;
    private String storedAnswer, username;
    protected TextView backToLogin;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.resetpwd);
        setupUI();
        dbHelper = new DatabaseHelper(this);
    }

    private void setupUI() {
        usernameEdit = findViewById(R.id.username);
        securityQuestionInput = findViewById(R.id.securityQuestionInput);
        answerInput = findViewById(R.id.answerInput);
        newPasswordInput = findViewById(R.id.newPasswordInput);
        resetButtonR = findViewById(R.id.resetButtonr);
        backToLogin = findViewById(R.id.backToLogin);

        resetButtonR.setEnabled(false); // Disable reset button initially
        securityQuestionInput.setInputType(InputType.TYPE_NULL); // Make question field non-editable

        // Fetch security question when username loses focus
        usernameEdit.setOnFocusChangeListener((v, hasFocus) -> {
            if (!hasFocus) {
                username = usernameEdit.getText().toString().trim();
                if (!username.isEmpty()) {
                    String question = dbHelper.getSecurityQuestion(username);
                    storedAnswer = dbHelper.getSecurityAnswer(username);

                    if (question != null && storedAnswer != null) { // Ensure no null values
                        securityQuestionInput.setText(question);
                        resetButtonR.setEnabled(true);
                    } else {
                        securityQuestionInput.setText("User not found!");
                        resetButtonR.setEnabled(false);
                    }
                }
            }
        });


        // Reset Password button logic
        resetButtonR.setOnClickListener(v -> {
            username = usernameEdit.getText().toString().trim();
            String answer = answerInput.getText().toString().trim();
            String newPassword = newPasswordInput.getText().toString().trim(); // ✅ Ensure newPassword is retrieved

            if (username.isEmpty() || answer.isEmpty()) {
                Toast.makeText(this, "Username and answer cannot be empty!", Toast.LENGTH_SHORT).show();
                return;
            }

            if (storedAnswer == null) {
                Toast.makeText(this, "User not found!", Toast.LENGTH_SHORT).show();
                return;
            }

            if (!answer.equalsIgnoreCase(storedAnswer)) {
                Toast.makeText(this, "Incorrect answer!", Toast.LENGTH_SHORT).show();
                return;
            }

            if (newPassword.length() < 6) {
                Toast.makeText(this, "Password must be at least 6 characters!", Toast.LENGTH_SHORT).show();
                return;
            }

            // ✅ Call resetPassword() with the correct arguments
            boolean isUpdated = dbHelper.resetPassword(username, newPassword);

            if (isUpdated) {
                Toast.makeText(this, "Password reset successful!", Toast.LENGTH_SHORT).show();
                startActivity(new Intent(ResetPwd.this, MainActivity.class));
                finish();
            } else {
                Toast.makeText(this, "Error resetting password!", Toast.LENGTH_SHORT).show();
            }
        });
        backToLogin.setOnClickListener(v -> {
            Intent intent = new Intent(ResetPwd.this, MainActivity.class);
            startActivity(intent);
        });
}
}