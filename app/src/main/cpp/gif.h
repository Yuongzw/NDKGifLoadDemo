//
// Created by Administrator on 2019/9/28.
//

#include "giflib/gif_lib.h"
#include <android/bitmap.h>
#ifndef NDKGIFLOADDEMO_GIF_H
#define NDKGIFLOADDEMO_GIF_H

#endif //NDKGIFLOADDEMO_GIF_H

extern "C"
int drawFrame(GifFileType *gif, AndroidBitmapInfo *bitmapInfo, int *pixels, int frameIndex, bool force_Dispose);
