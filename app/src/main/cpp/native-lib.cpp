#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>
#include <android/log.h>

#define TAG "JNI_TAG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

using namespace cv;

extern "C" {
JNIEXPORT jint JNICALL
Java_com_vegen_facedetection_FaceDetection_faceDetectionSaveInfo(JNIEnv *env, jobject instance, jobject mFaceBitmap);

JNIEXPORT void JNICALL
Java_com_vegen_facedetection_FaceDetection_loadCascade(JNIEnv *env, jobject instance, jstring filePath_);

// bitmap 转成 Mat
void bitmap2Mat(JNIEnv *env, Mat &mat, jobject bitmap);
// mat 转成 Bitmap
void mat2Bitmap(JNIEnv *env, Mat mat, jobject bitmap);
}


JNIEXPORT void bitmap2Mat(JNIEnv *env, Mat &mat, jobject bitmap) {
    // Mat 里面有个 type ： CV_8UC4 刚好对上我们的 Bitmap 中 ARGB_8888 , CV_8UC2 刚好对象我们的 Bitmap 中 RGB_565
    // 1. 获取 bitmap 信息
    AndroidBitmapInfo info;
    void* pixels;
    AndroidBitmap_getInfo(env,bitmap,&info);

    // 锁定 Bitmap 画布
    AndroidBitmap_lockPixels(env,bitmap,&pixels);
    // 指定 mat 的宽高和type  BGRA
    mat.create(info.height,info.width,CV_8UC4);

    if(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888){
        // 对应的 mat 应该是  CV_8UC4
        Mat temp(info.height,info.width,CV_8UC4,pixels);
        // 把数据 temp 复制到 mat 里面
        temp.copyTo(mat);
    } else if(info.format == ANDROID_BITMAP_FORMAT_RGB_565){
        // 对应的 mat 应该是  CV_8UC2
        Mat temp(info.height,info.width,CV_8UC2,pixels);
        // mat 是 CV_8UC4 ，CV_8UC2 -> CV_8UC4
        cvtColor(temp,mat,COLOR_BGR5652BGRA);
    }
    // todo 其他要自己去转

    // 解锁 Bitmap 画布
    AndroidBitmap_unlockPixels(env,bitmap);
}

void mat2Bitmap(JNIEnv *env, Mat mat, jobject bitmap) {
    // 1. 获取 bitmap 信息
    AndroidBitmapInfo info;
    void* pixels;
    AndroidBitmap_getInfo(env,bitmap,&info);

    // 锁定 Bitmap 画布
    AndroidBitmap_lockPixels(env,bitmap,&pixels);

    if(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888){// C4
        Mat temp(info.height,info.width,CV_8UC4,pixels);
        if(mat.type() == CV_8UC4){
            mat.copyTo(temp);
        }
        else if(mat.type() == CV_8UC2){
            cvtColor(mat,temp,COLOR_BGR5652BGRA);
        }
        else if(mat.type() == CV_8UC1){// 灰度 mat
            cvtColor(mat,temp,COLOR_GRAY2BGRA);
        }
    } else if(info.format == ANDROID_BITMAP_FORMAT_RGB_565){// C2
        Mat temp(info.height,info.width,CV_8UC2,pixels);
        if(mat.type() == CV_8UC4){
            cvtColor(mat,temp,COLOR_BGRA2BGR565);
        }
        else if(mat.type() == CV_8UC2){
            mat.copyTo(temp);

        }
        else if(mat.type() == CV_8UC1){// 灰度 mat
            cvtColor(mat,temp,COLOR_GRAY2BGR565);
        }
    }
    // todo 其他要自己去转

    // 解锁 Bitmap 画布
    AndroidBitmap_unlockPixels(env,bitmap);
}

CascadeClassifier cascadeClassifier;

jint JNICALL
Java_com_vegen_facedetection_FaceDetection_faceDetectionSaveInfo(JNIEnv *env, jobject instance, jobject bitmap) {
    // 检测人脸  , opencv 有一个非常关键的类是 Mat ，opencv 是 C 和 C++ 写的，只会处理 Mat , android里面是Bitmap
    // 1. Bitmap 转成 opencv 能操作的 C++ 对象 Mat , Mat 是一个矩阵
    Mat mat;
    bitmap2Mat(env,mat,bitmap);

    // 处理灰度 opencv 处理灰度图, 提高效率，一般所有的操作都会对其进行灰度处理
    Mat gray_mat;
    cvtColor(mat,gray_mat,COLOR_BGRA2GRAY);

    // 再次处理 直方均衡补偿
    Mat equalize_mat;
    equalizeHist(gray_mat,equalize_mat);

    // 识别人脸，也可以直接用 彩色图去做,识别人脸要加载人脸分类器文件
    std::vector<Rect> faces;
    cascadeClassifier.detectMultiScale(equalize_mat,faces,1.1,5);
    LOGE("人脸个数：%d",faces.size());
    if (faces.size() != 0) {
        for(Rect faceRect : faces) {
            // 在人脸部分花个图
            rectangle(mat,faceRect,Scalar(255,155,155),8);
            // 把 mat 我们又放到 bitmap 里面
            mat2Bitmap(env,mat,bitmap);
            // 保存人脸信息 Mat , 图片 jpg
            Mat face_info_mat(equalize_mat, faceRect);
            // 保存 face_info_mat
        }
    }

    // 保存人脸信息
    return 0;
}


JNIEXPORT void JNICALL
Java_com_vegen_facedetection_FaceDetection_loadCascade(JNIEnv *env, jobject instance, jstring filePath_) {
    const char *filePath = env->GetStringUTFChars(filePath_, 0);
    cascadeClassifier.load(filePath);
    LOGE("加载分类器文件成功");
    env->ReleaseStringUTFChars(filePath_, filePath);
}