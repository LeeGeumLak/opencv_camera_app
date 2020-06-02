#include <jni.h>
#include <opencv2/opencv.hpp>
#include <android/log.h>
#include <string>

using namespace cv;
using namespace std;

float imgResize(Mat img_src, Mat &img_resize, int resize_width){
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

//void detectAndSunglasses( Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestedCascade, double scale, bool tryflip, Mat glasses );
void overlayImage(const Mat &background, const Mat &foreground, Mat &output, Point2i location);
Mat putMask( Mat src, Mat mask, Point center, Size face_size );

// 메인 액티비티에서 사용, 메인 액티비티에서 인자로 카메라로 들어오는 화면(mat_addr_input)과 화면으로 내보내는 화면(mat_addr_result), 얼굴위에 씌울 선글라스 이미지(mat_addr_sunglasses),
// 그리고 얼굴과 눈을 인식하는 cascade 파일을 받는다.
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lglcamera_activity_MainActivity_DetectAndSunglasses(JNIEnv *env, jobject type, jlong mat_addr_input, jlong mat_addr_result, jlong mat_addr_sunglasses,
                                                                     jlong cascadeClassifier_face, jlong cascadeClassifier_eye) {
    //bool tryflip = false;
    //double scale = 1;

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ", "DetectAndSunglasses 진입 %d", 1);

    // long 타입으로 받은 선글라스 이미지를 Mat 타입으로 캐스팅
    Mat &glasses = *(Mat *) mat_addr_sunglasses;
    //String glassesName = "sunglasses_black.png";
    //glasses = imread(glassesName, IMREAD_UNCHANGED);

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","선글라스 이미지 받아옴 %d", 1);

    // long 타입으로 받은 input과 output 화면을 Mat 타입으로 캐스팅
    Mat &img_input = *(Mat *) mat_addr_input;
    Mat &img_result = *(Mat *) mat_addr_result;

    // img_result는 null 상태이므로 img_input 의 정보를 복제
    img_result = img_input.clone();

    // 원본에 영향을 주지 않고, 영상처리를 하기 위해 output2 생성, img_input 의 내용을 복사함
    Mat output2;
    img_input.copyTo(output2);

    vector<Rect> faces;//, faces2; // 탐지한 얼굴 정보(위치) 저장, faces2

    // 디스플레이의 색상 정보 Scalar 로 미리 저장
    /*const static Scalar colors[] = {
            Scalar(255,0,0),
            Scalar(255,128,0),
            Scalar(255,255,0),
            Scalar(0,255,0),
            Scalar(0,128,255),
            Scalar(0,255,255),
            Scalar(0,0,255),
            Scalar(255,0,255)
    };*/

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","cvt 직전 %d", 1);

    // 히스토그램 평활화를 위해 img_input 화면을 grayscale로 변환한다.
    Mat img_gray;
    cvtColor(img_input, img_gray, COLOR_BGR2GRAY);

    // 히스토그램 평활화 equalizeHist(원본 영상, 히스토그램 평활화가 저장될 Mat 이름)
    // 원본 영상을 gray-scale로 변환 후, 한쪽으로 치우칠 수 있는 명암을 고르게 분포시켜주는 작업
    equalizeHist( img_gray, img_gray );

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","cvt 직후 %d", 1);

    //Mat smallImg;
    //double resizeRatio = imgResize(img_gray, img_resize, 640);
    //double fx = 1 / scale;
    //resize( img_gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT );

    // 위의 히스토그램 평활화까지 마친 이미지를 적당한 사이즈로 resize 함
    Mat img_resize;
    resize( img_gray, img_resize, Size(), 1, 1, INTER_LINEAR_EXACT );

    //equalizeHist( smallImg, smallImg );

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","detectMultiScale 직전 %d", 1);

    // 얼굴을 인식하게 하는 cascade 파일을 이용하여 화면속의 얼굴(들)을 인식하여, faces 벡터 컨테이너에 저장한다.
    // vector : 대표적인 시퀀스 컨테이너로 배열과 비슷함(원소가 연속하게 저장됨)
    ((CascadeClassifier *) cascadeClassifier_face)->detectMultiScale( img_resize, faces,
                                                                      1.1, 2, 0
                                                                              //|CASCADE_FIND_BIGGEST_OBJECT
                                                                              //|CASCADE_DO_ROUGH_SEARCH
                                                                              //|CASCADE_DO_CANNY_PRUNING
                                                                              |CASCADE_SCALE_IMAGE,
                                                                      Size(30, 30) );

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","얼굴 검출 for문 직전 %d", 1);

    // 얼굴에 선글라스를 씌우는 overlayImage 메서드에 결과값으로 넘길 인자값
    Mat result;

    // 위에서 찾은 얼굴의 개수만큼 반복문 실행( faces.size() : 인식한 얼굴의 개수 )
    for ( size_t i = 0; i < faces.size(); i++ ) {
        // 찾은 얼굴의 x 값과 y 값 계산
        //double real_facesize_x = faces[i].x / resizeRatio;
        //double real_facesize_y = faces[i].y / resizeRatio;

        // 얼굴의 높이와 너비 계산
        //double real_facesize_width = faces[i].width / resizeRatio;
        //double real_facesize_height = faces[i].height / resizeRatio;

        // 인식한 얼굴을 저장한 faces 벡터의 i 번째 인자값을 r 로 지정
        Rect r = faces[i];
        Mat faceROI; //smallImgROI;

        // 인식할 눈의 위치를 저장할 벡터 컨테이너
        vector<Rect> eyes;

        __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","원 정의 직전 %d", 1);

        // 원 정의
        Point center;
        //Point center( cvRound(real_facesize_x + real_facesize_width / 2), cvRound(real_facesize_y + real_facesize_height / 2));
        //Scalar color = colors[i%8];
        int radius;

        //ellipse(img_result, center, Size(cvRound(real_facesize_width / 2), cvRound(real_facesize_height / 2)), 0, 0, 360, Scalar(255, 192, 0), 4, 8, 0);

        // 얼굴의 너비와 높이의 비율을 이용하여, 인식한 얼굴에 원을 그림
        double aspect_ratio = (double)r.width/r.height;
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 ) {
            center.x = cvRound(r.x + r.width*0.5);
            center.y = cvRound(r.y + r.height*0.5);
            radius = cvRound((r.width + r.height)*0.25);
            circle( img_result, center, radius, Scalar(255, 192, 0), 4, 8, 0 );
        }
        else {
            rectangle( img_result, Point(cvRound(r.x), cvRound(r.y)),
                       Point(cvRound((r.x + r.width-1)), cvRound((r.y + r.height-1))), Scalar(255, 192, 0), 4, 8, 0);
        }

        /*if( 0.75 < aspect_ratio && aspect_ratio < 1.3 ) {
            center.x = cvRound((r.x + r.width*0.5)*scale);
            center.y = cvRound((r.y + r.height*0.5)*scale);
            radius = cvRound((r.width + r.height)*0.25*scale);
            circle( img_input, center, radius, color, 3, 8, 0 );
        }
        else {
            rectangle(img_input, Point(cvRound(r.x * scale), cvRound(r.y * scale)),
                      Point(cvRound((r.x + r.width - 1) * scale), cvRound((r.y + r.height - 1) * scale)), color, 3, 8, 0);
        }*/

        //smallImgROI = smallImg( r );
        //Rect faces_area(cvRound(real_facesize_x), cvRound(real_facesize_y), cvRound(real_facesize_width), cvRound(real_facesize_height));
        faceROI = img_gray( r );
        //faceROI = img_gray( faces[i] );

        __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","눈 detectMultiScale 직전 %d", 1);

        // cascade 파일을 이용하여, 인식한 얼굴의 눈을 인식하여 eyes 벡터 컨테이너에 저장
        ((CascadeClassifier *) cascadeClassifier_eye)->detectMultiScale( faceROI, eyes,
                                                                         1.1, 2, 0
                                                                                 //|CASCADE_FIND_BIGGEST_OBJECT
                                                                                 //|CASCADE_DO_ROUGH_SEARCH
                                                                                 //|CASCADE_DO_CANNY_PRUNING
                                                                                 |CASCADE_SCALE_IMAGE,
                                                                         Size(15, 15) );

        vector<Point> points;

        __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","눈 검출 반복문 직전 %d", 1);

        // eyes.size() : 검출한 눈의 개수, 인식한 눈의 개수만큼 반복문 실행
        for ( size_t j = 0; j < eyes.size(); j++ ) {
            //Point eye_center(cvRound(real_facesize_x + eyes[j].x + eyes[j].width/2), cvRound(real_facesize_y + eyes[j].y + eyes[j].height/2));

            //int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);

            // 인식한 눈의 좌표를 정의하고, 원을 그림
            Rect nr = eyes[j];
            center.x = cvRound((r.x + nr.x + nr.width*0.5));
            center.y = cvRound((r.y + nr.y + nr.height*0.5));
            radius = cvRound((nr.width + nr.height)*0.25);

            // 눈이 인식되면 원을 그리는 부분
            // img_result = 원이 그려질 이미지
            // eye_center = 원의 중심 좌표
            // radius = 원의 반지름
            // Scalar = 원의 색깔 BRG 순서 / 이 색의 원을 그려준다
            circle( img_result, center, radius, Scalar( 89, 89, 89 ), 4, 8, 0 );

            /*Rect nr = eyes[j];
            center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
            center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
            radius = cvRound((nr.width + nr.height)*0.25*scale);
            circle( img_input, center, radius, color, 3, 8, 0 );*/

            __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","눈에 circle 직후 %d", 1);

            //push_back : vector의 끝에 요소를 추가, 각 얼굴마다의 1개 이상의 눈 위치(좌표)를 points에 저장
            Point p(center.x, center.y);
            points.push_back( p );
        }

        __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","points.size() ==2 직전 %d", 1);

        // 인식한 눈이 2개일 때만( 1개만 인식될 때가 있는데, 그때는 예외처리함 --> 2개의 눈 모두 인식되어야 선글라스를 씌울 수 있음 )
        if ( points.size() == 2){

            __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","points.size() ==2 직후 %d", 1);

            Point center1 = points[0]; // 눈 1
            Point center2 = points[1]; // 눈 2

            // 두 눈의 x 좌표를 따져 화면상 왼쪽의 눈을 가려냄
            if ( center1.x > center2.x ) {
                Point temp;
                temp = center1;
                center1 = center2;
                center2 = temp;
            }

            // 두 눈의 사이의 거리를 인식
            int width = abs(center2.x - center1.x);
            int height = abs(center2.y - center1.y);

            __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","width > height 직전 %d", 1);

            if ( width > height) {

                __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","width > height 직후 %d", 1);

                // 두 눈의 사이의 거리에 따라 선글라스 이미지를 resize 하기 위해 그 크기값을 구함
                // 선글라스를 얼마나 resize 할지의 크기 값
                double imgScale = width/200.0;

                int w, h;
                w = cvRound(glasses.cols * imgScale);
                h = cvRound(glasses.rows * imgScale);

                int offsetX = cvRound(150 * imgScale);
                int offsetY = cvRound(160 * imgScale);

                Mat resized_glasses;

                __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","resize 직전 %d", 1);

                // TODO : resize() 함수에서 에러 발생 (아래는 에러 내용) --> 해결 (이미지를 제대로 못 받아와서 생긴 에러)
                // E/cv::error(): OpenCV(4.3.0) Error: Assertion failed (!ssize.empty()) in resize, file /build/master_pack-android/opencv/modules/imgproc/src/resize.cpp, line 3929
                // E/libc++abi: terminating with uncaught exception of type cv::Exception: OpenCV(4.3.0)
                //                              /build/master_pack-android/opencv/modules/imgproc/src/resize.cpp:3929: error: (-215:Assertion failed) !ssize.empty() in function 'resize'
                //resize( glasses, resized_glasses, Size( w, h), 0, 0);

                // 인자로 받은 안경 이미지를 얼굴 크기에 맞게 resize 함
                resize( glasses, resized_glasses, cv::Size( w, h), 0, 0 );

                __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","오버레이 직전 %d", 1);

                // resize한 얼굴 이미지와 선글라스 이미지를 인자로 입력 --> 얼굴 위에 선글라스를 오버레이하여 result 이미지에 입력 (포인터를 이용하므로 따로 return 받지는 않음)
                overlayImage(output2, resized_glasses, result, Point(center1.x-offsetX, center1.y-offsetY));

                __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","오버레이 직후 %d", 1);

                // result 값을 output2에 복제
                output2 = result.clone();
            }
        }
    }
    // output2 값을 img_result에 복제 : 포인터를 이용하므로 img_result에 최종 이미지를 저장하면, 메인 액티비티에서 이 이미지를 사용할 수 있다.
    output2.copyTo(img_result);
}

// 선글라스 이미지 오버레이
void overlayImage(const Mat &background, const Mat &foreground, Mat &output, Point2i location) {
    // background 값을 output에 복사 --> output 은 null 상태이므로, 미리 원본 사진을 넣어둠
    background.copyTo(output);

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","오버레이 반복문 시작 직전", 1);

    // start at the row indicated by location, or at row 0 if location.y is negative.
    for (int y = max(location.y, 0); y < background.rows; ++y) {
        int fY = y - location.y; // because of the translation

        // we are done of we have processed all rows of the foreground image.
        if (fY >= foreground.rows) {
            break;
        }

        // start at the column indicated by location, or at column 0 if location.x is negative.
        for (int x = max(location.x, 0); x < background.cols; ++x) {
            int fX = x - location.x; // because of the translation.

            // we are done with this row if the column is outside of the foreground image.
            if (fX >= foreground.cols) {
                break;
            }

            // determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
            double opacity = ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3]) / 255.;

            // and now combine the background and foreground pixel, using the opacity, but only if opacity > 0.
            for (int c = 0; opacity > 0 && c < output.channels(); ++c) {
                unsigned char foregroundPx =
                        foreground.data[fY * foreground.step + fX * foreground.channels() + c];
                unsigned char backgroundPx =
                        background.data[y * background.step + x * background.channels() + c];
                output.data[y*output.step + output.channels()*x + c] = backgroundPx * (1. - opacity) + foregroundPx * opacity;
            }
        }
    }
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","오버레이 반복문 끝 직후", 1);
}

// 메인 액티비티에서 사용, 메인 액티비티에서 인자로 카메라로 들어오는 화면(mat_addr_input)과 화면으로 내보내는 화면(mat_addr_result), 얼굴위에 씌울 마스크 이미지(mat_addr_sunglasses),
// 그리고 얼굴을 인식하는 cascade 파일을 받는다.
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lglcamera_activity_MainActivity_DetectAndMask(JNIEnv *env, jobject type, jlong mat_addr_input, jlong mat_addr_result, jlong mat_addr_mask,
                                                                     jlong cascadeClassifier_face) {

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ", "DetectAndMask 진입 %d", 1);

    // long 타입으로 받은 선글라스 이미지를 Mat 타입으로 캐스팅
    Mat &mask = *(Mat *) mat_addr_mask;

    // long 타입으로 받은 input과 output 화면을 Mat 타입으로 캐스팅
    Mat &img_input = *(Mat *) mat_addr_input;
    Mat &img_result = *(Mat *) mat_addr_result;

    // img_result는 null 상태이므로 img_input 의 정보를 복제
    img_result = img_input.clone();

    // 원본에 영향을 주지 않고, 영상처리를 하기 위해 output2 생성, img_input 의 내용을 복사함
    Mat output2;
    img_input.copyTo(output2);

    vector<Rect> faces;//, faces2; // 탐지한 얼굴 정보(위치) 저장, faces2

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","cvt 직전 %d", 1);

    // 히스토그램 평활화를 위해 img_input 화면을 grayscale로 변환한다.
    Mat img_gray;
    cvtColor(img_input, img_gray, COLOR_BGR2GRAY);

    // 히스토그램 평활화 equalizeHist(원본 영상, 히스토그램 평활화가 저장될 Mat 이름)
    // 원본 영상을 gray-scale로 변환 후, 한쪽으로 치우칠 수 있는 명암을 고르게 분포시켜주는 작업
    equalizeHist( img_gray, img_gray );

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","cvt 직후 %d", 1);

    // 위의 히스토그램 평활화까지 마친 이미지를 적당한 사이즈로 resize 함
    Mat img_resize;
    resize( img_gray, img_resize, Size(), 1, 1, INTER_LINEAR_EXACT );

    //equalizeHist( smallImg, smallImg );

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","detectMultiScale 직전 %d", 1);

    // 얼굴을 인식하게 하는 cascade 파일을 이용하여 화면속의 얼굴(들)을 인식하여, faces 벡터 컨테이너에 저장한다.
    // vector : 대표적인 시퀀스 컨테이너로 배열과 비슷함(원소가 연속하게 저장됨)
    ((CascadeClassifier *) cascadeClassifier_face)->detectMultiScale( img_resize, faces,
                                                                      1.1, 3, 0
                                                                              //|CASCADE_FIND_BIGGEST_OBJECT
                                                                              //|CASCADE_DO_ROUGH_SEARCH
                                                                              //|CASCADE_DO_CANNY_PRUNING
                                                                              |CASCADE_SCALE_IMAGE,
                                                                      Size(30, 30) );

    __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","얼굴 검출 for문 직전 %d", 1);

    // 얼굴에 마스크를 씌우는 메서드에 넘길 인자값
    Mat image = img_input;

    // 위에서 찾은 얼굴의 개수만큼 반복문 실행( faces.size() : 인식한 얼굴의 개수 )
    for ( size_t i = 0; i < faces.size(); i++ ) {

        /*// 인식한 얼굴을 저장한 faces 벡터의 i 번째 인자값을 r 로 지정
        Rect r = faces[i];
        Mat faceROI; //smallImgROI;

        __android_log_print(ANDROID_LOG_DEBUG, "native-lib :: ","원 정의 직전 %d", 1);

        // 원 정의
        Point center;
        int radius;

        // 얼굴의 너비와 높이의 비율을 이용하여, 인식한 얼굴에 원을 그림
        double aspect_ratio = (double)r.width/r.height;
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 ) {
            center.x = cvRound(r.x + r.width*0.5);
            center.y = cvRound(r.y + r.height*0.5);
            radius = cvRound((r.width + r.height)*0.25);
            circle( img_result, center, radius, Scalar(255, 192, 0), 4, 8, 0 );
        }
        else {
            rectangle( img_result, Point(cvRound(r.x), cvRound(r.y)),
                       Point(cvRound((r.x + r.width-1)), cvRound((r.y + r.height-1))), Scalar(255, 192, 0), 4, 8, 0);
        }*/
        double min_face_size = faces[i].width * 0.7;
        double max_face_size = faces[i].width * 1.5;
        Point center( faces[i].x + faces[i].width * 0.5, faces[i].y + faces[i].height * 0.5 );
        output2 = putMask(image, mask, center, Size( faces[i].width, faces[i].height ) );

    }

    // output2 값을 img_result에 복제 : 포인터를 이용하므로 img_result에 최종 이미지를 저장하면, 메인 액티비티에서 이 이미지를 사용할 수 있다.
    output2.copyTo(img_result);
}

Mat putMask( Mat src, Mat mask, Point center, Size face_size ) {
    Mat mask1, src1;
    resize( mask, mask1, face_size );

    /// ROI selection
    Rect roi( center.x - face_size.width / 2, center.y - face_size.width / 2, face_size.width, face_size.width );
    src(roi).copyTo(src1);

    /// to make the white region transparent
    Mat mask2, m, m1;
    cvtColor( mask1, mask2, COLOR_BGR2GRAY );
    threshold( mask2, mask2, 230, 255, THRESH_BINARY_INV );

    vector<Mat> maskChannels( 3 ), result_mask( 3 );
    split( mask1, maskChannels );
    bitwise_and( maskChannels[0], mask2, result_mask[0] );
    bitwise_and( maskChannels[1], mask2, result_mask[1] );
    bitwise_and( maskChannels[2], mask2, result_mask[2] );
    merge( result_mask, m );

    mask2 = 255 - mask2;
    vector<Mat> srcChannels( 3 );
    split(src1, srcChannels);
    bitwise_and( srcChannels[0], mask2, result_mask[0] );
    bitwise_and( srcChannels[1], mask2, result_mask[1] );
    bitwise_and( srcChannels[2], mask2, result_mask[2] );
    merge( result_mask, m1 );

    addWeighted( m, 1, m1, 1, 0, m1);
    m1.copyTo( src( roi ) );

    return src;
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
/*extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_lglcamera_activity_PhotoPreviewActivity_LoadCascade(JNIEnv *env, jobject type, jstring cascadeFileName_){
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
}*/

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

// cascade file copy and load
extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_lglcamera_activity_MainActivity_LoadCascade(JNIEnv *env, jobject type, jstring cascadeFileName_){
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

// 얼굴을 인식하고, 각 얼굴마다의 눈 인식하여 원으로 표시
/*extern "C"
JNIEXPORT void JNICALL
Java_com_example_lglcamera_activity_MainActivity_DetectAndDraw (JNIEnv *env, jobject type, jlong cascadeClassifier_face,
                                                         jlong cascadeClassifier_eye, jlong mat_addr_Input, jlong mat_addr_Result) {

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 1);

    Mat &img_input = *(Mat *) mat_addr_Input;
    Mat &img_result = *(Mat *) mat_addr_Result;
    img_result = img_input.clone();

    Mat output2;
    img_result.copyTo(output2);

    //---------------------------------------------------------------------------------
    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 2);

    vector<Rect> faces; // 탐지한 얼굴 정보(위치) 저장

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 3);

    Mat img_gray;
    cvtColor(img_input, img_gray, COLOR_BGR2GRAY);
    equalizeHist(img_gray, img_gray);

    Mat img_resize;
    float resizeRatio = imgResize(img_gray, img_resize, 640);

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 4);

    // 인풋 이미지 분류 (얼굴 -> 눈)
    //-- Detect faces
    ((CascadeClassifier *) cascadeClassifier_face)->detectMultiScale( img_resize, faces,
                                                                      1.1, 2, 0
                                                                              //|CASCADE_FIND_BIGGEST_OBJECT
                                                                              //|CASCADE_DO_ROUGH_SEARCH
                                                                              //|CASCADE_DO_CANNY_PRUNING
                                                                              |CASCADE_SCALE_IMAGE,
                                                                      Size(30, 30) );

    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ", (char *) "face %d found ", faces.size());
    //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 5);

    // faces.size() 가 0 이상이면 얼굴을 감지한 것으로 간주하여 진행
    for (int i = 0; i < faces.size(); i++) {
        //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 6);


        // 찾은 얼굴의 x 값과 y 값 계산
        double real_facesize_x = faces[i].x / resizeRatio;
        double real_facesize_y = faces[i].y / resizeRatio;

        // 얼굴의 높이와 너비 계산
        double real_facesize_width = faces[i].width / resizeRatio;
        double real_facesize_height = faces[i].height / resizeRatio;

        Point center( real_facesize_x + real_facesize_width / 2, real_facesize_y + real_facesize_height/2);

        // ellipse() : 타원 그리기 함수
        // img_result = 원이 그려질 이미지
        // center = 원의 중심 좌표
        // Size = 원의 반지름
        // Scalar = 원의 색깔 BRG 순서 / 해당하는 색의 원을 그려준다
        ellipse(img_result, center, Size( real_facesize_width / 2, real_facesize_height / 2), 0, 0, 360, Scalar(255, 192, 0), 4, 8, 0);

        // Rect 구조체 : 사각형의 너비, 높이, 위치를 지정
        Rect face_area(real_facesize_x, real_facesize_y, real_facesize_width,real_facesize_height);

        Mat faceROI = img_gray( face_area );

        vector<Rect> eyes; // 탐지한 눈의 정보(위치) 저장

        //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 7);

        //-- In each face, detect eyes
        ((CascadeClassifier *) cascadeClassifier_eye)->detectMultiScale( faceROI, eyes,
                                                                         1.1, 2, 0
                                                                                 //|CASCADE_FIND_BIGGEST_OBJECT
                                                                                 //|CASCADE_DO_ROUGH_SEARCH
                                                                                 //|CASCADE_DO_CANNY_PRUNING
                                                                                 |CASCADE_SCALE_IMAGE,
                                                                         Size(15, 15) );

        //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 8);

        // eyes.size() : 검출한 눈의 개수
        for ( size_t j = 0; j < eyes.size(); j++ ) {
            //__android_log_print(ANDROID_LOG_DEBUG, (char *) "native-lib :: ","%d", 9);

            Point eye_center( real_facesize_x + eyes[j].x + eyes[j].width/2, real_facesize_y + eyes[j].y + eyes[j].height/2 );

            int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );

            // 눈이 인식되면 원을 그리는 부분
            // img_result = 원이 그려질 이미지
            // eye_center = 원의 중심 좌표
            // radius = 원의 반지름
            // Scalar = 원의 색깔 BRG 순서 / 해당 색의 원을 그려준다
            circle( img_result, eye_center, radius, Scalar( 89, 89, 89 ), 4, 8, 0 );
        }
    }
}*/

// rgb -> gray
// mat_addr_input 은 rgba 이미지, mat_addr_result 는 null 상태
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lglcamera_activity_MainActivity_ConvertRGBtoGray(JNIEnv *env, jobject thiz,
                                                         jlong mat_addr_input,
                                                         jlong mat_addr_result) {
    // TODO: implement ConvertRGBtoGray()
    // long 타입인 이미지 정보를 Mat 타입으로 캐스팅
    Mat &matInput = *(Mat *)mat_addr_input;
    Mat &matResult = *(Mat *)mat_addr_result;

    // rgba 이미지인 matInput을 grayscale로 변환하여 matResult에 입력
    cvtColor(matInput, matResult, COLOR_RGBA2GRAY);
}

// rgb -> hsv
// mat_addr_input 은 rgba 이미지, mat_addr_result 는 null 상태
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lglcamera_activity_MainActivity_ConvertRGBtoHSV(JNIEnv *env, jobject thiz,
                                                         jlong mat_addr_input,
                                                         jlong mat_addr_result) {
    // TODO: implement ConvertRGBtoGray()
    // long 타입인 이미지 정보를 Mat 타입으로 캐스팅
    Mat &matInput = *(Mat *)mat_addr_input;
    Mat &matResult = *(Mat *)mat_addr_result;

    // rgba 이미지인 matInput을 hsv로 변환하여 matResult에 입력
    cvtColor(matInput, matResult, COLOR_RGB2HSV);
}

