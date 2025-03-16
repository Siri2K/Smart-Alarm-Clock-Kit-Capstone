package com.example.sack;

import android.app.Activity;
import android.content.Intent;
import android.view.MenuItem;
import com.google.android.material.bottomnavigation.BottomNavigationView;

public class NavigationBar {

    public static void setupNavigation(Activity activity, BottomNavigationView bottomNavigationView) {
        bottomNavigationView.setOnItemSelectedListener(item -> onNavigationItemSelected(activity, item));

        // Highlight the current activity
        int selectedItem = getSelectedItem(activity);
        bottomNavigationView.setSelectedItemId(selectedItem);
    }

    private static boolean onNavigationItemSelected(Activity activity, MenuItem item) {
        int id = item.getItemId();

        if (id == R.id.nav_home && !(activity instanceof HomePage)) {
            activity.startActivity(new Intent(activity, HomePage.class));
        } else if (id == R.id.nav_alarm && !(activity instanceof AlarmSetPage)) {
            activity.startActivity(new Intent(activity, AlarmSetPage.class));
        } else if (id == R.id.nav_ConnectToDevice && !(activity instanceof ConnectToDevice)) {
            activity.startActivity(new Intent(activity, ConnectToDevice.class));
        } else if (id == R.id.nav_profile && !(activity instanceof ProfileActivity)) {
            activity.startActivity(new Intent(activity, ProfileActivity.class));
        } else if (id == R.id.nav_wifi_ble && !(activity instanceof WiFiBleSettingsActivity)) {
            activity.startActivity(new Intent(activity, WiFiBleSettingsActivity.class));
        } else {
            return false;
        }

        activity.overridePendingTransition(0, 0); // Smooth transition
        return true;
    }

    private static int getSelectedItem(Activity activity) {
        if (activity instanceof HomePage) return R.id.nav_home;
        if (activity instanceof AlarmSetPage) return R.id.nav_alarm;
        if (activity instanceof ConnectToDevice) return R.id.nav_ConnectToDevice;
        if (activity instanceof ProfileActivity) return R.id.nav_profile;
        if (activity instanceof WiFiBleSettingsActivity) return R.id.nav_wifi_ble;
        return -1;
    }
}
