package com.example.lglcamera.activity;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.AssetManager;
import android.media.FaceDetector;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import com.example.lglcamera.R;

import org.opencv.core.Mat;
import org.opencv.core.MatOfByte;
import org.opencv.imgcodecs.Imgcodecs;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.concurrent.Semaphore;

import static org.opencv.core.Core.findFileOrKeep;
import static org.opencv.imgcodecs.Imgcodecs.IMREAD_COLOR;
import static org.opencv.imgcodecs.Imgcodecs.imread;

public class PhotoPreviewActivity extends AppCompatActivity {

    private static final String TAG = "PhotoPreviewActivity";

    private Button button_yes, button_no;
    private ImageView preview_img;

    private String fileName;
    Mat fileImg;
    private File file;
    private boolean ret;

    public native long LoadCascade(String cascadeFileName );
    //public native void DetectAndSunglasses(long mat_addr_input, long cascadeClassifier_face, long cascadeClassifier_eye);
    //CascadeClassifier& cascade, CascadeClassifier& nestedCascade == long cascadeClassifier_face, long cascadeClassifier_eye

    public long cascadeClassifier_face = 0;
    public long cascadeClassifier_eye = 0;

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
        cascadeClassifier_face = LoadCascade( "haarcascade_frontalface_alt.xml");
        Log.d(TAG, "read_cascade_file:");

        cascadeClassifier_eye = LoadCascade( "haarcascade_eye_tree_eyeglasses.xml");
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_photo_preview);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        Log.e(TAG, "init 들어가기 전");

        init();

        Log.e(TAG, "init 들어갔다 나옴");

        // xml 파일 읽어와 객체 로드
        read_cascade_file();
    }

    // 버튼 및 이미지뷰 초기화
    public void init() {
        button_yes = findViewById(R.id.button_yes);
        button_no = findViewById(R.id.button_no);
        preview_img = findViewById(R.id.preview_img);

        button_yes.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                isSave();
            }
        });

        button_no.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                isCancel();
            }
        });
    }

    @Override
    public void onBackPressed() {
        isCancel();
    }

    @Override
    protected void onResume() {
        super.onResume();

        Log.e(TAG, "onResume 들어옴");

        Intent intent = getIntent();

        if(intent != null) {
            Log.e(TAG, "getIntent 해서 null 이 아닐때");

            long addr = intent.getLongExtra("matInput", 0);
            Mat mat = new Mat(addr);
            fileImg = mat;

            Log.e(TAG, "fileImg 받아옴");


            //file = new File(intent.getStringExtra("file"));
            //file = (File)intent.getExtras().get("file");

            //Log.e(TAG, "file 받아옴");

            //Log.e(TAG, "선글라스 들어가기 전");
            // TODO : 에러 발생 지점 : DetectAndSunglasses 안에 'cvt 직전' 이후

            //DetectAndSunglasses( fileImg.getNativeObjAddr(), cascadeClassifier_face, cascadeClassifier_eye);

            //Log.e(TAG, "선글라스 들어가고 나온 후");

            //선글라스를 씌운 이미지를 fileName 파일에 씌움
            //ret = Imgcodecs.imwrite( fileName, fileImg);

            Log.e(TAG, "선글라스 이미지 씌운 후");

            // setImageURI - Uri 경로에 따른 SDCard에 있는 이미지 파일을 로드하고, 이미지뷰 설정
//            try {
////            Uri uri = Uri.parse("file:///" + fileName);
//                Uri uri = Uri.parse(fileName);
//                preview_img.setImageURI(uri);
//            }catch (Exception e){
//                e.printStackTrace();
//            }

            Log.e(TAG, "이미지 뷰 설정후");
        }
    }

    // 사진 프리뷰를 본 후, 사진을 저장 취소할지
    public void isCancel() {
        final AlertDialog.Builder builder = new AlertDialog.Builder(PhotoPreviewActivity.this);

        builder.setTitle("저장 취소");
        builder.setMessage("사진 저장을 취소하시겠습니까?");
        builder.setPositiveButton("예", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                finish();
                Intent faceDetectionIntent = new Intent(PhotoPreviewActivity.this, FaceDetectionActivity.class);
                faceDetectionIntent.putExtra("isSave", "no");
                startActivity(faceDetectionIntent);
            }
        });
        builder.setNegativeButton("아니요",null);
        builder.show();
    }

    // 사진 프리뷰를 본 후, 사진을 저장할지
    public void isSave() {
        final AlertDialog.Builder builder = new AlertDialog.Builder(PhotoPreviewActivity.this);

        builder.setTitle("저장");
        builder.setMessage("사진을 저장하시겠습니까?");
        builder.setPositiveButton("예", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if ( ret ) {
                    Log.d(TAG, "take picture SUCCESS");

                    //Intent mediaScanIntent = new Intent( Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
                    //mediaScanIntent.setData(Uri.fromFile(file));
                    //sendBroadcast(mediaScanIntent);
                }
                else {
                    Log.d(TAG, "take picture FAIL");
                }

                finish();
                Intent faceDetectionIntent = new Intent(PhotoPreviewActivity.this, FaceDetectionActivity.class);
                faceDetectionIntent.putExtra("isSave", "yes");
                startActivity(faceDetectionIntent);
            }
        });
        builder.setNegativeButton("아니요",null);
        builder.show();
    }
}
