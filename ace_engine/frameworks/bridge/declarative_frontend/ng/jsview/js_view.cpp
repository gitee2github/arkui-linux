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

#include "frameworks/bridge/declarative_frontend/jsview/js_view.h"

#include "base/log/ace_trace.h"
#include "base/memory/referenced.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "frameworks/bridge/declarative_frontend/engine/js_execution_scope_defines.h"

namespace OHOS::Ace::Framework {

void JSView::JSBind(BindingTarget object)
{
    JSViewPartialUpdate::JSBind(object);
    JSViewFullUpdate::JSBind(object);
}

void JSView::MarkNeedUpdate()
{
    ACE_SCOPED_TRACE("JSView::MarkNeedUpdate");
    needsUpdate_ = true;
}

RefPtr<PageTransitionComponent> JSView::BuildPageTransitionComponent()
{
    return nullptr;
}

RefPtr<Component> JSViewFullUpdate::CreateComponent()
{
    return nullptr;
}

RefPtr<NG::UINode> JSViewFullUpdate::CreateUINode()
{
    ACE_SCOPED_TRACE("JSView::CreateSpecializedComponent");
    // create component, return new something, need to set proper ID
    std::string key = NG::ViewStackProcessor::GetInstance()->ProcessViewId(viewId_);
    // TODO: add unique id.
    auto composedNode = NG::CustomNode::CreateCustomNode(0, key);
    node_ = composedNode;

    {
        ACE_SCORING_EVENT("Component[" + viewId_ + "].Appear");
        if (jsViewFunction_) {
            jsViewFunction_->ExecuteAppear();
        }
    }

    // add callback for element creation to component, and get pointer reference
    // to the element on creation. When state of this view changes, mark the
    // element to dirty.
    auto renderFunction = [weak = AceType::WeakClaim(this)]() -> RefPtr<NG::UINode> {
        auto jsView = weak.Upgrade();
        return jsView ? jsView->InternalRender() : nullptr;
    };

    composedNode->SetRenderFunction(std::move(renderFunction));
    return composedNode;
}

RefPtr<Component> JSViewFullUpdate::InternalRender(const RefPtr<Component>& parent)
{
    needsUpdate_ = false;
    return nullptr;
}

RefPtr<NG::UINode> JSViewFullUpdate::InternalRender()
{
    needsUpdate_ = false;
    RenderJSExecution();
    CleanUpAbandonedChild();
    jsViewFunction_->Destroy();
    return NG::ViewStackProcessor::GetInstance()->Finish();
}

/**
 * marks the JSView's composed component as needing update / rerender
 */
void JSViewFullUpdate::MarkNeedUpdate()
{
    ACE_SCOPED_TRACE("JSView::MarkNeedUpdate");
    LOGW("not support full update in ng structure");
}

void JSViewFullUpdate::Create(const JSCallbackInfo& info)
{
    if (info[0]->IsObject()) {
        JSRef<JSObject> object = JSRef<JSObject>::Cast(info[0]);
        auto* view = object->Unwrap<JSViewFullUpdate>();
        if (view == nullptr) {
            LOGE("JSView is null");
            return;
        }
        NG::ViewStackProcessor::GetInstance()->Push(view->CreateUINode(), true);
    } else {
        LOGE("JSView Object is expected.");
    }
}

JSRef<JSObject> JSViewFullUpdate::GetChildById(const std::string& viewId)
{
    return {};
}

std::string JSViewFullUpdate::AddChildById(const std::string& viewId, const JSRef<JSObject>& obj)
{
    return "";
}

RefPtr<NG::UINode> JSViewPartialUpdate::CreateUINode()
{
    ACE_SCOPED_TRACE("JSViewPartialUpdate::CreateSpecializedComponent");
    auto viewId = NG::ViewStackProcessor::GetInstance()->ClaimNodeId();
    viewId_ = std::to_string(viewId);
    auto key = NG::ViewStackProcessor::GetInstance()->ProcessViewId(viewId_);
    LOGD("Creating CustomNode with claimed elmtId %{public}d.", viewId);
    auto customNode = NG::CustomNode::CreateCustomNode(viewId, key);
    node_ = customNode;
    {
        ACE_SCORING_EVENT("Component[" + viewId_ + "].Appear");
        if (jsViewFunction_) {
            jsViewFunction_->ExecuteAppear();
        }
    }

    auto renderFunction = [weak = AceType::WeakClaim(this)]() -> RefPtr<NG::UINode> {
        auto jsView = weak.Upgrade();
        CHECK_NULL_RETURN(jsView, nullptr);
        if (!jsView->isFirstRender_) {
            LOGW("the js view has already called initial render");
            return nullptr;
        }
        jsView->isFirstRender_ = false;
        return jsView->InitialUIRender();
    };
    customNode->SetRenderFunction(renderFunction);

    auto updateFunction = [weak = AceType::WeakClaim(this)]() -> void {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        if (!jsView->needsUpdate_) {
            LOGW("the js view does not need to update");
            return;
        }
        jsView->needsUpdate_ = false;
        LOGD("Rerender function start for ComposedElement elmtId %{public}s - start...", jsView->viewId_.c_str());
        {
            ACE_SCOPED_TRACE("JSView: ExecuteRerender");
            jsView->jsViewFunction_->ExecuteRerender();
        }
    };
    customNode->SetUpdateFunction(std::move(updateFunction));

    // partial update relies on remove function
    auto removeFunction = [weak = AceType::WeakClaim(this)]() -> void {
        LOGD("call remove view function");
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        jsView->Destroy(nullptr);
        jsView->node_.Reset();
    };
    customNode->SetDestroyFunction(std::move(removeFunction));
    return customNode;
}

RefPtr<NG::UINode> JSViewPartialUpdate::InitialUIRender()
{
    needsUpdate_ = false;
    RenderJSExecution();
    return NG::ViewStackProcessor::GetInstance()->Finish();
}

void JSViewPartialUpdate::MarkNeedUpdate()
{
    auto node = node_.Upgrade();
    if (!node) {
        LOGE("fail to update due to custom Node is null");
        return;
    }
    needsUpdate_ = true;
    if (AceType::InstanceOf<NG::CustomNode>(node)) {
        auto customNode = AceType::DynamicCast<NG::CustomNode>(node);
        customNode->MarkNeedUpdate();
    } else if (AceType::InstanceOf<NG::CustomMeasureLayoutNode>(node)) {
        auto customNode = AceType::DynamicCast<NG::CustomMeasureLayoutNode>(node);
        customNode->MarkNeedUpdate();
    }
}

/**
 * in JS View.create(new View(...));
 * used for FullRender case, not for re-render case
 */
void JSViewPartialUpdate::Create(const JSCallbackInfo& info)
{
    LOGD("Creating new JSViewPartialUpdate for partial update");
    ACE_DCHECK(Container::IsCurrentUsePartialUpdate());

    if (info[0]->IsObject()) {
        JSRef<JSObject> object = JSRef<JSObject>::Cast(info[0]);
        auto* view = object->Unwrap<JSView>();
        if (view == nullptr) {
            LOGE("View is null");
            return;
        }
        NG::ViewStackProcessor::GetInstance()->Push(view->CreateUINode(), true);
    } else {
        LOGE("View Object is expected.");
    }
}

// ===========================================================
// partial update own functions start below
// ===========================================================

void JSViewPartialUpdate::JsFinishUpdateFunc(int32_t elmtId)
{
    NG::ViewStackProcessor::GetInstance()->FlushRerenderTask();
}

void JSViewPartialUpdate::ComponentToElementLocalizedUpdate(const UpdateFuncResult& updateFuncResult) {}

} // namespace OHOS::Ace::Framework
