package com.example.sack;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.AdapterView;
public class SignUpPage extends AppCompatActivity {
@Override
protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.signuppage);
    Spinner spinnerAge = findViewById(R.id.agespinner);
    int StartAge = 14;
    int EndAge = 100;
String[] ageArray = new String[EndAge-StartAge+1];
for(int i=0; i<ageArray.length ; i++) {
    ageArray[i] = String.valueOf(StartAge + 1);
}

    // Create an ArrayAdapter using the generated age array
    ArrayAdapter<String> adapter = new ArrayAdapter<>(this,
            android.R.layout.simple_spinner_item, ageArray);

    // Specify the layout to use when the list of choices appears
    adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

    // Apply the adapter to the Spinner
    spinnerAge.setAdapter(adapter);

    // Set an item selected listener (optional)
    spinnerAge.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView<?> parentView, android.view.View view, int position, long id) {
            String selectedAge = parentView.getItemAtPosition(position).toString();
            // You can use selectedAge here (for example, save or display it)
        }

        @Override
        public void onNothingSelected(AdapterView<?> parentView) {
            // Optional: Handle case when nothing is selected
        }
    });
}
}
