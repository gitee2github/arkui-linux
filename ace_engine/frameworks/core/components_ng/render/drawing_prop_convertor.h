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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_DRAWING_PROP_CONVERTOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_DRAWING_PROP_CONVERTOR_H

#include "base/geometry/ng/rect_t.h"
#include "base/geometry/ng/point_t.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/render/drawing.h"
#include "core/pipeline/pipeline_base.h"

#ifndef ACE_UNITTEST
namespace OHOS::Ace {
RSPoint ToRSPoint(const NG::PointF& point);
RSColor ToRSColor(const Color& color);
RSColor ToRSColor(const LinearColor& color);
RSRect ToRSRect(const NG::RectF& rect);
RSPen::CapStyle ToRSCapStyle(const LineCap& lineCap);

rosen::TextDirection ToRSTextDirection(const TextDirection& txtDir);
rosen::TextAlign ToRSTextAlign(const TextAlign& align);
rosen::WordBreakType ToRSWordBreakType(const WordBreak& wordBreak);
rosen::TextStyle ToRSTextStyle(const RefPtr<PipelineBase>& context, const TextStyle& textStyle);
} // namespace OHOS::Ace
#else
#include "core/components_ng/render/drawing_prop_convertor_mock.h"
#endif
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_DRAWING_PROP_CONVERTOR_H
