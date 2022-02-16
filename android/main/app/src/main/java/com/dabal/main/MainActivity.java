package com.dabal.main;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import com.dabal.main.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'main' library on application startup.
    static {
        System.loadLibrary("main");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());
        mainFromJNI();
    }

    /**
     * A native method that is implemented by the 'main' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native void mainFromJNI();
}