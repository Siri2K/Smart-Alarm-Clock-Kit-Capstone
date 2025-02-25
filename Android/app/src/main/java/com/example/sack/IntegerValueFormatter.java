package com.example.sack;
import com.github.mikephil.charting.formatter.ValueFormatter;

public class IntegerValueFormatter extends ValueFormatter {
    @Override
    public String getFormattedValue(float value) {
        return String.valueOf((int) value); // Convert to integer for display
    }
}