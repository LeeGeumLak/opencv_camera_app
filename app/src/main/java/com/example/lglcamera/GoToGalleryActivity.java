package com.example.lglcamera;

import androidx.appcompat.app.AppCompatActivity;
import androidx.viewpager.widget.ViewPager;

import android.os.Bundle;
import android.os.Environment;

public class GoToGalleryActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_go_to_gallery);

        ViewPager vpGallery = (ViewPager)findViewById(R.id.vpGallery);
        VpGalleryAdapter vpGalleryAdapter = new VpGalleryAdapter(this, Environment.getExternalStorageDirectory() + "/Images/");
        vpGallery.setAdapter(vpGalleryAdapter);
    }
}
