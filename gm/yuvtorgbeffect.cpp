
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrTest.h"
#include "effects/GrYUVtoRGBEffect.h"
#include "SkBitmap.h"
#include "SkGr.h"
#include "SkGradientShader.h"

namespace skiagm {
/**
 * This GM directly exercises GrYUVtoRGBEffect.
 */
class YUVtoRGBEffect : public GM {
public:
    YUVtoRGBEffect() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("yuv_to_rgb_effect");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(334, 64);
    }

    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        // This is a GPU-specific GM.
        return kGPUOnly_Flag;
    }

    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        SkImageInfo info = SkImageInfo::MakeA8(24, 24);
        fBmp[0].allocPixels(info);
        fBmp[1].allocPixels(info);
        fBmp[2].allocPixels(info);
        unsigned char* pixels[3];
        for (int i = 0; i < 3; ++i) {
            pixels[i] = (unsigned char*)fBmp[i].getPixels();
        }
        int color[] = {0, 85, 170};
        const int limit[] = {255, 0, 255};
        const int invl[]  = {0, 255, 0};
        const int inc[]   = {1, -1, 1};
        for (int j = 0; j < 576; ++j) {
            for (int i = 0; i < 3; ++i) {
                pixels[i][j] = (unsigned char)color[i];
                color[i] = (color[i] == limit[i]) ? invl[i] : color[i] + inc[i];
            }
        }
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        GrRenderTarget* rt = canvas->internal_private_accessTopLayerRenderTarget();
        if (NULL == rt) {
            return;
        }
        GrContext* context = rt->getContext();
        if (NULL == context) {
            return;
        }

        GrTestTarget tt;
        context->getTestTarget(&tt);
        if (NULL == tt.target()) {
            SkDEBUGFAIL("Couldn't get Gr test target.");
            return;
        }

        GrDrawState* drawState = tt.target()->drawState();

        GrTexture* texture[3];
        texture[0] = GrLockAndRefCachedBitmapTexture(context, fBmp[0], NULL);
        texture[1] = GrLockAndRefCachedBitmapTexture(context, fBmp[1], NULL);
        texture[2] = GrLockAndRefCachedBitmapTexture(context, fBmp[2], NULL);
        if ((NULL == texture[0]) || (NULL == texture[1]) || (NULL == texture[2])) {
            return;
        }

        static const SkScalar kDrawPad = 10.f;
        static const SkScalar kTestPad = 10.f;

        SkRect renderRect = SkRect::MakeWH(SkIntToScalar(fBmp[0].width()),
                                           SkIntToScalar(fBmp[0].height()));
        renderRect.outset(kDrawPad, kDrawPad);

        SkScalar y = kDrawPad + kTestPad;
        SkScalar x = kDrawPad + kTestPad;

        const int indices[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};

        for (int i = 0; i < 6; ++i) {
            SkAutoTUnref<GrEffectRef> effect(
                        GrYUVtoRGBEffect::Create(texture[indices[i][0]],
                                                 texture[indices[i][1]],
                                                 texture[indices[i][2]]));
            if (effect) {
                SkMatrix viewMatrix;
                viewMatrix.setTranslate(x, y);
                drawState->reset(viewMatrix);
                drawState->setRenderTarget(rt);
                drawState->setColor(0xffffffff);
                drawState->addColorEffect(effect, 1);
                tt.target()->drawSimpleRect(renderRect);
            }
            x += renderRect.width() + kTestPad;
        }

        GrUnlockAndUnrefCachedBitmapTexture(texture[0]);
        GrUnlockAndUnrefCachedBitmapTexture(texture[1]);
        GrUnlockAndUnrefCachedBitmapTexture(texture[2]);
    }

private:
    SkBitmap fBmp[3];

    typedef GM INHERITED;
};

DEF_GM( return SkNEW(YUVtoRGBEffect); )
}

#endif
