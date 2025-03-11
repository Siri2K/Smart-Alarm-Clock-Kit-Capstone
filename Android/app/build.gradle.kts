plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "com.example.sack"
    compileSdk = 35

    defaultConfig {
        applicationId = "com.example.sack"
        minSdk = 31
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"


        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
}

dependencies {
    implementation("androidx.core:core-ktx:1.9.0")
    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("com.google.android.material:material:1.9.0")
    implementation ("com.github.PhilJay:MPAndroidChart:v3.1.0")
        // Ensure you have this for BLE
    implementation("androidx.lifecycle:lifecycle-runtime-ktx:2.6.1")
    implementation("com.github.PhilJay:MPAndroidChart:v3.1.0")
    implementation(libs.appcompat)
    implementation(libs.material)
    implementation(libs.activity)
    implementation(libs.constraintlayout)
    testImplementation(libs.junit)
    androidTestImplementation(libs.ext.junit)
    androidTestImplementation(libs.espresso.core)
    implementation(libs.core.ktx)
    implementation(libs.appcompat.v161)
    implementation(libs.material.v190)
    implementation(libs.constraintlayout.v214)
}