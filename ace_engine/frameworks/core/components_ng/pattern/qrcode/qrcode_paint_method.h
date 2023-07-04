/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_QRCODE_QRCODE_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_QRCODE_QRCODE_PAINT_METHOD_H

#include "qrcodegen.hpp"

#include "base/memory/ace_type.h"
#include "base/utils/macros.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/node_paint_method.h"

namespace OHOS::Ace::NG {

class QRCodePaintMethod : public NodePaintMethod {
    DECLARE_ACE_TYPE(QRCodePaintMethod, NodePaintMethod)

    union BGRA {
        struct {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
            uint8_t alpha;
        } argb;

        uint32_t value;
    };

public:
    QRCodePaintMethod(float qrCodeSize) : qrCodeSize_(qrCodeSize) {}
    ~QRCodePaintMethod() override = default;
    CanvasDrawFunction GetContentDrawFunction(PaintWrapper* paintWrapper) override;

private:
    void Paint(RSCanvas& canvas, PaintWrapper* paintWrapper) const;
    RSBitmap CreateBitMap(
        int32_t width, const qrcodegen::QrCode& qrCode, const Color& color, const Color& backgroundColor) const;
    uint32_t ConvertColorFromHighToLow(const Color& color) const;

    float qrCodeSize_ = 0.0f;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_QRCODE_QRCODE_PAINT_METHOD_H
