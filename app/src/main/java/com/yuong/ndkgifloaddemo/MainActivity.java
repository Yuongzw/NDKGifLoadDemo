package com.yuong.ndkgifloaddemo;

import android.annotation.SuppressLint;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.ImageView;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    //1、使用GifLib
    //2、创建JNI 接口
    //3、使用 native 渲染图片
    //4、传回给 Java 显示

    ImageView imageView;
    GifHelper helper;
    Bitmap bitmap;
    private String path = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "demo.gif";
    int maxLength;
    int currentIndex = 0;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView = (ImageView) findViewById(R.id.imageView);

    }

    @SuppressLint("HandlerLeak")
    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (msg.what == 1) {
                long nextFrameTime = helper.renderFrame(bitmap, currentIndex);
                currentIndex ++;
                if (currentIndex >= maxLength) {
                    currentIndex = 0;
                }
                imageView.setImageBitmap(bitmap);
                handler.sendEmptyMessageDelayed(1, nextFrameTime);
            }
        }
    };

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public void play(View view) {
        helper = new GifHelper(path);
        int width = helper.getWidth();
        int height = helper.getHeight();
        maxLength = helper.getLength();
        bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        //渲染并获取延迟事件
        long delayTime = helper.renderFrame(bitmap, currentIndex);
        imageView.setImageBitmap(bitmap);
        currentIndex++;
        handler.sendEmptyMessageDelayed(1, delayTime);
    }
}
