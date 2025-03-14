package com.example.sack;

import android.app.Activity;
import android.content.Intent;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.MenuItem;
import com.google.android.material.bottomnavigation.BottomNavigationView;

public class NavigationBar {

    public static void setupNavigation(Activity activity, BottomNavigationView bottomNavigationView) {
        bottomNavigationView.setOnItemSelectedListener(item -> {
            return onNavigationItemSelected(activity, item);
        });

        // Get the selected item
        int selectedItem = getSelectedItem(activity);

        // Ensure the correct menu item is highlighted
        if (selectedItem != -1) {
            new Handler(Looper.getMainLooper()).postDelayed(() -> {
                bottomNavigationView.setSelectedItemId(selectedItem);
                // Manually force selection
                MenuItem menuItem = bottomNavigationView.getMenu().findItem(selectedItem);
                if (menuItem != null) {
                    menuItem.setChecked(true);
                }
                Log.d("NavigationDebug", "Set selected item: " + selectedItem);
            }, 100);
        }
    }

    private static boolean onNavigationItemSelected(Activity activity, MenuItem item) {
        int id = item.getItemId();

        // Prevent reloading the same activity
        if (id == getSelectedItem(activity)) {
            return false;
        }

        Intent intent = null;
        if (id == R.id.nav_home) {
            intent = new Intent(activity, HomePage.class);
        } else if (id == R.id.nav_alarm) {
            intent = new Intent(activity, AlarmSetPage.class);
        } else if (id == R.id.nav_ConnectToDevice) {
            intent = new Intent(activity, ConnectToDevice.class);
        } else if (id == R.id.nav_profile) {
            intent = new Intent(activity, ProfileActivity.class);
        }

        if (intent != null) {
            activity.startActivity(intent);
            activity.overridePendingTransition(0, 0); // Smooth transition
            activity.finish(); // Prevent stacking multiple activities
            return true;
        }

        return false;
    }

    private static int getSelectedItem(Activity activity) {
        if (activity instanceof HomePage) return R.id.nav_home;
        if (activity instanceof AlarmSetPage) return R.id.nav_alarm;
        if (activity instanceof ConnectToDevice) return R.id.nav_ConnectToDevice;
        if (activity instanceof ProfileActivity) return R.id.nav_profile;
        return -1;
    }
}
