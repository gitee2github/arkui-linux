/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIEW_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIEW_H

#include <functional>
#include <list>
#include <string>

#include "base/log/ace_scoring_log.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/base/view_partial_update_model.h"
#include "frameworks/bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_functions.h"

namespace OHOS::Ace::Framework {

class JSView : public JSViewAbstract, public Referenced {
public:
    JSView() : instanceId_(Container::CurrentId()) {}
    ~JSView() override = default;
    virtual void Destroy(JSView* parentCustomView) = 0;

    virtual RefPtr<AceType> CreateViewNode()
    {
        LOGE("Internal error. Not implemented");
        return nullptr;
    }

    void SyncInstanceId();
    void RestoreInstanceId();

    void FireOnShow()
    {
        if (jsViewFunction_) {
            ACE_SCORING_EVENT("OnShow");
            jsViewFunction_->ExecuteShow();
        }
    }

    void FireOnHide()
    {
        if (jsViewFunction_) {
            ACE_SCORING_EVENT("OnHide");
            jsViewFunction_->ExecuteHide();
        }
    }

    bool FireOnBackPress()
    {
        if (jsViewFunction_) {
            ACE_SCORING_EVENT("OnBackPress");
            return jsViewFunction_->ExecuteOnBackPress();
        }
        return false;
    }

    void RenderJSExecution();

    virtual void MarkNeedUpdate() = 0;

    bool NeedsUpdate();

    static void JSBind(BindingTarget globalObj);
    /**
     * Views which do not have a state can mark static.
     * The element will be reused and re-render will be skipped.
     */
    void MarkStatic();

    bool IsStatic()
    {
        return isStatic_;
    }

    void SetContext(const JSExecutionContext& context)
    {
        jsViewFunction_->SetContext(context);
    }

    // Used to set/get card id C++
    void SetCardId(int64_t cardId)
    {
        cardId_ = cardId;
    }

    int64_t GetCardId() const
    {
        return cardId_;
    }

    void RegisterRenderDoneCallback(std::function<void()>&& OnRenderDone)
    {
        notifyRenderDone_ = std::move(OnRenderDone);
    }

    // Used to set/get card id JS
    void JsSetCardId(int64_t cardId);
    void JsGetCardId(const JSCallbackInfo& info);

    // Used by full update variant only from js_lazy_foreach.cpp
    virtual void RemoveChildGroupById(const std::string& viewId) {}
    virtual void MarkLazyForEachProcess(const std::string& groudId) {}
    virtual void ResetLazyForEachProcess() {}
    virtual void ExecuteUpdateWithValueParams(const std::string& jsonData) {}
    virtual void ExecuteInitiallyProvidedValue(const std::string& jsonData) {}

    virtual bool isFullUpdate() const
    {
        return true;
    }

#ifdef UICAST_COMPONENT_SUPPORTED
    void ExecuteCreateChildView(const std::string& childViewId)
    {
        std::string jsonData = R"({"viewID":")" + childViewId + R"("})";
        LOGI("UICast para: %{public}s", jsonData.c_str());
        jsViewFunction_->ExecuteCreateChildView(jsonData);
    }

    void ExecuteRouterHandle(const std::string& type, const std::string& uri)
    {
        std::string jsonData = R"({"uri":")" + uri + R"(","type":")" + type + R"("})";
        LOGI("UICast para: %{public}s", jsonData.c_str());
        ContainerScope scope(instanceId_);
        jsViewFunction_->ExecuteRouterHandle(jsonData);
    }

    void ExecuteReplayOnEvent(const std::string& event)
    {
        std::string jsonData = R"({"event":")" + event + R"("})";
        LOGI("UICast para: %{public}s", jsonData.c_str());
        jsViewFunction_->ExecuteReplayOnEvent(jsonData);
    }
#endif

    std::string UICastGetViewId() const
    {
        return viewId_;
    }

    int UICastGetUniqueId() const
    {
        return uniqueId_;
    }

    int32_t GetInstanceId() const
    {
        return instanceId_;
    }

protected:
    RefPtr<ViewFunctions> jsViewFunction_;
    bool needsUpdate_ = false;

    WeakPtr<AceType> viewNode_;
    // view id for custom view itself
    std::string viewId_;
    int uniqueId_ = -1;

    // card id for eTS Card
    // set on the root JSView of the card and inherited by all child JSViews
    // -1 means not part of a card
    int64_t cardId_ = -1;

private:
    int32_t instanceId_ = -1;
    int32_t restoreInstanceId_ = -1;
    bool isStatic_ = false;
    std::function<void()> notifyRenderDone_;
};

class JSViewFullUpdate : public JSView {
public:
    JSViewFullUpdate(const std::string& viewId, JSRef<JSObject> jsObject, JSRef<JSFunc> jsRenderFunction);
    ~JSViewFullUpdate() override;

    void Destroy(JSView* parentCustomView) override;

    // TODO: delete this after the toolchain for partial update is ready.
    RefPtr<AceType> InternalRender();

    RefPtr<AceType> CreateViewNode() override;

    void MarkNeedUpdate() override;

    /**
     * During render function execution, the child custom view with same id will
     * be recycled if they exist already in our child map. The ones which are not
     * recycled needs to be cleaned. So After render function execution, clean the
     * abandoned child custom view.
     */
    void CleanUpAbandonedChild();

    /**
     * Retries the custom view child for recycling
     * always use FindChildById to be certain before calling this method
     */
    JSRef<JSObject> GetChildById(const std::string& viewId);

    void FindChildById(const JSCallbackInfo& info);
    void FindChildByIdForPreview(const JSCallbackInfo& info);
    bool GetChildByViewId(const std::string& viewId, JSRef<JSObject>& childView, JSRef<JSObject>& targetView);

    void ExecuteUpdateWithValueParams(const std::string& jsonData) override
    {
        jsViewFunction_->ExecuteUpdateWithValueParams(jsonData);
    }

    void MarkLazyForEachProcess(const std::string& groudId) override
    {
        isLazyForEachProcessed_ = true;
        lazyItemGroupId_ = groudId;
    }

    void ResetLazyForEachProcess() override
    {
        isLazyForEachProcessed_ = false;
        lazyItemGroupId_ = "";
    }

    /**
     * New CustomView child will be added to the map.
     * and it can be retrieved for recycling in next render function
     * In next render call if this child is not recycled, it will be destroyed.
     */
    std::string AddChildById(const std::string& viewId, const JSRef<JSObject>& obj);

    void RemoveChildGroupById(const std::string& viewId) override;

    static void Create(const JSCallbackInfo& info);
    static void JSBind(BindingTarget globalObj);

    static void ConstructorCallback(const JSCallbackInfo& args);
    static void DestructorCallback(JSViewFullUpdate* instance);

private:
    void DestroyChild(JSView* parentCustomView);

    /**
     * Takes care of the viewId wrt to foreach
     */
    std::string ProcessViewId(const std::string& viewId);
    /**
     * creates a set of valid view ids on a render function execution
     * its cleared after cleaning up the abandoned child.
     */
    void ChildAccessedById(const std::string& viewId);

    bool isLazyForEachProcessed_ = false;
    std::string lazyItemGroupId_;

    // unique view id for custom view to recycle.
    std::string id_;
    // hold handle to the native and javascript object to keep them alive
    // until they are abandoned
    std::unordered_map<std::string, JSRef<JSObject>> customViewChildren_;

    // hold handle to the native and javascript object to keep them alive
    // until they are abandoned used by lazyForEach
    std::unordered_map<std::string, JSRef<JSObject>> customViewChildrenWithLazy_;

    // hold js view ids by lazy item ground.
    // until they are abandoned used by lazyForEach
    std::unordered_map<std::string, std::list<std::string>> lazyItemGroups_;

    // a set of valid view ids on a render function execution
    // its cleared after cleaning up the abandoned child.
    std::unordered_set<std::string> lastAccessedViewIds_;

    // The C++ JSView object owns a reference to the JS Object
    // AssignNewView assigns the JS View
    // Destroy deleted the ref, and thereby triggers the deletion
    // GC -> JS View Object -> JSView C++ Object
    JSRef<JSObject> jsViewObject_;
};

class JSViewPartialUpdate : public JSView {
public:
    explicit JSViewPartialUpdate(JSRef<JSObject> jsObject);
    ~JSViewPartialUpdate() override;

    void Destroy(JSView* parentCustomView) override;

    RefPtr<AceType> InitialRender();

    RefPtr<AceType> CreateViewNode() override;

    static void Create(const JSCallbackInfo& info);
    static void JSBind(BindingTarget globalObj);

    static void ConstructorCallback(const JSCallbackInfo& args);
    static void DestructorCallback(JSViewPartialUpdate* instance);

    // public functions added by partial update added below ==================

    /**
     * Last step of executing an partial update function
     * get the result component from ViewStackProcessor
     * add it to the queue to [elmtId, Component] to
     * execute an local update on in the UI thread
     * parameters
     * elmtId of the Component/Element that's updated
     * removedElementId : Array<number>  ids of Elements that were removed while updating
     * caused by if condition toggle or ForEach array deleted / replaced items
     * return boolean - true means success
     */
    void JsFinishUpdateFunc(int32_t elmtId);

    // The process of Component to Element sync leads to Elements being
    // deleted. ElementRegister keeps track of these deletions
    // before the framework can forget about these elmtIds
    // these need to be removed from its own book keeping
    // state variables keep track of dependent elmtIds and
    // View objects keep a map elmtId -> update function,
    // both on TS side.
    // View.purgeDeletedElmtIds cleans both state variables
    // and update function map from deleted ElmtIds
    // afterwards it informs the ElementRegister that elmtIds
    // it was able to purge.
    // only then ElementRegister can forget about these elmtIds
    void JsGetDeletedElemtIds(const JSCallbackInfo& info);

    // JS signature: View.deletedElmtIdsHaveBeenPurged(elmtIds : number[])
    // inform ElementRegister that given deleted eltIds
    // have been deleted from partial updates book keeping
    // at this point ElementRegister can forget about the,
    void JsDeletedElmtIdsHaveBeenPurged(const JSCallbackInfo& info);

    /**
    JS exposed function to check from ElementRegister if given elmtId is (still) in use
    */
    bool JsElementIdExists(int32_t elmtId);

    void JSGetProxiedItemRenderState(const JSCallbackInfo& info);

    bool isFullUpdate() const override
    {
        return false;
    }

    void IsFirstRender(const JSCallbackInfo& info);
    void FindChildByIdForPreview(const JSCallbackInfo& info);

    void ExecuteInitiallyProvidedValue(const std::string& jsonData) override
    {
        jsViewFunction_->ExecuteInitiallyProvidedValue(jsonData);
    }

private:
    void MarkNeedUpdate() override;

    // indicates if the JSView has ever completed initial render
    // used for code branching in lambda given to ComposedComponent
    // render callback
    bool isFirstRender_ = true;

    /* list of update function result is a triple (tuple with three entries)
    <0> elmtId
    <1> outmost wrapping Component
    <2> main Component
    */
    std::list<UpdateTask> pendingUpdateTasks_;

    // The C++ JSView object owns a reference to the JS Object
    // AssignNewView assigns the JS View
    // Destroy deleted the ref, and thereby triggers the deletion
    // GC -> JS View Object -> JSView C++ Object
    JSRef<JSObject> jsViewObject_;
};

} // namespace OHOS::Ace::Framework
#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIEW_H
