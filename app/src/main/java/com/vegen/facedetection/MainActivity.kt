package com.vegen.facedetection

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.widget.ImageView
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

/**
 * author: huweijian
 * description: 人脸识别demo
 */
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
