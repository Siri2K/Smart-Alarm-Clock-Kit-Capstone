package com.example.sack;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.view.View;
import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class MainActivity extends AppCompatActivity {
protected EditText Email;
protected EditText Password;
protected TextView ForgotPwd;
protected TextView SignUp;
protected Button Login;



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
    }
    private void setupUI() {
        Login = findViewById(R.id.loginButton);
        ForgotPwd = findViewById(R.id.forgotPassword);
        SignUp = findViewById(R.id.signupText);
        Email = findViewById(R.id.emailInput);
        Password = findViewById(R.id.passwordInput);

        Login.setOnClickListener(v-> {

        });
        ForgotPwd.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, SignUpPage.class);
                startActivity(intent);
            }
        });
    }
}