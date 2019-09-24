package com.vegen.facedetection

import android.graphics.Bitmap


/**
 * author: huweijian
 * description: 人脸识别demo
 */
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
