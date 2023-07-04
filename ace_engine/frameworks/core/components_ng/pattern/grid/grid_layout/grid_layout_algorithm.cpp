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

#include "core/components_ng/pattern/grid/grid_layout/grid_layout_algorithm.h"

#include <vector>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/grid/grid_item_layout_property.h"
#include "core/components_ng/pattern/grid/grid_layout_property.h"
#include "core/components_ng/pattern/grid/grid_utils.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_utils.h"

namespace OHOS::Ace::NG {
namespace {
void OffsetByAlign(
    const LayoutConstraintF& layoutConstraint, float rowLen, float colLen, float& positionX, float& positionY)
{
    // only support Alignment.Center now
    auto size = layoutConstraint.selfIdealSize;
    if (size.Width().has_value()) {
        auto width = size.Width().value();
        positionX += (colLen - width) / 2;
    }
    if (size.Height().has_value()) {
        auto height = size.Height().value();
        positionY += (rowLen - height) / 2;
    }
}
} // namespace

LayoutConstraintF GridLayoutAlgorithm::CreateChildConstraint(const SizeF& idealSize,
    const RefPtr<GridLayoutProperty>& layoutProperty, int32_t row, int32_t col, int32_t& rowSpan, int32_t& colSpan,
    const RefPtr<LayoutProperty>& childLayoutProperty) const
{
    LayoutConstraintF layoutConstraint = layoutProperty->CreateChildConstraint();

    float rowLen = 0.0;
    for (int32_t i = 0; i < rowSpan; ++i) {
        rowLen += gridCells_.at(row + i).at(col).Height();
    }
    rowLen += (rowSpan - 1) * rowsGap_;

    float colLen = 0.0;
    for (int32_t i = 0; i < colSpan; ++i) {
        colLen += gridCells_.at(row).at(col + i).Width();
    }
    colLen += (colSpan - 1) * columnsGap_;

    layoutConstraint.maxSize = SizeF(colLen, rowLen);
    layoutConstraint.percentReference = SizeF(colLen, rowLen);
    if (!childLayoutProperty || !childLayoutProperty->GetCalcLayoutConstraint()) {
        layoutConstraint.selfIdealSize.UpdateIllegalSizeWithCheck(layoutConstraint.maxSize);
    }
    return layoutConstraint;
}

void GridLayoutAlgorithm::InitGridCeils(LayoutWrapper* layoutWrapper, const SizeF& idealSize)
{
    auto layoutProperty = DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto scale = layoutProperty->GetLayoutConstraint()->scaleProperty;
    rowsGap_ = ConvertToPx(layoutProperty->GetRowsGap().value_or(0.0_vp), scale, idealSize.Height()).value_or(0);
    columnsGap_ = ConvertToPx(layoutProperty->GetColumnsGap().value_or(0.0_vp), scale, idealSize.Width()).value_or(0);
    auto rowsLen = GridUtils::ParseArgs(layoutProperty->GetRowsTemplate().value_or(""), idealSize.Height(), rowsGap_);
    auto colsLen =
        GridUtils::ParseArgs(layoutProperty->GetColumnsTemplate().value_or(""), idealSize.Width(), columnsGap_);
    if (rowsLen.empty()) {
        rowsLen.push_back(idealSize.Height());
    }
    if (colsLen.empty()) {
        colsLen.push_back(idealSize.Width());
    }
    gridCells_.clear();
    int32_t row = 0;
    for (const auto& height : rowsLen) {
        int32_t col = 0;
        for (const auto& width : colsLen) {
            gridCells_[row][col] = SizeF(width, height);
            ++col;
        }
        ++row;
    }
}

bool GridLayoutAlgorithm::CheckGridPlaced(int32_t index, int32_t row, int32_t col, int32_t& rowSpan, int32_t& colSpan)
{
    auto rowIter = gridLayoutInfo_.gridMatrix_.find(row);
    if (rowIter != gridLayoutInfo_.gridMatrix_.end()) {
        auto colIter = rowIter->second.find(col);
        if (colIter != rowIter->second.end()) {
            return false;
        }
    }
    rowSpan = std::min(mainCount_ - row, rowSpan);
    colSpan = std::min(crossCount_ - col, colSpan);
    int32_t rSpan = 0;
    int32_t cSpan = 0;
    int32_t retColSpan = 1;
    while (rSpan < rowSpan) {
        rowIter = gridLayoutInfo_.gridMatrix_.find(rSpan + row);
        if (rowIter != gridLayoutInfo_.gridMatrix_.end()) {
            cSpan = 0;
            while (cSpan < colSpan) {
                if (rowIter->second.find(cSpan + col) != rowIter->second.end()) {
                    colSpan = cSpan;
                    break;
                }
                ++cSpan;
            }
        } else {
            cSpan = colSpan;
        }
        if (retColSpan > cSpan) {
            break;
        }
        retColSpan = cSpan;
        ++rSpan;
    }

    rowSpan = rSpan;
    colSpan = retColSpan;
    for (int32_t i = row; i < row + rowSpan; ++i) {
        std::map<int32_t, int32_t> rowMap;
        auto iter = gridLayoutInfo_.gridMatrix_.find(i);
        if (iter != gridLayoutInfo_.gridMatrix_.end()) {
            rowMap = iter->second;
        }
        for (int32_t j = col; j < col + colSpan; ++j) {
            rowMap.emplace(std::make_pair(j, index));
        }
        gridLayoutInfo_.gridMatrix_[i] = rowMap;
    }
    return true;
}

void GridLayoutAlgorithm::GetNextGrid(int32_t& curRow, int32_t& curCol) const
{
    if (isVertical_) {
        ++curCol;
        if (curCol >= crossCount_) {
            curCol = 0;
            ++curRow;
        }
    } else {
        ++curRow;
        if (curRow >= mainCount_) {
            curRow = 0;
            ++curCol;
        }
    }
}

OffsetF GridLayoutAlgorithm::ComputeItemPosition(LayoutWrapper* layoutWrapper, int32_t row, int32_t col,
    int32_t& rowSpan, int32_t& colSpan, const RefPtr<OHOS::Ace::NG::LayoutProperty>& childLayoutProperty) const
{
    auto layoutProperty = DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, OffsetF());
    auto frameSize = layoutWrapper->GetGeometryNode()->GetMarginFrameSize();

    // Calculate the position for current child.
    float positionX = 0.0f;
    float positionY = 0.0f;
    for (int32_t i = 0; i < row; ++i) {
        positionY += gridCells_.at(i).at(0).Height();
    }
    positionY += row * rowsGap_;
    for (int32_t i = 0; i < col; ++i) {
        positionX += gridCells_.at(0).at(i).Width();
    }
    positionX += col * columnsGap_;

    // Calculate the size for current child.
    float rowLen = 0.0f;
    float colLen = 0.0f;
    for (int32_t i = 0; i < rowSpan; ++i) {
        rowLen += gridCells_.at(row + i).at(col).Height();
    }
    rowLen += (rowSpan - 1) * rowsGap_;
    for (int32_t i = 0; i < colSpan; ++i) {
        colLen += gridCells_.at(row).at(col + i).Width();
    }
    colLen += (colSpan - 1) * columnsGap_;

    if (childLayoutProperty) {
        auto layoutConstraint = childLayoutProperty->GetLayoutConstraint();
        if (layoutConstraint.has_value()) {
            OffsetByAlign(layoutConstraint.value(), rowLen, colLen, positionX, positionY);
        }
    }

    // If RTL, place the item from right.
    if (rightToLeft_) {
        positionX = frameSize.Width() - positionX - colLen;
    }
    return OffsetF(positionX, positionY);
}

void GridLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto gridLayoutProperty = AceType::DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(gridLayoutProperty);
    Axis axis = gridLayoutInfo_.axis_;
    auto idealSize =
        CreateIdealSize(gridLayoutProperty->GetLayoutConstraint().value(), axis, MeasureType::MATCH_PARENT, true);
    if (GreatOrEqual(GetMainAxisSize(idealSize, axis), Infinity<float>())) {
        LOGE("size of main axis value is infinity, please check");
        return;
    }

    layoutWrapper->GetGeometryNode()->SetFrameSize(idealSize);
    MinusPaddingToSize(gridLayoutProperty->CreatePaddingAndBorder(), idealSize);
    InitGridCeils(layoutWrapper, idealSize);

    int32_t rowIndex = 0;
    int32_t colIndex = 0;
    int32_t itemIndex = 0;
    itemsPosition_.clear();
    gridLayoutInfo_.gridMatrix_.clear();
    for (int32_t index = 0; index < mainCount_ * crossCount_; ++index) {
        auto childLayoutWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        if (!childLayoutWrapper) {
            break;
        }
        auto layoutProperty = childLayoutWrapper->GetLayoutProperty();
        if (!layoutProperty) {
            break;
        }
        auto childLayoutProperty = DynamicCast<GridItemLayoutProperty>(layoutProperty);
        int32_t itemRowStart = -1;
        int32_t itemColStart = -1;
        int32_t itemRowSpan = 1;
        int32_t itemColSpan = 1;

        if (childLayoutProperty) {
            itemRowStart = childLayoutProperty->GetRowStart().value_or(-1);
            itemColStart = childLayoutProperty->GetColumnStart().value_or(-1);
            itemRowSpan = std::max(childLayoutProperty->GetRowEnd().value_or(-1) - itemRowStart + 1, 1);
            itemColSpan = std::max(childLayoutProperty->GetColumnEnd().value_or(-1) - itemColStart + 1, 1);
        }

        if (itemRowStart >= 0 && itemRowStart < mainCount_ && itemColStart >= 0 && itemColStart < crossCount_ &&
            CheckGridPlaced(itemIndex, itemRowStart, itemColStart, itemRowSpan, itemColSpan)) {
            childLayoutWrapper->Measure(CreateChildConstraint(idealSize, gridLayoutProperty, itemRowStart, itemColStart,
                itemRowSpan, itemColSpan, childLayoutProperty));
            itemsPosition_.try_emplace(index, ComputeItemPosition(layoutWrapper, itemRowStart, itemColStart,
                itemRowSpan, itemColSpan, childLayoutProperty));
        } else {
            while (!CheckGridPlaced(itemIndex, rowIndex, colIndex, itemRowSpan, itemColSpan)) {
                GetNextGrid(rowIndex, colIndex);
                if (rowIndex >= mainCount_ || colIndex >= crossCount_) {
                    break;
                }
            }
            if (rowIndex >= mainCount_ || colIndex >= crossCount_) {
                continue;
            }
            childLayoutWrapper->Measure(CreateChildConstraint(
                idealSize, gridLayoutProperty, rowIndex, colIndex, itemRowSpan, itemColSpan, childLayoutProperty));
            itemsPosition_.try_emplace(index,
                ComputeItemPosition(layoutWrapper, rowIndex, colIndex, itemRowSpan, itemColSpan, childLayoutProperty));
        }
        ++itemIndex;
    }
}

void GridLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto layoutProperty = AceType::DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto frameSize = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto padding = layoutProperty->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, frameSize);

    for (int32_t index = 0; index < mainCount_ * crossCount_; ++index) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        if (!childWrapper) {
            break;
        }
        OffsetF childOffset;
        auto childPosition = itemsPosition_.find(index);
        if (childPosition != itemsPosition_.end()) {
            childOffset = itemsPosition_.at(index);
            // TODO: add center position when grid item is less than ceil.
            childWrapper->GetGeometryNode()->SetMarginFrameOffset(padding.Offset() + childOffset);
            childWrapper->Layout();
        } else {
            LayoutWrapper::RemoveChildInRenderTree(childWrapper);
        }
    }

    for (const auto& mainLine : gridLayoutInfo_.gridMatrix_) {
        int32_t itemIndex = -1;
        for (const auto& crossLine : mainLine.second) {
            // If item index is the same, must be the same GridItem, needn't layout again.
            if (itemIndex == crossLine.second) {
                continue;
            }
            itemIndex = crossLine.second;
            auto wrapper = layoutWrapper->GetOrCreateChildByIndex(itemIndex);
            if (!wrapper) {
                LOGE("Layout item wrapper of index: %{public}d is null, please check.", itemIndex);
                break;
            }
            auto layoutProperty = wrapper->GetLayoutProperty();
            CHECK_NULL_VOID(layoutProperty);
            auto gridItemLayoutProperty = AceType::DynamicCast<GridItemLayoutProperty>(layoutProperty);
            CHECK_NULL_VOID(gridItemLayoutProperty);
            gridItemLayoutProperty->UpdateMainIndex(mainLine.first);
            gridItemLayoutProperty->UpdateCrossIndex(crossLine.first);
        }
    }
}

} // namespace OHOS::Ace::NG