package com.example.lglcamera.activity;

import androidx.appcompat.app.AppCompatActivity;
import androidx.viewpager.widget.ViewPager;

import android.os.Bundle;
import android.os.Environment;

import com.example.lglcamera.R;
import com.example.lglcamera.adapter.VpGalleryAdapter;

public class GoToGalleryActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_go_to_gallery);

        // viewpager : 데이터를 페이지 단위로 표시, flip 하여 페이지를 전환하게 하는 컨테이너
        ViewPager vpGallery = (ViewPager)findViewById(R.id.vpGallery);
        VpGalleryAdapter vpGalleryAdapter = new VpGalleryAdapter(this, Environment.getExternalStorageDirectory() + "/Images/");
        vpGallery.setAdapter(vpGalleryAdapter);
    }
}