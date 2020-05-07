package com.example.lglcamera;

//기본 패키지
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.FileProvider;

import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.Toast;

//openCV 패키지
import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.imgcodecs.Imgcodecs;
import org.opencv.imgproc.Imgproc;

//자바 패키지
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.concurrent.Semaphore;

import static android.provider.MediaStore.Files.FileColumns.MEDIA_TYPE_IMAGE;

public class FaceDetectionActivity extends AppCompatActivity
        implements CameraBridgeViewBase.CvCameraViewListener2 {
    private static final String TAG = "FaceDetectionActivity";

    private CameraBridgeViewBase openCvCameraView;

    private Button Button_capture, Button_change;

    private Mat matInput;
    private Mat matResult;
    private int cameraType = 0;

    private Uri fileUri;
    private File file;
    private String filename;

    // Native c++ 메서드
    public native long loadCascade(String cascadeFileName );
    public native void detect(long cascadeClassifier_face, long cascadeClassifier_eye, long mat_addr_input, long mat_addr_result);

    public long cascadeClassifier_face = 0;
    public long cascadeClassifier_eye = 0;

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

    private void copyFile(String filename) {
        String baseDir = Environment.getExternalStorageDirectory().getPath();
        String pathDir = baseDir + File.separator + filename;

        AssetManager assetManager = this.getAssets();

        InputStream inputStream = null;
        OutputStream outputStream = null;

        try {
            Log.d( TAG, "copyFile : 다음 경로로 파일복사 "+ pathDir);
            inputStream = assetManager.open(filename);
            outputStream = new FileOutputStream(pathDir);

            byte[] buffer = new byte[1024];
            int read;

            while ((read = inputStream.read(buffer)) != -1) {
                outputStream.write(buffer, 0, read);
            }

            inputStream.close();
            inputStream = null;
            outputStream.flush();
            outputStream.close();
            outputStream = null;

        } catch (Exception e) {
            Log.d(TAG, "copyFile : 파일 복사 중 예외 발생 "+e.toString() );
        }
    }

    private void read_cascade_file(){
        // Assets에서 파일 가져와 복사
        copyFile("haarcascade_frontalface_alt.xml");
        copyFile("haarcascade_eye_tree_eyeglasses.xml");

        Log.d(TAG, "read_cascade_file:");

        // 외부 저장소에서 파일 읽어와 객체 로드
        cascadeClassifier_face = loadCascade( "haarcascade_frontalface_alt.xml");
        Log.d(TAG, "read_cascade_file:");

        cascadeClassifier_eye = loadCascade( "haarcascade_eye_tree_eyeglasses.xml");
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

        setContentView(R.layout.activity_face_detection);

        buttonInit();

        matResult = new Mat();

        // xml 파일 읽어와 객체 로드
        read_cascade_file();

        openCvCameraView = (CameraBridgeViewBase)findViewById(R.id.activity_surface_view);
        openCvCameraView.setVisibility(SurfaceView.VISIBLE);
        openCvCameraView.setCvCameraViewListener(this);
        openCvCameraView.setCameraIndex(1); // front-camera(1), back-camera(0) 후면 카메라 사용
        cameraType = 1;

        //loaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
    }

    // 전/후면 카메라 전환 메서드
    private void swapCamera() {
        cameraType = cameraType^1; // 카메라 방향 바꾸기 ==> 기존 camera id에 1 과 비트연산하여 1 / 0 결과 나오게끔
        openCvCameraView.disableView();
        openCvCameraView.setCameraIndex(cameraType);
        openCvCameraView.enableView();
    }

    // 버튼 초기화 및 리스너 설정
    private void buttonInit() {
        Button_capture = findViewById(R.id.Button_capture);
        Button_change = findViewById(R.id.Button_change);

        Button_change.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                swapCamera();
            }
        });

        Button_capture.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                //Log.d(TAG, "capture : before try");

                try {
                    Toast.makeText(getApplicationContext(), "taking picture", Toast.LENGTH_SHORT).show();
                    //Log.d(TAG, "capture : after try");

                    getWriteLock();

                    //Log.d(TAG, "capture : after getWriteLock()");

                    fileUri = getOutputMediaFileUri(MEDIA_TYPE_IMAGE); // 이미지를 저장할 파일 생성

                    // TODO : error
                    // error : java.lang.NullPointerException: Attempt to read from field 'long org.opencv.core.Mat.nativeObj' on a null object reference
                    // matResult가 초기화되지 않은 상태에서, 호출되었기 때문
                    // ==> 근본적인 문제 : E/OpenCV/StaticHelper: OpenCV error: Cannot load info library for OpenCV --> 카메라 화면이 뜨지 않음 ( == matResult 가 초기화 되지 않았다는 뜻)
                    // 버전이 안맞는건가? permission problem 은 아님
                    Imgproc.cvtColor(matResult, matResult, Imgproc.COLOR_BGR2RGB, 4);

                    // 생성한 file에 matResult를 씌움
                    boolean ret = Imgcodecs.imwrite( filename, matResult);

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
            if (!mediaStorageDir.mkdirs()){ // 폴더 생성하고, 만약 mkdirs()가 제대로 동작하지 않을 경우, Log 출력 및 종료
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
            Log.d(TAG, "onResum : OpenCV library found inside package. Using it!");
            loaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    public void onDestroy() {
        super.onDestroy();
        if (openCvCameraView != null)
            openCvCameraView.disableView();
    }

    @Override
    public void onBackPressed() {
        finish();
    }

    @Override
    public void onCameraViewStarted(int width, int height) {
    }
    @Override
    public void onCameraViewStopped() {
    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        try {
            //Log.d(TAG, "before getWriteLock()");

            getWriteLock();

            //Log.d(TAG, "after getWriteLock()");

            matInput = inputFrame.rgba();

            if ( matResult == null ) {
                matResult = new Mat(matInput.rows(), matInput.cols(), matInput.type());
            }

            // 영상 180도 회전
            Core.flip(matInput, matInput, 1);

            //Log.d(TAG, "after rotate screen / before detect()");

            detect(cascadeClassifier_face,cascadeClassifier_eye, matInput.getNativeObjAddr(),
                    matResult.getNativeObjAddr());

            //Log.d(TAG, "after detect()");

        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        //Log.d(TAG, "before releaseWriteLock()");

        releaseWriteLock();

        //Log.d(TAG, "after releaseWriteLock()");

        return matResult;
    }
}