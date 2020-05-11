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

// 선글라스 이미지 오버레이
/*void overlayImage(const Mat &background, const Mat &foreground, Mat &output, Point2i location) {
    background.copyTo(output);


    // start at the row indicated by location, or at row 0 if location.y is negative.
    for (int y = max(location.y, 0); y < background.rows; ++y)
    {
        int fY = y - location.y; // because of the translation

        // we are done of we have processed all rows of the foreground image.
        if (fY >= foreground.rows)
            break;

        // start at the column indicated by location,

        // or at column 0 if location.x is negative.
        for (int x = max(location.x, 0); x < background.cols; ++x)
        {
            int fX = x - location.x; // because of the translation.

            // we are done with this row if the column is outside of the foreground image.
            if (fX >= foreground.cols)
                break;

            // determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
            double opacity =
                    ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3])

                    / 255.;


            // and now combine the background and foreground pixel, using the opacity,

            // but only if opacity > 0.
            for (int c = 0; opacity > 0 && c < output.channels(); ++c)
            {
                unsigned char foregroundPx =
                        foreground.data[fY * foreground.step + fX * foreground.channels() + c];
                unsigned char backgroundPx =
                        background.data[y * background.step + x * background.channels() + c];

                output.data[y*output.step + output.channels()*x + c] =
                        backgroundPx * (1. - opacity) + foregroundPx * opacity;
            }
        }
    }
}*/

void detectAndDraw( Mat& img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale, bool tryflip, Mat glasses );
void overlayImage(const Mat &background, const Mat &foreground,
                  Mat &output, Point2i location);

void detectAndDraw( Mat& img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale, bool tryflip, Mat glasses )
{

    Mat output2;
    img.copyTo(output2);

    double t = 0;
    vector<Rect> faces, faces2;
    const static Scalar colors[] =
            {
                    Scalar(255,0,0),
                    Scalar(255,128,0),
                    Scalar(255,255,0),
                    Scalar(0,255,0),
                    Scalar(0,128,255),
                    Scalar(0,255,255),
                    Scalar(0,0,255),
                    Scalar(255,0,255)
            };
    Mat gray, smallImg;

    cvtColor( img, gray, COLOR_BGR2GRAY );
    double fx = 1 / scale;
    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT );
    equalizeHist( smallImg, smallImg );

    t = (double)getTickCount();
    cascade.detectMultiScale( smallImg, faces,
                              1.1, 2, 0
                                      //|CASCADE_FIND_BIGGEST_OBJECT
                                      //|CASCADE_DO_ROUGH_SEARCH
                                      |CASCADE_SCALE_IMAGE,
                              Size(30, 30) );

    t = (double)getTickCount() - t;

    Mat result;

    printf( "detection time = %g ms\n", t*1000/getTickFrequency());
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Rect r = faces[i];
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;



        double aspect_ratio = (double)r.width/r.height;

        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            center.x = cvRound((r.x + r.width*0.5)*scale);
            center.y = cvRound((r.y + r.height*0.5)*scale);
            radius = cvRound((r.width + r.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
        else
            rectangle( img, Point(cvRound(r.x*scale), cvRound(r.y*scale)),
                       Point(cvRound((r.x + r.width-1)*scale), cvRound((r.y + r.height-1)*scale)),
                       color, 3, 8, 0);
        if( nestedCascade.empty() ){
            cout<<"nestedCascade.empty()"<<endl;
            continue;
        }

        smallImgROI = smallImg( r );
        nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
                                        1.1, 2, 0
                                                //|CASCADE_FIND_BIGGEST_OBJECT
                                                //|CASCADE_DO_ROUGH_SEARCH
                                                //|CASCADE_DO_CANNY_PRUNING
                                                |CASCADE_SCALE_IMAGE,
                                        Size(20, 20) );


        cout << nestedObjects.size() << endl;

        vector<Point> points;

        for ( size_t j = 0; j < nestedObjects.size(); j++ )
        {
            Rect nr = nestedObjects[j];
            center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
            center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
            radius = cvRound((nr.width + nr.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );

            Point p(center.x, center.y);
            points.push_back(p);
        }


        if ( points.size() == 2){

            Point center1 = points[0];
            Point center2 = points[1];

            if ( center1.x > center2.x ){
                Point temp;
                temp = center1;
                center1 = center2;
                center2 = temp;
            }


            int width = abs(center2.x - center1.x);
            int height = abs(center2.y - center1.y);

            if ( width > height){

                float imgScale = width/330.0;

                int w, h;
                w = glasses.cols * imgScale;
                h = glasses.rows * imgScale;

                int offsetX = 150 * imgScale;
                int offsetY = 160 * imgScale;

                Mat resized_glasses;
                resize( glasses, resized_glasses, cv::Size( w, h), 0, 0 );

                overlayImage(output2, resized_glasses, result, Point(center1.x-offsetX, center1.y-offsetY));
                output2 = result;
            }
        }
    }

    if ( result.empty() )
        imshow( "result", img );
    else
        imshow( "result", result );

}


void overlayImage(const Mat &background, const Mat &foreground,
                  Mat &output, Point2i location)
{
    background.copyTo(output);


    // start at the row indicated by location, or at row 0 if location.y is negative.
    for (int y = std::max(location.y, 0); y < background.rows; ++y)
    {
        int fY = y - location.y; // because of the translation

        // we are done of we have processed all rows of the foreground image.
        if (fY >= foreground.rows){
            break;
        }

        // start at the column indicated by location,

        // or at column 0 if location.x is negative.
        for (int x = std::max(location.x, 0); x < background.cols; ++x)
        {
            int fX = x - location.x; // because of the translation.

            // we are done with this row if the column is outside of the foreground image.
            if (fX >= foreground.cols){
                break;
            }

            // determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
            double opacity =
                    ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3])

                    / 255.;


            // and now combine the background and foreground pixel, using the opacity,

            // but only if opacity > 0.
            for (int c = 0; opacity > 0 && c < output.channels(); ++c)
            {
                unsigned char foregroundPx =
                        foreground.data[fY * foreground.step + fX * foreground.channels() + c];
                unsigned char backgroundPx =
                        background.data[y * background.step + x * background.channels() + c];
                output.data[y*output.step + output.channels()*x + c] =
                        backgroundPx * (1. - opacity) + foregroundPx * opacity;
            }
        }
    }
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
Java_com_example_lglcamera_MainActivity_LoadCascade(JNIEnv *env, jobject type, jstring cascadeFileName_){
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
Java_com_example_lglcamera_MainActivity_DetectAndDraw (JNIEnv *env, jobject type, jlong cascadeClassifier_face,
                                                         jlong cascadeClassifier_eye, jlong mat_addr_Input, jlong mat_addr_Result) {

    /*//__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 1);

    Mat &img_input = *(Mat *) mat_addr_Input;
    Mat &img_result = *(Mat *) mat_addr_Result;
    img_result = img_input.clone();

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 2);

    vector<Rect> faces;
    Mat img_gray;

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 3);

    cvtColor(img_input, img_gray, COLOR_BGR2GRAY);
    equalizeHist(img_gray, img_gray);
    Mat img_resize;
    float resizeRatio = resize(img_gray, img_resize, 640);

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 4);

    //-- Detect faces
    ((CascadeClassifier *) cascadeClassifier_face)->detectMultiScale( img_resize, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) );

    __android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ", (char *) "face %d found ", faces.size());

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 5);

    for (int i = 0; i < faces.size(); i++) {
        double real_facesize_x = faces[i].x / resizeRatio;
        double real_facesize_y = faces[i].y / resizeRatio;
        double real_facesize_width = faces[i].width / resizeRatio;
        double real_facesize_height = faces[i].height / resizeRatio;

        Point center( real_facesize_x + real_facesize_width / 2, real_facesize_y + real_facesize_height/2);

        ellipse(img_result, center, Size( real_facesize_width / 2, real_facesize_height / 2), 0, 0, 360, Scalar(255, 192, 0), 4, 8, 0);

        Rect face_area(real_facesize_x, real_facesize_y, real_facesize_width,real_facesize_height);

        Mat faceROI = img_gray( face_area );

        vector<Rect> eyes;

        //-- In each face, detect eyes
        ((CascadeClassifier *) cascadeClassifier_eye)->detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CASCADE_SCALE_IMAGE, Size(30, 30) );
        for ( size_t j = 0; j < eyes.size(); j++ ) {
            *//*Point eye_center( real_facesize_x + eyes[j].x + eyes[j].width/2, real_facesize_y + eyes[j].y + eyes[j].height/2 );

            int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
            circle( img_result, eye_center, radius, Scalar( 89, 89, 89 ), 4, 8, 0 );*//*
        }
    }

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 6);*/

    Mat &img_input = *(Mat *) mat_addr_Input;
    Mat &img_result = *(Mat *) mat_addr_Result;

    img_result = img_input;

    String glassesImage = "sunglasses.png";
    bool tryflip = false;
    double scale;
    scale = 1;

    Mat glasses = imread(glassesImage, IMREAD_UNCHANGED);

    CascadeClassifier cascade, nestedCascade;
    cascade = *(CascadeClassifier *)cascadeClassifier_face;
    nestedCascade = *(CascadeClassifier *)cascadeClassifier_eye;

    detectAndDraw(img_result, cascade, nestedCascade, scale, tryflip, glasses);
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

