#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include "gif.h"
extern "C" {
#include "giflib/gif_lib.h"
#include "giflib/gif_lib_private.h"
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_yuong_ndkgifloaddemo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_yuong_ndkgifloaddemo_GifHelper_openFile(JNIEnv *env, jobject instance, jstring gifPath_) {
    const char *gifPath = env->GetStringUTFChars(gifPath_, 0);
    int err;//错误码
    //打开一个gif文件
    GifFileType *gifFile = DGifOpenFileName(gifPath, &err);
    //刷新gif
    err = DGifSlurp(gifFile);


    env->ReleaseStringUTFChars(gifPath_, gifPath);
    return reinterpret_cast<jlong>(gifFile);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_yuong_ndkgifloaddemo_GifHelper_getBitWidth(JNIEnv *env, jobject instance, jlong gifInfo) {
    return ((GifFileType *) gifInfo)->SWidth;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_yuong_ndkgifloaddemo_GifHelper_getBitHeight(JNIEnv *env, jobject instance, jlong gifInfo) {
    return ((GifFileType *) gifInfo)->SHeight;
}extern "C"
JNIEXPORT jint JNICALL
Java_com_yuong_ndkgifloaddemo_GifHelper_getGifLength(JNIEnv *env, jobject instance, jlong gifInfo) {

    return ((GifFileType *) gifInfo)->ImageCount;

}extern "C"
JNIEXPORT jlong JNICALL
Java_com_yuong_ndkgifloaddemo_GifHelper_renderFrameBit(JNIEnv *env, jobject instance,
                                                       jobject bitmap, jint index, jlong gifInfo) {
    GifFileType *gif_info = ((GifFileType *) gifInfo);
    //bitmap 转成 Mat矩阵
    void *pixels;
    AndroidBitmapInfo bitmapInfo;
    int ret;
    ret = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo) < 0;
    if (ret) {
        //ret 根据错误码进行处理
        return -1;
    }
    if (bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        return -1;
    }
    ret = AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0;
    if (ret) {
        return -1;
    }

    //图像渲染过程
    long delayTime = drawFrame(gif_info, &bitmapInfo, (int *)pixels, index, false);

    //转回bitmap
    AndroidBitmap_unlockPixels(env, bitmap);
    return delayTime;

}

#define argb(a, r, g, b) (((a) & 0xff) << 24) | (((b) & 0xff) << 16) | (((b) & 0xff) << 8) | ((r) & 0xff)
#define dispose(ext) (((ext) -> Bytes[0] & 0x1c) >> 2)
#define trans_index(ext) ((ext)->Bytes[3])
#define transparency(ext) ((ext)->Bytes[0] & 1)
#define delay(ext) (10 * ((ext) -> Bytes[2] <<8 | (ext) -> Bytes[1]))

int drawFrame(GifFileType *gif, AndroidBitmapInfo *bitmapInfo, int *pixels, int frame_index,
              bool force_dispose) {
    GifColorType *bg;
    GifColorType *color;
    SavedImage *frame;
    ExtensionBlock *ext = 0;
    GifImageDesc *frameInfo;
    ColorMapObject *colorMap;
    int *line;
    int width, height, x, y, j, loc, n, inc, p;
    int *px;
    width = gif->SWidth;
    height = gif->SHeight;
    frame = &(gif->SavedImages[frame_index]);
    frameInfo = &(frame->ImageDesc);
    if (frameInfo ->ColorMap) {
        colorMap = frameInfo ->ColorMap;
    } else{
        colorMap = gif -> SColorMap;
    }
    bg = &colorMap ->Colors[gif ->SBackGroundColor];
    for (j = 0; j < frame->ExtensionBlockCount; j++) {
        if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
            ext = &(frame->ExtensionBlocks[j]);
            break;
        }
    }
    px = pixels;
    if (ext && dispose(ext) == 1 && force_dispose && frame_index > 0) {
        //覆盖前一帧
        drawFrame(gif, bitmapInfo, pixels, frame_index - 1, true);
    } else if (ext && dispose(ext) == 2 && bg) {
        for (y = 0; y < height; y++) {
            line = px;
            for (x = 0; x < width; x++) {
                line[x] = argb(255, bg ->Red, bg->Green, bg ->Blue);
            }
        }
        px = (int *) ((char *) px + bitmapInfo ->stride);
    } else if (ext && dispose(ext) == 3 && frame_index > 1) {
        //被前一帧覆盖
        drawFrame(gif, bitmapInfo, pixels,frame_index - 2, true);
    }
    px = pixels;
    if (frameInfo ->Interlace) {
        n = 0;
        inc = 8;
        p = 0;
        px = (int *)((char *)px + bitmapInfo ->stride * frameInfo ->Top);
        for (y = frameInfo ->Top; y < frameInfo ->Top + frameInfo ->Height; y++) {
            for (x = frameInfo ->Left; y < frameInfo ->Left + frameInfo ->Width; x++) {
                loc = (y - frameInfo ->Top) * frameInfo->Width + (x - frameInfo->Left);
                if (ext && frame ->RasterBits[loc] == trans_index(ext) && transparency(ext)) {
                    continue;
                }
                color = (ext && frame ->RasterBits[loc] == trans_index(ext)) ? bg : &colorMap ->Colors[frame ->RasterBits[loc]];
                if (color) {
                    line[x] = argb(255, color ->Red, color ->Green, color ->Blue);
                }
            }
            px = (int *) ((char *) px + bitmapInfo->stride * inc);
            n += inc;
            if (n >= frameInfo ->Height) {
                n = 0;
                switch (p) {
                    case 0:
                        px = (int *)((char *) pixels + bitmapInfo ->stride * (4 + frameInfo ->Top));
                        inc = 8;
                        p++;
                        break;
                    case 1:
                        px = (int *)((char *) pixels + bitmapInfo ->stride * (2 + frameInfo ->Top));
                        inc = 4;
                        p++;
                        break;
                    case 2:
                        px = (int *)((char *) pixels + bitmapInfo ->stride * (1 + frameInfo ->Top));
                        inc = 2;
                        p++;
                        break;
                }
            }
        }
    } else{
        px = (int *)((char *) px + bitmapInfo ->stride * frameInfo ->Top);
        for (y = frameInfo ->Top; y < frameInfo ->Top + frameInfo -> Height; y++) {
            line = (int *)px;
            for (x = frameInfo ->Left; x < frameInfo ->Left + frameInfo -> Width; x++) {
                loc = (y - frameInfo -> Top) * frameInfo->Width + (x - frameInfo ->Left);
                if (ext && frame ->RasterBits[loc] == trans_index(ext) && transparency(ext)) {
                    continue;
                }
                color = (ext && frame ->RasterBits[loc] ==trans_index(ext) ? bg :&colorMap ->Colors[frame ->RasterBits[loc]]);
                if (color) {
                    line[x] = argb(255, color ->Red, color ->Green, color ->Blue);
                }
            }
            px = (int *) ((char *) px + bitmapInfo -> stride);
        }
    }
    return delay(ext);
}