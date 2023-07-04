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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_DRAWING_MOCK_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_DRAWING_MOCK_H

#include "core/components_ng/test/mock/rosen/testing_bitmap.h"
#include "core/components_ng/test/mock/rosen/testing_brush.h"
#include "core/components_ng/test/mock/rosen/testing_camera.h"
#include "core/components_ng/test/mock/rosen/testing_canvas.h"
#include "core/components_ng/test/mock/rosen/testing_color.h"
#include "core/components_ng/test/mock/rosen/testing_filter.h"
#include "core/components_ng/test/mock/rosen/testing_mask_filter.h"
#include "core/components_ng/test/mock/rosen/testing_path.h"
#include "core/components_ng/test/mock/rosen/testing_path_effect.h"
#include "core/components_ng/test/mock/rosen/testing_pen.h"
#include "core/components_ng/test/mock/rosen/testing_point.h"
#include "core/components_ng/test/mock/rosen/testing_point3.h"
#include "core/components_ng/test/mock/rosen/testing_shader_effect.h"
#include "core/components_ng/test/mock/rosen/testing_shadowflags.h"

namespace OHOS::Ace {
using RSCanvas = Testing::TestingCanvas;
using RSPen = Testing::TestingPen;
using RSBrush = Testing::TestingBrush;
using RSPath = Testing::TestingPath;
using RSShaderEffect = Testing::TestingShaderEffect;
using RSPathDashStyle = Testing::TestingPathDashStyle;
using RSPathDirection = Testing::TestingPathDirection;
using RSPathEffect = Testing::TestingPathEffect;
using RSFilter = Testing::TestingFilter;
using RSColor = Testing::TestingColor;
using RSMaskFilter = Testing::TestingMaskFilter;
using RSBlurType = Testing::BlurType;
using RSTileMode = Testing::TileMode;
using RSBitmap = Testing::TestingBitmap;
using RSBitmapFormat = Testing::BitmapFormat;
using RSColorType = Testing::ColorType;
using RSAlphaType = Testing::AlphaType;
using RSBlendMode = Testing::BlendMode;
using RSCamera3D = Testing::TestingCamera;
using RSMatrix = Testing::TestingMatrix;
using RSClipOp = Testing::ClipOp;
using RSPoint = Testing::TestingPoint;
using RSPoint3 = Testing::TestingPoint3;
using RSShadowFlags = Testing::TestingShadowFlags;
using RSRect = Testing::TestingRect;
using RSRoundRect = Testing::TestingRoundRect;
using RSColorQuad = uint32_t;
using RSScalar = float;
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_DRAWING_MOCK_H
