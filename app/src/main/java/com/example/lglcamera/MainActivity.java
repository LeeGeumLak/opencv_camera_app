package com.example.lglcamera;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.FileProvider;

import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.annotation.TargetApi;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.Toast;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;
import org.opencv.imgcodecs.Imgcodecs;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.concurrent.Semaphore;

import static android.Manifest.permission.CAMERA;
import static android.hardware.Camera.CameraInfo.CAMERA_FACING_BACK;
import static android.provider.MediaStore.Files.FileColumns.MEDIA_TYPE_IMAGE;


public class MainActivity extends AppCompatActivity implements CameraBridgeViewBase.CvCameraViewListener2 {

    private static final String TAG = "MainActivity";

    private Button Button_RGB, Button_Gray, Button_HSV, Button_Text, Button_capture, Button_sticker;
    private Button Button_change, Button_filter;

    private Mat matInput, matResult;
    private int GrayScale, RGBA, HSV;

    private int cameraId = 0;

    private Uri fileUri;
    private File file;
    private String filename;

    private CameraBridgeViewBase openCvCameraView;

    // Native c++ 메서드
    public native void ConvertRGBtoGray(long mat_addr_input, long mat_addr_result);
    public native void ConvertRGBtoHSV(long mat_addr_input, long mat_addr_result);

    private final Semaphore writeLock = new Semaphore(1);

    public void getWriteLock() throws InterruptedException {
        writeLock.acquire();
    }
    public void releaseWriteLock() {
        writeLock.release();
    }

    static {
        System.loadLibrary("opencv_java4");
        System.loadLibrary("native-lib");
    }

    private BaseLoaderCallback loaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS: {
                    openCvCameraView.enableView();
                }
                break;

                default: {
                    super.onManagerConnected(status);
                }
                break;
            }
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main);

        // URI exposure 를 무시
       /* StrictMode.VmPolicy.Builder builder = new StrictMode.VmPolicy.Builder();
        StrictMode.setVmPolicy(builder.build());*/

        buttonInit();

        // 권한 설정 후, 카메라 실행
        openCvCameraView = (CameraBridgeViewBase) findViewById(R.id.camera_view);
        openCvCameraView.setVisibility(SurfaceView.VISIBLE);
        openCvCameraView.setCvCameraViewListener(this);
        openCvCameraView.setCameraIndex(CAMERA_FACING_BACK); // 후면 카메라 모드
        cameraId = CAMERA_FACING_BACK;
    }

    // 버튼 초기화 및 리스너 설정
    private void buttonInit() {
        Button_RGB = findViewById(R.id.Button_RGB);
        Button_Gray = findViewById(R.id.Button_Gray);
        Button_HSV = findViewById(R.id.Button_HSV);

        Button_Text = findViewById(R.id.Button_Text);
        Button_sticker = findViewById(R.id.Button_Sticker);

        Button_capture = findViewById(R.id.Button_capture);
        Button_change = findViewById(R.id.Button_change);
        //Button_filter = findViewById(R.id.Button_filter);

        Button_RGB.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                RGBA = 1;
                GrayScale = 0;
                HSV  = 0;
            }
        });

        Button_HSV.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                RGBA = 0;
                GrayScale = 0;
                HSV  = 1;
            }
        });

        Button_Gray.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                RGBA = 0;
                GrayScale = 1;
                HSV  = 0;
            }
        });

        Button_Text.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //TODO : 문자인식
                Intent textify_intent = new Intent(MainActivity.this, TextifyActivity.class);
                startActivity(textify_intent);
            }
        });

        Button_sticker.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //TODO : 얼굴 인식
                Intent sticker_intent = new Intent(MainActivity.this, FaceDetectionActivity.class);
                startActivity(sticker_intent);
            }
        });

        Button_change.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                swapCamera();
            }
        });

        /*Button_filter.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
            }
        });*/

        // 카메라 찍기 버튼 리스너
        Button_capture.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                /*Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
                fileUri = getOutputMediaFileUri(MEDIA_TYPE_IMAGE); // 이미지를 저장할 파일 생성
                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, fileUri);

                if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
                    startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE);
                }*/

                try {
                    Toast.makeText(getApplicationContext(), "taking picture", Toast.LENGTH_SHORT).show();
                    //Log.d(TAG, "capture : after try");

                    getWriteLock();

                    //Log.d(TAG, "capture : after getWriteLock()");
                    /*File path = new File(Environment.getExternalStorageDirectory() + "/Images/");
                    path.mkdirs();
                    File file = new File(path, "image.png");
                    String filename = file.toString();*/

                    fileUri = getOutputMediaFileUri(MEDIA_TYPE_IMAGE); // 이미지를 저장할 파일 생성

                    boolean ret = Imgcodecs.imwrite( filename, matResult); // 위 생성한 파일에 현재 카메라 화면 씌움

                    if ( ret ) {
                        Log.d(TAG, "take picture SUCCESS");
                    }
                    else {
                        Log.d(TAG, "take picture FAIL");
                    }

                    Intent mediaScanIntent = new Intent( Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
                    mediaScanIntent.setData(Uri.fromFile(file));
                    sendBroadcast(mediaScanIntent);

                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                releaseWriteLock();

            }
        });
    }

    // 전/후면 카메라 전환 메서드
    private void swapCamera() {
        cameraId = cameraId^1; // 카메라 방향 바꾸기 ==> 기존 camera id에 1 과 비트연산하여 1 / 0 결과 나오게끔
        openCvCameraView.disableView();
        openCvCameraView.setCameraIndex(cameraId);
        openCvCameraView.enableView();
    }

    private Uri getOutputMediaFileUri(int type){
        // 아래 capture한 사진이 저장될 file 공간을 생성하는 method를 통해 반환되는 File의 URI를 반환

        return FileProvider.getUriForFile(getApplicationContext(), "com.example.lglcamera.provider", getOutputMediaFile(type));
        //return Uri.fromFile(getOutputMediaFile(type));
    }

    private File getOutputMediaFile(int type){
        // 외부 저장소에 이 앱을 통해 촬영된 사진만 저장할 directory 경로와 File을 연결
        /*File mediaStorageDir = new File(Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_PICTURES), "LGL_Camera");*/
        File mediaStorageDir = new File(Environment.getExternalStorageDirectory() + "/Images/");

        // 폴더 없으면 생성
        if (!mediaStorageDir.exists()){
            // 예외처리
            if (!mediaStorageDir.mkdirs()){ // 만약 mkdirs()가 제대로 동작하지 않을 경우, 오류 Log를 출력한 뒤, 해당 method 종료
                Log.d(TAG, "폴더 생성 실패");
                return null;
            }
        }

        // 찍은 시간에 따른 파일 이름 설정
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        File mediaFile;

        if (type == MEDIA_TYPE_IMAGE){
            mediaFile = new File(mediaStorageDir.getPath() + File.separator +
                    "IMG_"+ timeStamp + ".jpg");
            filename = mediaStorageDir.getPath() + File.separator + "IMG_"+ timeStamp + ".jpg";
        } else {
            return null;
        }

        file = mediaFile;
        if(!file.exists()) {
            try {
                file.createNewFile();
                Toast.makeText(getApplicationContext(), "successed to create " + filename, Toast.LENGTH_SHORT).show();

            }
            catch(IOException e) {
                Toast.makeText(getApplicationContext(), "failed to create " + filename, Toast.LENGTH_SHORT).show();
            }
        }
        else {
            Toast.makeText(getApplicationContext(), filename + " is already exists", Toast.LENGTH_SHORT).show();
        }

        return mediaFile;
    }

    @Override
    public void onPause() {
        super.onPause();

        if (openCvCameraView != null)
            openCvCameraView.disableView();
    }

    @Override
    public void onResume() {
        super.onResume();

        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "onResume : Internal OpenCV library not found.");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_2_0, this, loaderCallback);
        } else {
            Log.d(TAG, "onResume : OpenCV library found inside package. Using it!");
            loaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }


    public void onDestroy() {
        super.onDestroy();

        if (openCvCameraView != null)
            openCvCameraView.disableView();
    }

    @Override
    public void onCameraViewStarted(int width, int height) {

    }

    @Override
    public void onCameraViewStopped() {

    }

    // inputFrame 에 filter 씌어서 outputFrame 으로 return
    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        matInput = inputFrame.rgba();

        if (matResult == null) {
            matResult = new Mat(matInput.rows(), matInput.cols(), matInput.type());
        }

        matResult = matInput;

        // 필터링
        /*if(RGBA == 1) {
            matResult = matInput;
        }*/
        if(GrayScale == 1) {
            ConvertRGBtoGray(matInput.getNativeObjAddr(), matResult.getNativeObjAddr());
        }
        if(HSV == 1) {
            ConvertRGBtoHSV(matInput.getNativeObjAddr(), matResult.getNativeObjAddr());
        }

        return matResult;
    }

    protected List<? extends CameraBridgeViewBase> getCameraViewList() {
        return Collections.singletonList(openCvCameraView);
    }


    //권한 관련 메소드
    private static final int CAMERA_PERMISSION_REQUEST_CODE = 200;

    protected void onCameraPermissionGranted() {
        List<? extends CameraBridgeViewBase> cameraViews = getCameraViewList();
        if (cameraViews == null) {
            return;
        }
        for (CameraBridgeViewBase cameraBridgeViewBase : cameraViews) {
            if (cameraBridgeViewBase != null) {
                cameraBridgeViewBase.setCameraPermissionGranted();
            }
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        boolean havePermission = true;

        // 권한 여부
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (checkSelfPermission(CAMERA) != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(new String[]{CAMERA}, CAMERA_PERMISSION_REQUEST_CODE);
                havePermission = false;
            }
        }
        if (havePermission) {
            onCameraPermissionGranted();
        }
    }

    @Override
    @TargetApi(Build.VERSION_CODES.M)
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == CAMERA_PERMISSION_REQUEST_CODE && grantResults.length > 0
                && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            onCameraPermissionGranted();
        } else {
            showDialogForPermission("앱을 실행하려면 권한 설정을 해야합니다.");
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

    // 카메라 권한 설정 창
    @TargetApi(Build.VERSION_CODES.M)
    private void showDialogForPermission(String msg) {

        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
        builder.setTitle("알림");
        builder.setMessage(msg);
        builder.setCancelable(false);
        builder.setPositiveButton("예", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                requestPermissions(new String[]{CAMERA}, CAMERA_PERMISSION_REQUEST_CODE);
            }
        });
        builder.setNegativeButton("아니오", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface arg0, int arg1) {
                finish();
            }
        });
        builder.create().show();
    }
}