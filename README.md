#### 1. 前言

本文讲述如何使用 OpenCV 进行简单的人脸识别开发，在此之前，需要配置好 OpenCV 和 NDK 环境。OpenCV 我使用的版本是：`OpenCV 3.4.6`，可在 [这里](https://github.com/opencv/opencv/releases) 下载。NDK 使用的版本是 `android-ndk-r16b`，可在 [这里](https://developer.android.google.cn/ndk/downloads/older_releases.html) 下载，对于因为使用其他版本导致的问题，本文不做叙述，因为使用其他版本，会遇到非常多的坑，敬请留意。

#### 2. NDK 配置

在 SDK Manage 中安装 LLDB 和 CMake 工具，下面的 NDK 先不安装，因为后面我们要配置下载好的 16b 版本。

![](https://user-gold-cdn.xitu.io/2019/9/25/16d641d7cc2a4ca0?w=729&h=435&f=png&s=51535)

然后在 Project Structure 中的 SDK Location 下面配置 NDK 路径


![](https://user-gold-cdn.xitu.io/2019/9/25/16d641eac754c116?w=836&h=701&f=png&s=44347)

#### 3. OpenCV 配置

需要在 SDK 中的 OpenCV-android-sdk\sdk\native\libs\armeabi-v7a 找到 libopencv_java3.so，在 OpenCV-android-sdk\sdk\native\jni\include 中找到 opencv 和 opencv2 文件夹，copy 到项目中去。


![](https://user-gold-cdn.xitu.io/2019/9/25/16d641fd33116c8f?w=397&h=594&f=png&s=39730)

在 CMakeLists.txt 中，配置好 opencv，配置如下 4 点

```
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
# 1.判断编译器类型,如果是gcc编译器,则在编译选项中加入c++11支持
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++11")
endif(CMAKE_COMPILER_IS_GNUCXX)

# 2.需要引入我们头文件,以这个配置的目录为基准
include_directories(../../main/jniLibs/include)

# 3.添加依赖 opencv.so 库
set(distribution_DIR ${CMAKE_SOURCE_DIR}/../../../../src/main/jniLibs)
add_library(
        opencv_java3
        SHARED
        IMPORTED)
set_target_properties(
        opencv_java3
        PROPERTIES IMPORTED_LOCATION
        ../../../../src/main/jniLibs/armeabi-v7a/libopencv_java3.so)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib opencv_java3
        # 4.加入该依赖库
        jnigraphics

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})
```

gradle 的配置

```
android {
    ...
    defaultConfig {
        ...
        externalNativeBuild {
            cmake {
                cppFlags "-std=c++11 -frtti -fexceptions"
                abiFilters 'armeabi-v7a'
                arguments "-DANDROID_STL=gnustl_static"
            }
            ndk {
                abiFilters 'armeabi-v7a'
            }
        }
    }
    
    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
        }
    }
}
```

#### 4. 具体核心实现

##### 4.1 Kotlin 上层实现

编写 FaceDetection 类，用于与 native 交互，所具有的方法是 **检测人脸并保存人脸信息** 以及 **加载人脸识别的分类器文件**


```
class FaceDetection {

    /**
     * 检测人脸并保存人脸信息
     * @param mFaceBitmap
     */
    external fun faceDetectionSaveInfo(mFaceBitmap: Bitmap): Int

    /**
     * 加载人脸识别的分类器文件
     * @param filePath
     */
    external fun loadCascade(filePath: String)

    companion object {

        init {
            System.loadLibrary("native-lib")
        }
    }

}
```

在 MainActivity 中编写功能代码


```
class MainActivity : AppCompatActivity() {

    private var mFaceBitmap: Bitmap? = null
    private var mFaceDetection: FaceDetection? = null
    private var mCascadeFile: File? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        mFaceBitmap = BitmapFactory.decodeResource(resources, R.drawable.timg1)
        face_image.setImageBitmap(mFaceBitmap)
        copyCascadeFile()
        mFaceDetection = FaceDetection()
        if (mCascadeFile != null) {
            mFaceDetection?.loadCascade(mCascadeFile!!.absolutePath)
        }
    }


    private fun copyCascadeFile() {
        try {
            // load cascade file from application resources
            var inputStream = resources.openRawResource(R.raw.lbpcascade_frontalface)
            val cascadeDir = getDir("cascade", Context.MODE_PRIVATE)
            mCascadeFile = File(cascadeDir, "lbpcascade_frontalface.xml")
            if (mCascadeFile!!.exists()) return
            val os = FileOutputStream(mCascadeFile)

            var buffer = ByteArray(4096)
            var bytesRead: Int = inputStream.read(buffer)
            while (bytesRead != -1) {
                os.write(buffer, 0, bytesRead)
                bytesRead = inputStream.read(buffer)
            }
            inputStream.close()
            os.close()
        } catch (e: IOException) {
            e.printStackTrace()
        }

    }

    fun faceDetection(view: View) {
        // 识别人脸，保存人脸特征信息
        mFaceBitmap?.let {
            mFaceDetection?.faceDetectionSaveInfo(it)
            face_image.setImageBitmap(it)
        }

    }

}

```


##### 4.2 native 层的代码实现

我们分析需求，结合 OpenCV 的特性，需要对传下来的 Bitmap 进行转换成 Mat，后面识别完画框需要将 Mat 转换为 Bitmap 回调给 Kotlin 层。Mat 里面有个 type ： CV_8UC4 刚好对上我们的 Bitmap 中 ARGB_8888 , CV_8UC2 刚好对象我们的 Bitmap 中 RGB_565。函数具体实现如下


```
// bitmap 转成 Mat
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

// mat 转成 Bitmap
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
```

人脸识别核心部分，可利用 OpenCV 对图片进行`灰度处理`和`直方均衡化`，这样可以提高识别率，识别到人脸后，我们需要在人脸上画一个框，以看出识别结果。


```
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
            // 在人脸部分画个图
            rectangle(mat,faceRect,Scalar(255,155,155),8);
            // 把 mat 我们又放到 bitmap 里面
            mat2Bitmap(env,mat,bitmap);
            
            // 保存人脸信息
            // 保存人脸信息 Mat , 图片 jpg
            Mat face_info_mat(equalize_mat, faceRect);
            // 保存 face_info_mat
        }
    }

    return 0;
}
```

加载分类器文件


```
JNIEXPORT void JNICALL
Java_com_vegen_facedetection_FaceDetection_loadCascade(JNIEnv *env, jobject instance, jstring filePath_) {
    const char *filePath = env->GetStringUTFChars(filePath_, 0);
    cascadeClassifier.load(filePath);
    LOGE("加载分类器文件成功");
    env->ReleaseStringUTFChars(filePath_, filePath);
}
```

#### 5. 识别成果

对一张女排团体照进行识别，发现识别率还是挺高的，效果如下图

原图 *（女排图片来源网络，侵删）*


![](https://user-gold-cdn.xitu.io/2019/9/25/16d6420df8bbdbb7?w=1080&h=2340&f=png&s=989604)

识别后效果图


![](https://user-gold-cdn.xitu.io/2019/9/25/16d64215c76d4f18?w=1080&h=2340&f=png&s=979543)

#### 6. 后话

OpenCV 的功能十分强大，后面将介绍更加详细的使用教程，以及后续会完善实时识别人脸，敬请期待。
欢迎 star。
