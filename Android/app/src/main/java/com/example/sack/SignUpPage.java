package com.example.sack;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;

public class SignUpPage extends AppCompatActivity {
    protected EditText nameInput;
    protected EditText usernameInput;
    protected Spinner ageSpinner, genderSpinner, conditionSpinner;
    protected Button NextButton;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.signuppage);
        setupUI();
    }
    private void setupUI() {
        // Spinner Initialization
        nameInput = findViewById(R.id.nameInput);
        usernameInput = findViewById(R.id.usernameInput);
        ageSpinner = findViewById(R.id.ageSpinner);
        genderSpinner = findViewById(R.id.genderSpinner);
        conditionSpinner = findViewById(R.id.conditionSpinner);
        NextButton = findViewById(R.id.NextButton);

        //spinners setup
        setupAgeSpinner();
        setupGenderSpinner();
        setupConditionSpinner();

        NextButton.setOnClickListener(v->{
            // Retrieve input values
            String name = nameInput.getText().toString().trim();
            String username = usernameInput.getText().toString().trim();
            String age = ageSpinner.getSelectedItem().toString();
            String gender = genderSpinner.getSelectedItem().toString();

            // Validate inputs
            if (name.isEmpty() || username.isEmpty() || age.equals("Age") || gender.equals("Gender")) {
                Toast.makeText(this, "Please fill out all fields", Toast.LENGTH_SHORT).show();
                return;
            }
            // Pass data to the next activity
            Intent intent = new Intent(SignUpPage.this, nextRegisterStep.class);
            intent.putExtra("full_name", nameInput.getText().toString().trim());
            intent.putExtra("username", usernameInput.getText().toString().trim());
            intent.putExtra("age", ageSpinner.getSelectedItem().toString());
            intent.putExtra("gender", genderSpinner.getSelectedItem().toString());
            startActivity(intent);
        });
    }

    private void setupAgeSpinner() {
        // Age array with placeholder
        ArrayList<String> ageList = new ArrayList<>();
        ageList.add("Age");
        for (int i = 10; i <= 100; i++) {
            ageList.add(String.valueOf(i));
        }

        // Create ArrayAdapter
        ArrayAdapter<String> ageAdapter = new ArrayAdapter<>(this, R.layout.spinner_item, ageList);
        ageAdapter.setDropDownViewResource(R.layout.spinner_item);
        ageSpinner.setAdapter(ageAdapter);

        // Add Listener
        ageSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                applySpinnerItemStyle(view);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                // Optional handling
            }
        });
    }

    private void setupGenderSpinner() {
        // Gender array from strings.xml
        ArrayAdapter<CharSequence> genderAdapter = ArrayAdapter.createFromResource(
                this, R.array.gender_array, R.layout.spinner_item);
        genderAdapter.setDropDownViewResource(R.layout.spinner_item);
        genderSpinner.setAdapter(genderAdapter);

        // Add Listener
        genderSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                applySpinnerItemStyle(view);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                // Optional handling
            }
        });
    }

    private void setupConditionSpinner() {
        // Condition array with placeholder
        ArrayList<String> conditions = new ArrayList<>();
        conditions.add("Any Medical Conditions");
        conditions.add("Yes, I have Heart Medical Condition");
        conditions.add("Yes, I have A Sleep Disorder Condition");
        conditions.add("No, I Have No Medical Conditions");

        // Create ArrayAdapter
        ArrayAdapter<String> conditionAdapter = new ArrayAdapter<>(this, R.layout.spinner_item, conditions);
        conditionAdapter.setDropDownViewResource(R.layout.spinner_item);
        conditionSpinner.setAdapter(conditionAdapter);

        // Add Listener
        conditionSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                applySpinnerItemStyle(view);
                // Check if the user selected a medical condition
                String selectedCondition = conditions.get(position);
                if (selectedCondition.equals("Yes, I have Heart Medical Condition") ||
                        selectedCondition.equals("Yes, I have A Sleep Disorder Condition")) {
                    showWarningDialog();
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                // Optional handling
            }
        });
    }


    private void applySpinnerItemStyle(View view) {
        if (view instanceof TextView) {
            TextView textView = (TextView) view;
            textView.setTextColor(Color.WHITE); // Set text color to white
            textView.setBackgroundColor(Color.parseColor("#1E1E2E")); // Set background color
        }
    }
    private void showWarningDialog() {
        runOnUiThread(() -> { // Ensure it runs on the main UI thread
            new AlertDialog.Builder(this)
                    .setTitle("Important Notice")
                    .setMessage("Our product may not be able to wake you up on time and might lose some results accuracy.\n\nThank you for your understanding.\n\nIf you still wish to use our application, please click Yes.")
                    .setPositiveButton("Yes", (dialog, which) -> dialog.dismiss())  // Dismiss the dialog if user chooses Yes
                    .setNegativeButton("No", (dialog, which) -> finishAffinity())  // Close the app if user chooses No
                    .setCancelable(false)  // Prevent dismissal by clicking outside
                    .show();
        });
    }
}
