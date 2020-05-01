package com.example.lglcamera;

import androidx.appcompat.app.AppCompatActivity;

import android.animation.Animator;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.airbnb.lottie.LottieAnimationView;

public class LoadingActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_loading);

        //Lottie Animation
        LottieAnimationView animationView = findViewById(R.id.animation_view);

        // intent setting (loading to main)
        final Intent main_intent = new Intent(LoadingActivity.this, MainActivity.class);

        /*try {
            //Lottie Animation start
            animationView.playAnimation();

            //Thread.sleep(5000); //대기 초 설정(5초)
            //startActivity(main_intent);

            //finish();

        } catch (Exception e) {
            Log.e(TAG, "LoadingActivity ERROR", e);
        }*/

        animationView.playAnimation();

        animationView.addAnimatorListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animation) {
                Log.e("Animation:","start");
            }

            @Override
            public void onAnimationEnd(Animator animation) {
                Log.e("Animation:","end");

                startActivity(main_intent);

                finish();
            }

            @Override
            public void onAnimationCancel(Animator animation) {
                Log.e("Animation:","cancel");
            }

            @Override
            public void onAnimationRepeat(Animator animation) {
                Log.e("Animation:","repeat");
            }
        });
    }
}
