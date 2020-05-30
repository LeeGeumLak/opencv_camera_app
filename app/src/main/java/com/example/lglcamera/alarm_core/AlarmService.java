package com.example.lglcamera.alarm_core;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.IBinder;

import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.core.app.NotificationCompat;

import com.example.lglcamera.R;
import com.example.lglcamera.activity.MainActivity;


public class AlarmService extends Service {
    private NotificationManager notificationManager;
    private Notification notification;
    private PendingIntent pendingIntent;
    private int startId;

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        //
        /*long when = System.currentTimeMillis();
        NotificationManager notificationManager = (NotificationManager) context
                .getSystemService(Context.NOTIFICATION_SERVICE);

        Intent notificationIntent = new Intent(context, EVentsPerform.class);
        notificationIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

        PendingIntent pendingIntent = PendingIntent.getActivity(context, 0,
                notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);


        Uri alarmSound = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);

        NotificationCompat.Builder mNotifyBuilder = new NotificationCompat.Builder(
                context).setSmallIcon(R.drawable.applogo)
                .setContentTitle("Alarm Fired")
                .setContentText("Events to be Performed").setSound(alarmSound)
                .setAutoCancel(true).setWhen(when)
                .setContentIntent(pendingIntent)
                .setVibrate(new long[]{1000, 1000, 1000, 1000, 1000});
        notificationManager.notify(MID, mNotifyBuilder.build());
        MID++;*/
        //

        int MID = 1;
        pendingIntent = PendingIntent.getActivity(this, 0,
                new Intent(getApplicationContext(), MainActivity.class), PendingIntent.FLAG_UPDATE_CURRENT);

        if (Build.VERSION.SDK_INT >= 26) {
            NotificationChannel channel = new NotificationChannel("default", "알람기능",
                    NotificationManager.IMPORTANCE_DEFAULT);

            long when = System.currentTimeMillis();

            notificationManager = ((NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE));
            notificationManager.createNotificationChannel(channel);

            NotificationCompat.Builder notiBuilder = new NotificationCompat.Builder(getApplicationContext())
                    .setContentTitle("LGL Camera")
                    .setContentText("LGL Camera 를 통해 특별한 경험을 기록하세요!!")
                    .setSmallIcon(R.mipmap.ic_launcher)
                    .setAutoCancel(true).setWhen(when)
                    .setContentIntent(pendingIntent);

            notificationManager.notify(MID, notiBuilder.build());
            MID++;

            /*notification = new NotificationCompat.Builder(this, "default")
                    .setContentTitle("LGL Camera")
                    .setContentText("LGL Camera 를 통해 특별한 경험을 기록하세요!!")
                    .setSmallIcon(R.mipmap.ic_launcher)
                    .setAutoCancel(true)
                    .setContentIntent(pendingIntent)
                    .build();*/

            //notificationManager.notify(1, notification);
            //startForeground(1, notification);
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.N)
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        String getState = intent.getExtras().getString("state");

        assert getState != null;
        switch (getState) {
            case "alarm on":
                //startForeground(1, notification);
                break;
            case "alarm off":
                /*this.stopForeground(true);
                notificationManager.cancel(1);*/
                onDestroy();

                break;
            default:
                break;
        }

        /*// 알람음 재생 X , 알람음 시작 클릭
        if(!this.isRunning && startId == 1) {

            mediaPlayer = MediaPlayer.create(this,R.raw.alarm_sound);
            mediaPlayer.start();

            this.isRunning = true;
            this.startId = 0;
        }

        // 알람음 재생 O , 알람음 종료 버튼 클릭
        else if(this.isRunning && startId == 0) {

            mediaPlayer.stop();
            mediaPlayer.reset();
            mediaPlayer.release();

            this.isRunning = false;
            this.startId = 0;

            onDestroy();
        }

        // 알람음 재생 X , 알람음 종료 버튼 클릭
        else if(!this.isRunning && startId == 0) {

            this.isRunning = false;
            this.startId = 0;

            onDestroy();
        }

        // 알람음 재생 O , 알람음 시작 버튼 클릭
        else if(this.isRunning && startId == 1){

            this.isRunning = true;
            this.startId = 1;
        }*/

        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        // 서비스 파괴
        super.onDestroy();
    }
}