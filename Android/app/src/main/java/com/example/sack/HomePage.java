package com.example.sack;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.view.MenuItem;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.google.android.material.bottomnavigation.BottomNavigationView;

public class HomePage extends AppCompatActivity {
    @SuppressLint("SetTextI18n")
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.homepage);
        TextView welcomeMessage = findViewById(R.id.welcomeMessage);
        BottomNavigationView bottomNavigationView = findViewById(R.id.bottom_navigation);
        bottomNavigationView.setOnItemSelectedListener(this::onNavigationItemSelected);
        String username = getIntent().getStringExtra("USERNAME");
        DatabaseHelper dbHelper = new DatabaseHelper(this);
        if (username != null && !username.isEmpty()) {
            welcomeMessage.setText("Welcome " + username);
        } else {
            welcomeMessage.setText("Welcome User");
        }
    }

    private boolean onNavigationItemSelected(@NonNull MenuItem item) {
        int id = item.getItemId();
        if (id == R.id.nav_home) {
            startActivity(new Intent(HomePage.this, HomePage.class));
            return true;
        } else if (id == R.id.nav_alarm) {
            startActivity(new Intent(HomePage.this, AlarmSetPage.class));
            return true;
        } else if (id == R.id.nav_ConnectToDevice) {
            startActivity(new Intent(HomePage.this, ConnectToDevice.class));
            return true;
        }
        return false;
    }
}
