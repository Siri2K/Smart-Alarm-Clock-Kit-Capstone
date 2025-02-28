package com.example.sack;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.view.View;
import android.widget.Toast;
import com.google.android.material.bottomnavigation.BottomNavigationView;
import androidx.annotation.NonNull;
import android.view.MenuItem;
import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class  MainActivity extends AppCompatActivity {
protected EditText Username;
protected EditText Password;
protected TextView ForgotPwd;
protected TextView SignUp;
protected Button Login;
private DatabaseHelper dbHelper;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        setupUI();
        dbHelper = new DatabaseHelper(this);
    }
    private void setupUI() {
        Login = findViewById(R.id.loginButton);
        ForgotPwd = findViewById(R.id.forgotPassword);
        SignUp = findViewById(R.id.signupText);
        Username = findViewById(R.id.usernameInput);
        Password = findViewById(R.id.passwordInput);



        // Login Button Click Listener
        Login.setOnClickListener(v -> {
            String username = Username.getText().toString().trim();
            String password = Password.getText().toString().trim();
            if (username.isEmpty() || password.isEmpty()) {
                Toast.makeText(this, "Please enter both username and password", Toast.LENGTH_SHORT).show();
            } else {
                if (dbHelper.validateUser(username, password)) {
                    Toast.makeText(this, "Login Successful!", Toast.LENGTH_SHORT).show();
                    SharedPreferences sharedPreferences = getSharedPreferences("MyPreferences", MODE_PRIVATE);
                    SharedPreferences.Editor editor = sharedPreferences.edit();
                    editor.putString("USERNAME", username); // Save username
                    editor.apply();
                    Intent intent = new Intent(MainActivity.this, HomePage.class);
                    startActivity(intent);
                } else {
                    Toast.makeText(this, "Invalid Credentials!", Toast.LENGTH_SHORT).show();
                }
            }
        });

        // Sign-Up Text Click Listener
        SignUp.setOnClickListener(v -> {
            Intent intent = new Intent(MainActivity.this, SignUpPage.class);
            startActivity(intent);
        });

        ForgotPwd.setOnClickListener(v ->{
                Intent intent = new Intent(MainActivity.this, ResetPwd.class);
                startActivity(intent);
        });
    }
}