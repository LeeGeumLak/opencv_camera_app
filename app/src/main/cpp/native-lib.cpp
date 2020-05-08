#include <jni.h>
#include <opencv2/opencv.hpp>
#include <android/log.h>
#include <string>

using namespace cv;
using namespace std;

/*#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_lglcamera_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject *//* this *//*) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}*/

// image resize
float resize(Mat img_src, Mat &img_resize, int resize_width){
    float scale = resize_width / (float)img_src.cols ;
    if (img_src.cols > resize_width) {
        int new_height = cvRound(img_src.rows * scale);
        resize(img_src, img_resize, Size(resize_width, new_height));
    }
    else {
        img_resize = img_src;
    }
    return scale;
}

// cascade file copy and load
/*extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_lglcamera_FaceDetectionActivity_loadCascade(JNIEnv *env, jobject type, jstring cascadeFileName_){
    const char *nativeFileNameString = env->GetStringUTFChars(cascadeFileName_, 0);
    string baseDir("/storage/emulated/0/");
    baseDir.append(nativeFileNameString);
    const char *pathDir = baseDir.c_str();
    jlong ret = 0;
    ret = (jlong) new CascadeClassifier(pathDir);
    if (((CascadeClassifier *) ret)->empty()) {
        __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","CascadeClassifier로 로딩 실패 %s", nativeFileNameString);
    }
    else
        __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","CascadeClassifier로 로딩 성공 %s", nativeFileNameString);

    env->ReleaseStringUTFChars(cascadeFileName_, nativeFileNameString);

    //__android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ", "return %s", nativeFileNameString);

    return ret;
}

// face detect and circle
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lglcamera_FaceDetectionActivity_detect (JNIEnv *env, jobject type, jlong cascadeClassifier_face,
        jlong cascadeClassifier_eye, jlong mat_addr_Input, jlong mat_addr_Result) {

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 1);

    Mat &img_input = *(Mat *) mat_addr_Input;
    Mat &img_result = *(Mat *) mat_addr_Result;
    img_result = img_input.clone();

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 2);

    std::vector<Rect> faces;
    Mat img_gray;

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 3);

    cvtColor(img_input, img_gray, COLOR_BGR2GRAY);
    equalizeHist(img_gray, img_gray);
    Mat img_resize;
    float resizeRatio = resize(img_gray, img_resize, 640);

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 4);

    //-- Detect faces
    ((CascadeClassifier *) cascadeClassifier_face)->detectMultiScale( img_resize, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) );
    __android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ",
                        (char *) "face %d found ", faces.size());

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 5);

    for (int i = 0; i < faces.size(); i++) {
        double real_facesize_x = faces[i].x / resizeRatio;
        double real_facesize_y = faces[i].y / resizeRatio;
        double real_facesize_width = faces[i].width / resizeRatio;
        double real_facesize_height = faces[i].height / resizeRatio;
        Point center( real_facesize_x + real_facesize_width / 2, real_facesize_y + real_facesize_height/2);
        ellipse(img_result, center, Size( real_facesize_width / 2, real_facesize_height / 2), 0, 0, 360,
                Scalar(255, 192, 0), 4, 8, 0);
        Rect face_area(real_facesize_x, real_facesize_y, real_facesize_width,real_facesize_height);
        Mat faceROI = img_gray( face_area );
        std::vector<Rect> eyes;

        //-- In each face, detect eyes
        ((CascadeClassifier *) cascadeClassifier_eye)->detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CASCADE_SCALE_IMAGE, Size(30, 30) );
        for ( size_t j = 0; j < eyes.size(); j++ ) {
            Point eye_center( real_facesize_x + eyes[j].x + eyes[j].width/2, real_facesize_y + eyes[j].y + eyes[j].height/2 );
            int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
            circle( img_result, eye_center, radius, Scalar( 89, 89, 89 ), 4, 8, 0 );
        }
    }

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 6);

}*/
extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_lglcamera_MainActivity_loadCascade(JNIEnv *env, jobject type, jstring cascadeFileName_){
    const char *nativeFileNameString = env->GetStringUTFChars(cascadeFileName_, 0);
    string baseDir("/storage/emulated/0/");
    baseDir.append(nativeFileNameString);
    const char *pathDir = baseDir.c_str();
    jlong ret = 0;
    ret = (jlong) new CascadeClassifier(pathDir);
    if (((CascadeClassifier *) ret)->empty()) {
        __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","CascadeClassifier로 로딩 실패 %s", nativeFileNameString);
    }
    else
        __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","CascadeClassifier로 로딩 성공 %s", nativeFileNameString);

    env->ReleaseStringUTFChars(cascadeFileName_, nativeFileNameString);

    //__android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ", "return %s", nativeFileNameString);

    return ret;
}

// face detect and circle
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lglcamera_MainActivity_detect (JNIEnv *env, jobject type, jlong cascadeClassifier_face,
                                                         jlong cascadeClassifier_eye, jlong mat_addr_Input, jlong mat_addr_Result) {

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 1);

    Mat &img_input = *(Mat *) mat_addr_Input;
    Mat &img_result = *(Mat *) mat_addr_Result;
    img_result = img_input.clone();

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 2);

    std::vector<Rect> faces;
    Mat img_gray;

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 3);

    cvtColor(img_input, img_gray, COLOR_BGR2GRAY);
    equalizeHist(img_gray, img_gray);
    Mat img_resize;
    float resizeRatio = resize(img_gray, img_resize, 640);

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 4);

    //-- Detect faces
    ((CascadeClassifier *) cascadeClassifier_face)->detectMultiScale( img_resize, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) );
    __android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ",
                        (char *) "face %d found ", faces.size());

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 5);

    for (int i = 0; i < faces.size(); i++) {
        double real_facesize_x = faces[i].x / resizeRatio;
        double real_facesize_y = faces[i].y / resizeRatio;
        double real_facesize_width = faces[i].width / resizeRatio;
        double real_facesize_height = faces[i].height / resizeRatio;
        Point center( real_facesize_x + real_facesize_width / 2, real_facesize_y + real_facesize_height/2);
        ellipse(img_result, center, Size( real_facesize_width / 2, real_facesize_height / 2), 0, 0, 360,
                Scalar(255, 192, 0), 4, 8, 0);
        Rect face_area(real_facesize_x, real_facesize_y, real_facesize_width,real_facesize_height);
        Mat faceROI = img_gray( face_area );
        std::vector<Rect> eyes;

        //-- In each face, detect eyes
        ((CascadeClassifier *) cascadeClassifier_eye)->detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CASCADE_SCALE_IMAGE, Size(30, 30) );
        for ( size_t j = 0; j < eyes.size(); j++ ) {
            Point eye_center( real_facesize_x + eyes[j].x + eyes[j].width/2, real_facesize_y + eyes[j].y + eyes[j].height/2 );
            int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
            circle( img_result, eye_center, radius, Scalar( 89, 89, 89 ), 4, 8, 0 );
        }
    }

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 6);

}

// rgb -> gray
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lglcamera_MainActivity_ConvertRGBtoGray(JNIEnv *env, jobject thiz,
                                                         jlong mat_addr_input,
                                                         jlong mat_addr_result) {
    // TODO: implement ConvertRGBtoGray()
    Mat &matInput = *(Mat *)mat_addr_input;
    Mat &matResult = *(Mat *)mat_addr_result;

    cvtColor(matInput, matResult, COLOR_RGBA2GRAY);
}

// rgb -> hsv
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lglcamera_MainActivity_ConvertRGBtoHSV(JNIEnv *env, jobject thiz,
                                                         jlong mat_addr_input,
                                                         jlong mat_addr_result) {
    // TODO: implement ConvertRGBtoGray()
    Mat &matInput = *(Mat *)mat_addr_input;
    Mat &matResult = *(Mat *)mat_addr_result;

    cvtColor(matInput, matResult, COLOR_RGB2HSV);
}

