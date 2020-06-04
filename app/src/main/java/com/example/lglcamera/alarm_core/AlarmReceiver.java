package com.example.lglcamera.alarm_core;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.core.app.NotificationCompat;

import com.example.lglcamera.R;
import com.example.lglcamera.activity.MainActivity;

public class AlarmReceiver extends BroadcastReceiver {
    private Context context;

    public AlarmReceiver() { }

    @Override
    public void onReceive(Context context, Intent intent) {

        /*this.context = context;

        Log.d("AlarmReceiver", "onReceive :: 브로드캐스트 리시버 진입");

        // intent 로부터 전달받은 string (key : state / value : "alarm on" 이나 "alarm off")
        String get_string = intent.getExtras().getString("state");

        // alarm repeat 되므로 null 값이 넘어옴(두번째 부터) : 예외처리
        if(get_string == null) {
            get_string = "alarm on";
        }

        // service intent 생성
        Intent service_intent = new Intent(context, AlarmService.class);

        // service로 string값 보내기
        service_intent.putExtra("state", get_string);

        Log.d("AlarmReceiver", "onReceive :: extra data 서비스 인텐트에 put");

        // start the service
        // 이미 실행중인 service를 반복해서 실행하면 서비스의 onCreate 이 아닌 onStartCommand가 호출된다
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O){
            this.context.startForegroundService(service_intent);

            Log.d("AlarmReceiver", "onReceive :: startForegroundService 실행");

        }
        else {
            this.context.startService(service_intent);

            Log.d("AlarmReceiver", "onReceive :: startService 실행");

        }*/
        Log.d("AlarmReceiver", "onReceive :: 브로드캐스트 리시버 진입");

        NotificationManager notificationManager = (NotificationManager)
                context.getSystemService(Context.NOTIFICATION_SERVICE);

        Intent contentIntent = new Intent(context, MainActivity.class);
        PendingIntent contentPendingIntent = PendingIntent.getActivity
                (context, 0, contentIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(context)
                .setSmallIcon(R.mipmap.ic_launcher)
                .setContentTitle("LGL Camera")
                .setContentText("LGL Camera 를 통해 특별한 경험을 기록하세요!!")
                .setContentIntent(contentPendingIntent)
                .setAutoCancel(true)
                .setPriority(NotificationCompat.PRIORITY_HIGH);

        notificationManager.notify(1, builder.build());
    }
}

/*
import android.app.*;
import android.content.*;
import android.os.*;

public class AlarmReceiver extends BroadcastReceiver {
    private Context context;

    @Override
    public void onReceive(Context context, Intent intent) {
        Intent service_intent = new Intent(context, AlarmService.class);
        // start the service
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O){
            this.context.startForegroundService(service_intent);
        }
        else {
            this.context.startService(service_intent);
        }
        //context.startService(background);
    }

}*/
