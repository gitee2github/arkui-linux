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

/**
 * ObservedPropertySimplePU
 * implementation of @State and @Provide decorated variables of types (T=) boolean | number | string | enum
 *
 * Holds an actual property value of type T
 * uses its base class to manage subscribers to this
 * property.
 * 
 * all definitions in this file are framework internal
*/
class ObservedPropertySimplePU<T> extends ObservedPropertySimpleAbstractPU<T>
  implements ISinglePropertyChangeSubscriber<T>, PeerChangeEventReceiverPU<T> {

  private wrappedValue_: T;

  constructor(value: T, owningView: IPropertySubscriber, propertyName: PropertyInfo) {
    super(owningView, propertyName);
    if (typeof value === "object") {
      throw new SyntaxError("ObservedPropertySimple value must not be an object")!
    }
    this.setValueInternal(value);
  }

  aboutToBeDeleted(unsubscribeMe?: IPropertySubscriber) {
    if (unsubscribeMe) {
      this.unlinkSuscriber(unsubscribeMe.id__());
    }
    super.aboutToBeDeleted();
  }

  /**
 * Called by a @Link - SynchedPropertySimpleTwoWay that uses this as sync peer when it has changed
 * @param eventSource 
 */
  syncPeerHasChanged(eventSource: ObservedPropertyAbstractPU<T>) {
    stateMgmtConsole.debug(`ObservedPropertySimple[${this.id__()}, '${this.info() || "unknown"}']: syncPeerHasChanged peer '${eventSource.info()}'.`);
    this.notifyPropertyHasChangedPU();
  }

  /*
    actually update this.wrappedValue_
    called needs to do value change check
    and also notify with this.aboutToChange();
  */
  private setValueInternal(newValue: T): void {
    stateMgmtConsole.debug(`ObservedPropertySimple[${this.id__()}, '${this.info() || "unknown"}'] new value is of simple type`);
    this.wrappedValue_ = newValue;
  }

  public getUnmonitored(): T {
    stateMgmtConsole.debug(`ObservedPropertySimple[${this.id__()}, '${this.info() || "unknown"}']: getUnmonitored returns '${JSON.stringify(this.wrappedValue_)}' .`);
    // unmonitored get access , no call to otifyPropertyRead !
    return this.wrappedValue_;
  }

  public get(): T {
    stateMgmtConsole.debug(`ObservedPropertySimple[${this.id__()}, '${this.info() || "unknown"}']: get returns '${JSON.stringify(this.wrappedValue_)}' .`);
    this.notifyPropertyHasBeenReadPU()
    return this.wrappedValue_;
  }

  public set(newValue: T): void {
    if (this.wrappedValue_ == newValue) {
      stateMgmtConsole.debug(`ObservedPropertySimple[${this.id__()}, '${this.info() || "unknown"}']: set with unchanged value - ignoring.`);
      return;
    }
    stateMgmtConsole.debug(`ObservedPropertySimple[${this.id__()}, '${this.info() || "unknown"}']: set, changed from '${JSON.stringify(this.wrappedValue_)}' to '${JSON.stringify(newValue)}.`);
    this.setValueInternal(newValue);
    this.notifyPropertyHasChangedPU();

  }
}
