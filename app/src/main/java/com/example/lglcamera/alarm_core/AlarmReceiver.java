package com.example.lglcamera.alarm_core;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class AlarmReceiver extends BroadcastReceiver {
    private Context context;
    private String INTENT_ACTION = Intent.ACTION_BOOT_COMPLETED;

    @Override
    public void onReceive(Context context, Intent intent) {

        this.context = context;

        // intent 로부터 전달받은 string
        String get_yout_string = intent.getExtras().getString("state");

        // service intent 생성
        Intent service_intent = new Intent(context, AlarmService.class);

        // service로 string값 보내기
        service_intent.putExtra("state", get_yout_string);

        // start the service
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O){
            this.context.startForegroundService(service_intent);
        }
        else {
            this.context.startService(service_intent);
        }
    }
}