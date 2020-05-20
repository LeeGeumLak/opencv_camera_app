package com.example.lglcamera.activity;

//기본 패키지
import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.FileProvider;

import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.Toast;

//openCV 패키지
import com.example.lglcamera.R;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.imgcodecs.Imgcodecs;
import org.opencv.imgproc.Imgproc;
import org.opencv.photo.Photo;

//자바 패키지
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.concurrent.Semaphore;

import static android.hardware.Camera.CameraInfo.CAMERA_FACING_BACK;
import static android.provider.MediaStore.Files.FileColumns.MEDIA_TYPE_IMAGE;

public class FaceDetectionActivity extends AppCompatActivity
        implements CameraBridgeViewBase.CvCameraViewListener2 {
    private static final String TAG = "FaceDetectionActivity";

    private CameraBridgeViewBase openCvCameraView;

    private Button Button_capture, Button_change, Button_gallery;

    private Mat matInput;
    private Mat matResult;
    private int cameraId = 0;

    private Uri fileUri;
    private File file;
    private File mediaStorageDir; // 캡쳐한 이미지가 저장되는 디렉토리
    private String fileName;

    private int RequestPreviewImg = 1234;
    private boolean ret = false;

    // Native c++ 메서드
    //public native long loadCascade(String cascadeFileName );
    //public native void detect(long cascadeClassifier_face, long cascadeClassifier_eye, long mat_addr_input, long mat_addr_result);
    //public native long LoadCascade(String cascadeFileName );
    //public native void DetectAndDraw(long cascadeClassifier_face, long cascadeClassifier_eye, long mat_addr_input, long mat_addr_result);
    //public native void DetectAndSunglasses(long mat_addr_input, long mat_addr_output, long cascadeClassifier_face, long cascadeClassifier_eye, double scale);
    //CascadeClassifier& cascade, CascadeClassifier& nestedCascade == long cascadeClassifier_face, long cascadeClassifier_eye

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

    /*private void read_cascade_file(){
        // Assets에서 파일 가져와 복사
        copyFile("haarcascade_frontalface_alt.xml");
        copyFile("haarcascade_eye_tree_eyeglasses.xml");

        Log.d(TAG, "read_cascade_file:");

        // 외부 저장소에서 파일 읽어와 객체 로드
        cascadeClassifier_face = LoadCascade( "haarcascade_frontalface_alt.xml");
        Log.d(TAG, "read_cascade_file:");

        cascadeClassifier_eye = LoadCascade( "haarcascade_eye_tree_eyeglasses.xml");
    }*/

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

        // xml 파일 읽어와 객체 로드
        //read_cascade_file();

        openCvCameraView = (CameraBridgeViewBase) findViewById(R.id.camera_view);
        openCvCameraView.setVisibility(SurfaceView.VISIBLE);
        openCvCameraView.setCvCameraViewListener(this);
        openCvCameraView.setCameraIndex(CAMERA_FACING_BACK); // 후면 카메라 모드
        cameraId = CAMERA_FACING_BACK;

        //loaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
    }

    // 전/후면 카메라 전환 메서드
    private void swapCamera() {
        cameraId = cameraId^1; // 카메라 방향 바꾸기 ==> 기존 camera id에 1 과 비트연산하여 1 / 0 결과 나오게끔
        openCvCameraView.disableView();
        openCvCameraView.setCameraIndex(cameraId);
        openCvCameraView.enableView();
    }

    // 버튼 초기화 및 리스너 설정
    private void buttonInit() {
        Button_capture = findViewById(R.id.Button_capture);
        Button_change = findViewById(R.id.Button_change);
        Button_gallery = findViewById(R.id.Button_gallery);

        Button_change.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                swapCamera();
            }
        });

        Button_gallery.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // 기존 갤러리로 이동하는 intent
                /*Intent galleryIntent = new Intent(Intent.ACTION_VIEW);
                galleryIntent.setType(android.provider.MediaStore.Images.Media.CONTENT_TYPE);
                //galleryIntent.setData(android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                startActivity(galleryIntent);*/

                //pager adapter 사용한 커스텀 갤러리로 이동하는 intent
                //TODO : 현재 이미지를 볼 수 있으나, 목록 형태가 아닌 이미지 뷰어 형태(무슨 파일들이 있는지 한번에 볼 수 없음)
                Intent galleryIntent = new Intent(FaceDetectionActivity.this, GoToGalleryActivity.class);
                startActivity(galleryIntent);
            }
        });

        Button_capture.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                /*Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
                fileUri = getOutputMediaFileUri(MEDIA_TYPE_IMAGE); // 이미지를 저장할 파일 생성
                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, fileUri);

                if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
                    startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE);
                }*/

                try {
                    //Log.d(TAG, "capture : after try");

                    Toast.makeText(getApplicationContext(), "taking picture", Toast.LENGTH_SHORT).show();

                    getWriteLock();

                    //Log.d(TAG, "capture : after getWriteLock()");
                    /*File path = new File(Environment.getExternalStorageDirectory() + "/Images/");
                    path.mkdirs();
                    File file = new File(path, "image.png");
                    String filename = file.toString();*/

                    fileUri = getOutputMediaFileUri(MEDIA_TYPE_IMAGE); // 이미지를 저장할 파일 생성

                    Imgcodecs.imwrite( fileName, matInput); // 위 생성한 파일에 현재 카메라 화면 씌움(얼굴 인식하여 동그라미로 표시된 것은 X)

                    // 씌운 파일을 이미지 프리뷰 액티비티로 putExtra하여 인텐트 이동
                    Intent previewImgIntent = new Intent(FaceDetectionActivity.this, PhotoPreviewActivity.class);
                    previewImgIntent.putExtra("filename", fileName);
                    startActivityForResult(previewImgIntent, RequestPreviewImg);

                    // ForResult 로 받아온 ret 값에 따라서 사진 저장 성공/실패 여부
                    if ( ret ) {
                        Log.d(TAG, "take picture SUCCESS");

                        Intent mediaScanIntent = new Intent( Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
                        mediaScanIntent.setData(Uri.fromFile(file));
                        sendBroadcast(mediaScanIntent);
                    }
                    else {
                        Log.d(TAG, "take picture FAIL");
                    }

                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                releaseWriteLock();

            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == RequestPreviewImg) {
            String ret_str = data.getStringExtra("isSave");

            if(ret_str.equals("yes")) ret = true;
            else if(ret_str.equals("no")) ret = false;
        }
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
            fileName = mediaStorageDir.getPath() + File.separator + "IMG_"+ timeStamp + ".jpg";
        } else {
            return null;
        }

        file = mediaFile;
        if(!file.exists()) {
            try {
                file.createNewFile();
                Toast.makeText(getApplicationContext(), "successed to create " + fileName, Toast.LENGTH_SHORT).show();

            }
            catch(IOException e) {
                Toast.makeText(getApplicationContext(), "failed to create " + fileName, Toast.LENGTH_SHORT).show();
            }
        }
        else {
            Toast.makeText(getApplicationContext(), fileName + " is already exists", Toast.LENGTH_SHORT).show();
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
        matInput = inputFrame.rgba();

        if (matResult == null) {
            matResult = new Mat(matInput.rows(), matInput.cols(), matInput.type());
        }

        matResult = matInput;

        //DetectAndDraw(cascadeClassifier_face,cascadeClassifier_eye, matInput.getNativeObjAddr(), matResult.getNativeObjAddr());
        //double scale = 1;
        //DetectAndSunglasses(matInput.getNativeObjAddr(), matResult.getNativeObjAddr(), cascadeClassifier_face,cascadeClassifier_eye, scale);

        return matResult;
    }
}