package com.dabal.main;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Spinner;
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
        Spinner tl = binding.spinnerTests;
        tl.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parentView, View selectedItemView, int position, long id) {

              //  mostrar menu distinto segun selecciona en otro combo?? como configuro iteraciones?? en principio sería un ui especial por cada opcion..
            }

            @Override
            public void onNothingSelected(AdapterView<?> parentView) {
                // your code here
            }

        });
        //mainFromJNI();
    }
    public void executeClick(View view)
    {
        Spinner tl = binding.spinnerTests;
        String text = ((TextView) tl.getSelectedView()).getText().toString();
        String[] commandLine = {"-t",text};
        mainJNI(commandLine);
    }


    /**
     * A native method that is implemented by the 'main' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native void mainJNI(String[] commandLine);
}