package com.yuong.ndkgifloaddemo;

import android.graphics.Bitmap;

public class GifHelper {

    static {
        System.loadLibrary("native-lib");
    }

    private volatile long gifInfo;

    //加载Gif 图片
    public GifHelper(String gifPath) {
        gifInfo = openFile(gifPath);
    }

    //获取宽信息
    public synchronized int getWidth() {
        return getBitWidth(gifInfo);
    }
    //获取高信息
    public synchronized int getHeight() {
        return getBitHeight(gifInfo);
    }

    //图片的帧数
    public synchronized int getLength() {
        return getGifLength(gifInfo);
    }

    // 渲染（循环播放 时间间隔）
    public long renderFrame(Bitmap bitmap, int index){
        return renderFrameBit(bitmap, index, gifInfo);
    }

    private native long renderFrameBit(Bitmap bitmap, int index, long gifInfo);


    private native int getBitWidth(long gifInfo);

    private native int getBitHeight(long gifInfo);

    private native int getGifLength(long gifInfo);

    private native long openFile(String gifPath);

}
