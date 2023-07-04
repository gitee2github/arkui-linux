/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 *
 * LocalStorage
 *
 * Class implements a Map of ObservableObjectBase UI state variables.
 * Instances can be created to manage UI state within a limited "local"
 * access, and life cycle as defined by the app.
 * AppStorage singleton is sub-class of LocalStorage for
 * UI state of app-wide access and same life cycle as the app.
 *
 * @since 9
 */
 class LocalStorage extends NativeLocalStorage {
    /**
     * Construct new instance of LocalStorage
     * initialzie with all properties and their values that Object.keys(params) returns
     * Property values must not be undefined.
     * @param initializingProperties Object containing keys and values. @see set() for valid values
     *
     * @since 9
     */
    constructor(initializingProperties = {}) {
        super();
        
        this.storage_ = new Map();
        if (Object.keys(initializingProperties).length) {
            this.initializeProps(initializingProperties);
        }
    }
    /**
     * clear storage and init with given properties
     * @param initializingProperties
     *
     * not a public / sdk function
     */
    initializeProps(initializingProperties = {}) {
        
        this.storage_.clear();
        Object.keys(initializingProperties).filter((propName) => initializingProperties[propName] != undefined).forEach((propName) => this.addNewPropertyInternal(propName, initializingProperties[propName]));
    }
    /**
     * Use before deleting owning Ability, window, or service UI
     * (letting it go out of scope).
     *
     * This method orderly closes down a LocalStorage instance by calling @see clear().
     * This requires that no property is left with one or more subscribers.
     * @see clear() and @see delete()
     * @returns true if all properties could be removed from storage
     */
    aboutToBeDeleted() {
        return this.clear();
    }
    /**
     * Check if LocalStorage has a property with given name
     * return true if prooperty with given name exists
     * same as ES6 Map.prototype.has()
     * @param propName searched property
     * @returns true if property with such name exists in LocalStorage
     *
     * @since 9
     */
    has(propName) {
        return this.storage_.has(propName);
    }
    /**
     * Provide names of all properties in LocalStorage
     * same as ES6 Map.prototype.keys()
     * @returns return a Map Iterator
     *
     * @since 9
    */
    keys() {
        return this.storage_.keys();
    }
    /**
     * Returns number of properties in LocalStorage
     * same as Map.prototype.size()
     * @param propName
     * @returns return number of properties
     *
     * @since 9
     */
    size() {
        return this.storage_.size;
    }
    /**
     * Returns value of given property
     * return undefined if no property with this name
     * @param propName
     * @returns property value if found or undefined
     *
     * @since 9
     */
    get(propName) {
        var p = this.storage_.get(propName);
        return (p) ? p.get() : undefined;
    }
    /**
     * Set value of given property in LocalStorage
     * Methosd sets nothing and returns false if property with this name does not exist
     * or if newValue is `undefined` or `null` (`undefined`, `null` value are not allowed for state variables).
     * @param propName
     * @param newValue must be of type T and must not be undefined or null
     * @returns true on success, i.e. when above conditions are satisfied, otherwise false
     *
     * @since 9
     */
    set(propName, newValue) {
        if (newValue == undefined) {
            stateMgmtConsole.warn(`${this.constructor.name}: set('${propName}') with newValue == undefined not allowed.`);
            return false;
        }
        var p = this.storage_.get(propName);
        if (p == undefined) {
            stateMgmtConsole.warn(`${this.constructor.name}: set: no property ${propName} error.`);
            return false;
        }
        p.set(newValue);
        return true;
    }
    /**
     * Set value of given property, if it exists, @see set() .
     * Add property if no property with given name and initialize with given value.
     * Do nothing and return false if newValuue is undefined or null
     * (undefined, null value is not allowed for state variables)
     * @param propName
     * @param newValue must be of type T and must not be undefined or null
     * @returns true on success, i.e. when above conditions are satisfied, otherwise false
     *
     * @since 9
     */
    setOrCreate(propName, newValue) {
        if (newValue == undefined) {
            stateMgmtConsole.warn(`${this.constructor.name}: setOrCreate('${propName}') with newValue == undefined not allowed.`);
            return false;
        }
        var p = this.storage_.get(propName);
        if (p) {
            
            p.set(newValue);
        }
        else {
            
            this.addNewPropertyInternal(propName, newValue);
        }
        return true;
    }
    /**
     * Internal use helper function to create and initialize a new property.
     * caller needs to be all the checking beforehand
     * @param propName
     * @param value
     *
     * Not a public / sdk method.
     */
    addNewPropertyInternal(propName, value) {
        const newProp = (typeof value === "object") ?
            new ObservedPropertyObject(value, undefined, propName)
            : new ObservedPropertySimple(value, undefined, propName);
        this.storage_.set(propName, newProp);
        return newProp;
    }
    /**
     * create and return a two-way sync "(link") to named property
     * @param propName name of source property in LocalStorage
     * @param linkUser IPropertySubscriber to be notified when source changes,
     * @param subscribersName optional, the linkUser (subscriber) uses this name for the property
     *      this name will be used in propertyChange(propName) callback of IMultiPropertiesChangeSubscriber
     * @returns  SynchedPropertyTwoWay{Simple|Object| object with given LocalStoage prop as its source.
     *           Apps can use SDK functions of base class SubscribedAbstractProperty<S>
     *           return undefiend if named property does not already exist in LocalStorage
     *           Apps can use SDK functions of base class SubscribedPropertyAbstract<S>
     *           return undefiend if named property does not already exist in LocalStorage
     *
     * @since 9
     */
    link(propName, linkUser, subscribersName) {
        var p = this.storage_.get(propName);
        if (p == undefined) {
            stateMgmtConsole.warn(`${this.constructor.name}: link: no property ${propName} error.`);
            return undefined;
        }
        let linkResult = p.createLink(linkUser, propName);
        linkResult.setInfo(subscribersName);
        return linkResult;
    }
    /**
     * Like @see link(), but will create and initialize a new source property in LocalStorge if missing
     * @param propName name of source property in LocalStorage
     * @param defaultValue value to be used for initializing if new creating new property in LocalStorage
     *        default value must be of type S, must not be undefined or null.
     * @param linkUser IPropertySubscriber to be notified when return 'link' changes,
     * @param subscribersName the linkUser (subscriber) uses this name for the property
     *      this name will be used in propertyChange(propName) callback of IMultiPropertiesChangeSubscriber
     * @returns SynchedPropertyTwoWay{Simple|Object| object with given LocalStoage prop as  its source.
     *          Apps can use SDK functions of base class SubscribedAbstractProperty<S>
     *
     * @since 9
     */
    setAndLink(propName, defaultValue, linkUser, subscribersName) {
        var p = this.storage_.get(propName);
        if (!p) {
            this.setOrCreate(propName, defaultValue);
        }
        return this.link(propName, linkUser, subscribersName);
    }
    /**
     * create and return a one-way sync ('prop') to named property
     * @param propName name of source property in LocalStorage
     * @param propUser IPropertySubscriber to be notified when source changes,
     * @param subscribersName the linkUser (subscriber) uses this name for the property
     *      this name will be used in propertyChange(propName) callback of IMultiPropertiesChangeSubscriber
     * @returns  SynchedPropertyOneWay{Simple|Object| object with given LocalStoage prop as  its source.
     *           Apps can use SDK functions of base class SubscribedAbstractProperty<S>
     *           return undefiend if named property does not already exist in LocalStorage.
     *           Apps can use SDK functions of base class SubscribedPropertyAbstract<S>
     *           return undefiend if named property does not already exist in LocalStorage.
     * @since 9
     */
    prop(propName, propUser, subscribersName) {
        var p = this.storage_.get(propName);
        if (p == undefined) {
            stateMgmtConsole.warn(`${this.constructor.name}: prop: no property ${propName} error.`);
            return undefined;
        }
        let propResult = p.createProp(propUser, propName);
        propResult.setInfo(subscribersName);
        return propResult;
    }
    /**
     * Like @see prop(), will create and initialize a new source property in LocalStorage if missing
     * @param propName name of source property in LocalStorage
     * @param defaultValue value to be used for initializing if new creating new property in LocalStorage.
     *        default value must be of type S, must not be undefined or null.
     * @param propUser IPropertySubscriber to be notified when returned 'prop' changes,
     * @param subscribersName the propUser (subscriber) uses this name for the property
     *      this name will be used in propertyChange(propName) callback of IMultiPropertiesChangeSubscriber
     * @returns  SynchedPropertyOneWay{Simple|Object| object with given LocalStoage prop as its source.
     *           Apps can use SDK functions of base class SubscribedAbstractProperty<S>
     * @since 9
     */
    setAndProp(propName, defaultValue, propUser, subscribersName) {
        var p = this.storage_.get(propName);
        if (!p) {
            this.setOrCreate(propName, defaultValue);
        }
        return this.prop(propName, propUser, subscribersName);
    }
    /**
     * Delete property from StorageBase
     * Use with caution:
     * Before deleting a prop from LocalStorage all its subscribers need to
     * unsubscribe from the property.
     * This method fails and returns false if given property still has subscribers
     * Another reason for failing is unkmown property.
     *
     * Developer advise:
     * Subscribers are created with @see link(), @see prop()
     * and also via @LocalStorageLink and @LocalStorageProp state variable decorators.
     * That means as long as their is a @Component instance that uses such decorated variable
     * or a sync relationship with a SubscribedAbstractProperty variable the property can nit
     * (and also should not!) be deleted from LocalStorage.
     *
     * @param propName
     * @returns false if method failed
     *
     * @since 9
    */
    delete(propName) {
        var p = this.storage_.get(propName);
        if (p) {
            if (p.numberOfSubscrbers()) {
                stateMgmtConsole.error(`${this.constructor.name}: Attempt to delete property ${propName} that has \
          ${p.numberOfSubscrbers()} subscribers. Subscribers need to unsubscribe before prop deletion.`);
                return false;
            }
            p.aboutToBeDeleted();
            this.storage_.delete(propName);
            return true;
        }
        else {
            stateMgmtConsole.warn(`${this.constructor.name}: Attempt to delete unknown property ${propName}.`);
            return false;
        }
    }
    /**
     * delete all properties from the LocalStorage instance
     * @see delete().
     * precondition is that there are no subscribers.
     * method returns false and deletes no poperties if there is any property
     * that still has subscribers
     *
     * @since 9
     */
    clear() {
        for (let propName of this.keys()) {
            var p = this.storage_.get(propName);
            if (p.numberOfSubscrbers()) {
                stateMgmtConsole.error(`${this.constructor.name}.deleteAll: Attempt to delete property ${propName} that \
          has ${p.numberOfSubscrbers()} subscribers. Subscribers need to unsubscribe before prop deletion.
          Any @Component instance with a @StorageLink/Prop or @LocalStorageLink/Prop is a subscriber.`);
                return false;
            }
        }
        for (let propName of this.keys()) {
            var p = this.storage_.get(propName);
            p.aboutToBeDeleted();
        }
        this.storage_.clear();
        return true;
    }
    /**
     * Subscribe to value change notifications of named property
     * Any object implementing ISinglePropertyChangeSubscriber interface
     * and registerign itself to SubscriberManager can register
     * Caution: do remember to unregister, otherwise the property will block
     * cleanup, @see delete() and @see clear()
     *
     * @param propName property in LocalStorage to subscribe to
     * @param subscriber object that implements ISinglePropertyChangeSubscriber interface
     * @returns false if named property does not exist
     *
     * @since 9
     */
    subscribeToChangesOf(propName, subscriber) {
        var p = this.storage_.get(propName);
        if (p) {
            p.subscribeMe(subscriber);
            return true;
        }
        return false;
    }
    /**
     * inverse of @see subscribeToChangesOf
     * @param propName property in LocalStorage to subscribe to
     * @param subscriberId id of the subscrber passed to @see subscribeToChangesOf
     * @returns false if named property does not exist
     *
     * @since 9
     */
    unsubscribeFromChangesOf(propName, subscriberId) {
        var p = this.storage_.get(propName);
        if (p) {
            p.unlinkSuscriber(subscriberId);
            return true;
        }
        return false;
    }
    /**
     * return number of subscribers to named property
     *  useful for debug purposes
     *
     * Not a public / sdk function
    */
    numberOfSubscrbersTo(propName) {
        var p = this.storage_.get(propName);
        if (p) {
            return p.numberOfSubscrbers();
        }
        return undefined;
    }
    __createSync(storagePropName, defaultValue, factoryFunc) {
        let p = this.storage_.get(storagePropName);
        if (p == undefined) {
            // property named 'storagePropName' not yet in storage
            // add new property to storage
            if (defaultValue === undefined) {
                stateMgmtConsole.error(`${this.constructor.name}.__createSync(${storagePropName}, non-existing property and undefined default value. ERROR.`);
                return undefined;
            }
            p = this.addNewPropertyInternal(storagePropName, defaultValue);
        }
        return factoryFunc(p);
    }
}
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
/**
 *
 * AppStorage
 *
 * Class implements a Map of ObservableObjectBase UI state variables.
 * AppStorage singleton is sub-class of @see LocalStorage for
 * UI state of app-wide access and same life cycle as the app.
 *
 * @since 7
 */
class AppStorage extends LocalStorage {
    /** singleton class, app can not create instances
     *
     * not a public / sdk function
    */
    constructor(initializingProperties) {
        super(initializingProperties);
    }
    /**
     * create and initialize singleton
     * initialzie with all properties and their values that Object.keys(params) returns
     * Property values must not be undefined.
     *
     * not a public / sdk function
     */
    static CreateSingleton(initializingPropersties) {
        if (!AppStorage.Instance_) {
            
            AppStorage.Instance_ = new AppStorage(initializingPropersties);
        }
        else {
            stateMgmtConsole.error("AppStorage.CreateNewInstance(..): instance exists already, internal error!");
        }
    }
    /**
    * create and return a two-way sync "(link") to named property
    *
    * Same as @see LocalStorage.link()
    *
    * @param propName name of source property in AppStorage
     * @param linkUser IPropertySubscriber to be notified when source changes,
     * @param subscribersName the linkUser (subscriber) uses this name for the property
     *      this name will be used in propertyChange(propName) callback of IMultiPropertiesChangeSubscriber
     * @returns  SynchedPropertyTwoWay{Simple|Object| object with given LocalStoage prop as its source.
     *           Apps can use SDK functions of base class SubscribedAbstractProperty<S>
     *           return undefiend if named property does not already exist in AppStorage
     *
     * @since 7
     */
    static Link(key, linkUser, subscribersName) {
        return AppStorage.GetOrCreate().link(key, linkUser, subscribersName);
    }
    /**
    * Like @see link(), but will create and initialize a new source property in LocalStorge if missing
    *
    * Same as @see LocalStorage.setAndLink()
    *
    * @param propName name of source property in AppStorage
    * @param defaultValue value to be used for initializing if new creating new property in AppStorage
    *        default value must be of type S, must not be undefined or null.
    * @param linkUser IPropertySubscriber to be notified when return 'link' changes,
    * @param subscribersName the linkUser (subscriber) uses this name for the property
    *      this name will be used in propertyChange(propName) callback of IMultiPropertiesChangeSubscriber
    * @returns SynchedPropertyTwoWay{Simple|Object| object with given LocalStoage prop as  its source.
    *          Apps can use SDK functions of base class SubscribedAbstractProperty<S>
    *
    * @since 7
    */
    static SetAndLink(key, defaultValue, linkUser, subscribersName) {
        return AppStorage.GetOrCreate().setAndLink(key, defaultValue, linkUser, subscribersName);
    }
    /**
   * create and return a one-way sync ('prop') to named property
   *
   * Same as @see LocalStorage.prop()
   *
   * @param propName name of source property in AppStorage
   * @param propUser IPropertySubscriber to be notified when source changes,
   * @param subscribersName the linkUser (subscriber) uses this name for the property
   *      this name will be used in propertyChange(propName) callback of IMultiPropertiesChangeSubscriber
   * @returns  SynchedPropertyOneWay{Simple|Object| object with given LocalStoage prop as  its source.
   *           Apps can use SDK functions of base class SubscribedAbstractProperty<S>
   *           return undefiend if named property does not already exist in AppStorage.
   * @since 7
   */
    static Prop(propName, propUser, subscribersName) {
        return AppStorage.GetOrCreate().prop(propName, propUser, subscribersName);
    }
    /**
   * Like @see prop(), will create and initialize a new source property in AppStorage if missing
   *
   * Same as @see LocalStorage.setAndProp()
   *
   * @param propName name of source property in AppStorage
   * @param defaultValue value to be used for initializing if new creating new property in AppStorage.
   *        default value must be of type S, must not be undefined or null.
   * @param propUser IPropertySubscriber to be notified when returned 'prop' changes,
   * @param subscribersName the propUser (subscriber) uses this name for the property
   *      this name will be used in propertyChange(propName) callback of IMultiPropertiesChangeSubscriber
   * @returns  SynchedPropertyOneWay{Simple|Object| object with given LocalStoage prop as its source.
   *           Apps can use SDK functions of base class SubscribedAbstractProperty<S>
   *
   * @since 7
   */
    static SetAndProp(key, defaultValue, propUser, subscribersName) {
        return AppStorage.GetOrCreate().setAndProp(key, defaultValue, propUser, subscribersName);
    }
    /**
     * Check if AppStorge has a property with given name
     * return true if prooperty with given name exists
     * same as ES6 Map.prototype.has()
     *
     * Same as @see LocalStorage.has()
     *
     * @param propName searched property
     * @returns true if property with such name exists in AppStorage
     *
     * @since 7
     */
    static Has(key) {
        return AppStorage.GetOrCreate().has(key);
    }
    /**
    * Returns value of given property
    * return undefined if no property with this name
    *
    *
    * @Same as see LocalStorage.get()
    *
    * @param propName
    * @returns property value if found or undefined
    *
    */
    static Get(key) {
        return AppStorage.GetOrCreate().get(key);
    }
    /**
    * Set value of given property in AppStorage
    * Methosd sets nothing and returns false if property with this name does not exist
    * or if newValue is `undefined` or `null` (`undefined`, `null` value are not allowed for state variables).
    *
    * Same as @see LocalStorage.set
    *
    * @param propName
    * @param newValue must be of type T and must not be undefined or null
    * @returns true on success, i.e. when above conditions are satisfied, otherwise false
    *
    * @since 7
    */
    static Set(key, newValue) {
        return AppStorage.GetOrCreate().set(key, newValue);
    }
    /**
   * Set value of given property, if it exists, @see set() .
   * Add property if no property with given name and initialize with given value.
   * Do nothing and return false if newValuue is undefined or null
   * (undefined, null value is not allowed for state variables)
   *
   * @see LocalStorage.setOrCreate()
   *
   * @param propName
   * @param newValue must be of type T and must not be undefined or null
   * @returns true on success, i.e. when above conditions are satisfied, otherwise false
   *
   * @since 7
   */
    static SetOrCreate(key, newValue) {
        AppStorage.GetOrCreate().setOrCreate(key, newValue);
    }
    /**
     * Delete property from StorageBase
     * Use with caution:
     * Before deleting a prop from AppStorage all its subscribers need to
     * unsubscribe from the property.
     * This method fails and returns false if given property still has subscribers
     * Another reason for failing is unkmown property.
     *
     * Developer advise:
     * Subscribers are created with @see link(), @see prop()
     * and also via @LocalStorageLink and @LocalStorageProp state variable decorators.
     * That means as long as their is a @Component instance that uses such decorated variable
     * or a sync relationship with a SubscribedAbstractProperty variable the property can nit
     * (and also should not!) be deleted from AppStorage.
     *
     * Same as @see LocalStorage.delete()
     *
     * @param propName
     * @returns false if method failed
     *
     * @since 7
    */
    static Delete(key) {
        return AppStorage.GetOrCreate().delete(key);
    }
    /**
    * Provide names of all properties in AppStorage
    * same as ES6 Map.prototype.keys()
    *
    * Same as @see LocalStorage.keys()
    *
    * @returns return a Map Iterator
    *
    * @since 7
   */
    static Keys() {
        return AppStorage.GetOrCreate().keys();
    }
    /**
     * Returns number of properties in AppStorage
     * same as Map.prototype.size()
     *
     * Same as @see LocalStorage.size()
     *
     * @param propName
     * @returns return number of properties
     *
     * @since 7
     */
    static Size() {
        return AppStorage.GetOrCreate().size();
    }
    /**
     * delete all properties from the AppStorage
     *
     * @see delete(), same as @see LocalStorage.clear()
     *
     * precondition is that there are no subscribers.
     * method returns false and deletes no poperties if there is any property
     * that still has subscribers
     *
     * @since 7
     */
    static Clear() {
        return AppStorage.GetOrCreate().clear();
    }
    /**
     * Same as @see Clear().
     *
     * @since 7, depreciated, used Clear() instead!
     *
     */
    static StaticClear() {
        return AppStorage.Clear();
    }
    /**
    * not a public / sdk function
    */
    static AboutToBeDeleted() {
        AppStorage.GetOrCreate().aboutToBeDeleted();
    }
    /**
     * return number of subscribers to named property
     * useful for debug purposes
     *
     * not a public / sdk function
    */
    static NumberOfSubscribersTo(propName) {
        return AppStorage.GetOrCreate().numberOfSubscrbersTo(propName);
    }
    /**
    * Subscribe to value change notifications of named property
    * Any object implementing ISinglePropertyChangeSubscriber interface
    * and registerign itself to SubscriberManager can register
    * Caution: do remember to unregister, otherwise the property will block
    * cleanup, @see delete() and @see clear()
    *
    * Same as @see LocalStorage.subscribeToChangesOf()
    *
    * @param propName property in AppStorage to subscribe to
    * @param subscriber object that implements ISinglePropertyChangeSubscriber interface
    * @returns false if named property does not exist
    *
    * @since 7
    */
    static SubscribeToChangesOf(propName, subscriber) {
        return AppStorage.GetOrCreate().subscribeToChangesOf(propName, subscriber);
    }
    /**
     * inverse of @see SubscribeToChangesOf,
     * same as @see LocalStorage.subscribeToChangesOf()
     *
     * @param propName property in AppStorage to subscribe to
     * @param subscriberId id of the subscrber passed to @see subscribeToChangesOf
     * @returns false if named property does not exist
     *
     * @since 7
     */
    static UnsubscribeFromChangesOf(propName, subscriberId) {
        return AppStorage.GetOrCreate().unsubscribeFromChangesOf(propName, subscriberId);
    }
    /**
     * Unimplemenrted, currently all properties of AppStorage are mutable.
     *
     * @since 7, depreciated
     */
    static IsMutable(key) {
        return true;
    }
    /**
     * not a public / sdk function
     */
    static __CreateSync(storagePropName, defaultValue, factoryFunc) {
        return AppStorage.GetOrCreate().__createSync(storagePropName, defaultValue, factoryFunc);
    }
    /**
     * not a public / sdk function
     */
    static GetOrCreate() {
        if (!AppStorage.Instance_) {
            stateMgmtConsole.warn("AppStorage instance missing. Use AppStorage.CreateInstance(initObj). Creating instance without any initialization.");
            AppStorage.Instance_ = new AppStorage({});
        }
        return AppStorage.Instance_;
    }
}
// instance functions below:
// Should all be protected, but TS lang does not allow access from static member to protected member
AppStorage.Instance_ = undefined;
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
 * Singleton class SubscriberManager implements IPropertySubscriberLookup
 * public API to manage IPropertySubscriber
 */
class SubscriberManager {
    /**
     * SubscriberManager is a singleton created by the framework
     * do not use
     *
     * internal method
     */
    constructor() {
        this.subscriberById_ = new Map();
        
    }
    /**
      * check subscriber is known
      * same as ES6 Map.prototype.has()
      *
      * @since 9
      */
    static Has(id) {
        return SubscriberManager.GetInstance().has(id);
    }
    /**
     *
     * retrieve subscriber by id
     * same as ES6 Map.prototype.get()
     *
     *  @since 9
     */
    static Find(id) {
        return SubscriberManager.GetInstance().get(id);
    }
    /**
     * unregister a subscriber
     * same as ES6 Map.prototype.delete()
     * @return boolean success or failure to delete
     *
     *  @since 9
     */
    static Delete(id) {
        return SubscriberManager.GetInstance().delete(id);
    }
    /**
    * add a new subscriber.
    * The subscriber must have a new (unused) id (@see MakeId() )
    * for add() to succeed.
    * same as Map.prototype.set()
    *
    *  @since 9
    */
    static Add(newSubsriber) {
        return SubscriberManager.GetInstance().add(newSubsriber);
    }
    /**
    *
    * @returns a globally unique id to be assigned to a IPropertySubscriber objet
    * Use MakeId() to assign a IPropertySubscriber object an id before calling @see add() .
    *
    *  @since 9
   */
    static MakeId() {
        return SubscriberManager.GetInstance().makeId();
    }
    /**
     * Check number of registered Subscriber / registered IDs.
     * @returns number of registered unique ids.
     *
     *  @since 9
     */
    static NumberOfSubscribers() {
        return SubscriberManager.GetInstance().numberOfSubscribers();
    }
    /**
     *
     * internal (non-SDK) methods below
     *
    */
    /**
   * Get singleton, create it on first call
   * @returns SubscriberManager singleton
   *
   * internal function
   * This function will be removed soon, use static functions instead!
   * Note: Fnction gets used by transpiler output for both full update and partial update
   */
    static Get() {
        if (!SubscriberManager.instance_) {
            SubscriberManager.instance_ = new SubscriberManager();
        }
        return SubscriberManager.instance_;
    }
    /**
     * Get singleton, create it on first call
     * @returns SubscriberManager singleton
     *
     * internal function
     */
    static GetInstance() {
        if (!SubscriberManager.instance_) {
            SubscriberManager.instance_ = new SubscriberManager();
        }
        return SubscriberManager.instance_;
    }
    /**
     * for debug purposes dump all known subscriber's info to comsole
     *
     * not a public / sdk function
     */
    static DumpSubscriberInfo() {
        SubscriberManager.GetInstance().dumpSubscriberInfo();
    }
    /**
     * not a public / sdk function
     * @see Has
     */
    has(id) {
        return this.subscriberById_.has(id);
    }
    /**
     * not a public / sdk function
     * @see Get
     */
    get(id) {
        return this.subscriberById_.get(id);
    }
    /**
   * not a public / sdk function
   * @see Delete
   */
    delete(id) {
        if (!this.has(id)) {
            stateMgmtConsole.warn(`SubscriberManager.delete unknown id ${id} `);
            return false;
        }
        return this.subscriberById_.delete(id);
    }
    /**
   * not a public / sdk function
   * @see Add
   */
    add(newSubsriber) {
        if (this.has(newSubsriber.id__())) {
            return false;
        }
        this.subscriberById_.set(newSubsriber.id__(), newSubsriber);
        return true;
    }
    /**
     * Method for testing purposes
     * @returns number of subscribers
     *
     * not a public / sdk function
     */
    numberOfSubscribers() {
        return this.subscriberById_.size;
    }
    /**
     * for debug purposes dump all known subscriber's info to comsole
     *
     * not a public / sdk function
     */
    dumpSubscriberInfo() {
        
        for (let [id, subscriber] of this.subscriberById_) {
            
        }
        
    }
    /**
     *
     * @returns a globally unique id to be assigned to a Subscriber
     */
    makeId() {
        return ViewStackProcessor.MakeUniqueId();
    }
}
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
 *
 *   SubscribedAbstractProperty is base class of ObservedPropertyAbstract
 *   and includes these 3 functions that are part of the SDK.
 *
 *   SubscribedAbstractProperty<T> is the return type of
 *   - AppStorage static functions Link(), Prop(), SetAndLink(), and SetAndProp()
 *   - LocalStorage methods link(), prop(), setAndLink(), and setAndProp()
 *
 *   'T' can be boolean, string, number or custom class.
 *
 * Main functions
 *   @see get() reads the linked AppStorage/LocalStorage property value,
 *   @see set(newValue) write a new value to the synched AppStorage/LocalStorage property value
 *   @see aboutToBeDeleted() ends the sync relationship with the AppStorage/LocalStorage property
 *        The app must call this function before the SubscribedAbstractProperty<T> object
 *        goes out of scope.
 *
 * @since 7
*/
class SubscribedAbstractProperty {
}
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
/**
 *
 * SubscriableAbstract
 *
 * This class is part of the SDK.
 * @since 9
 *
 * SubscriableAbstract is an abstract class that manages subscribers
 * to value changes. These subscribers are the implementation of
 * @State, @Link, @Provide, @Consume decorated variables inside the
 * framework. Each using @State, @Link, etc., decorated varibale in
 * a @Component will make its own subscription. When the component
 * is created the subscription is added, and before the component
 * is deleted it unsubscribes
 *
 * An application may extend SubscriableAbstract for a custom class
 * that manages state data. @State, @Link, @Provide, @Consume
 * decorated variables can hold an Object that is instance of
 * SubscribaleAbstract.
 *
 * About lifecycle: It is legal use for two @Components with two @State
 * decorated variables to share the same SubscribaleAbstract object.
 * Each such decorated variable implementation makes its own
 * subscription to the SubscribaleAbstract object. Hence, when both variables
 * have unsubscribed the SubscribaleAbstract custom class may do its own
 * de-initilialization, e.g. release held external resources.
 *
 * How to extend:
 * A subclass manages the get and set to one or several properties on its own.
 * The subclass needs to notify all relevant value changes to the framework for the
 * UI to be updated. Notification should only be given for class properties that
 * are used to generate the UI.
 *
 * A subclass must call super() in its constructor to let this base class
 * initialize itself.
 *
 * A subclass must call 'notifyPropertyHasChanged*(' after the relevant property
 * has changes. The framework will notify all dependent components to re-render.
 *
 * A sub-class may overwrite the 'addOwningProperty' function to add own
 * functionality, but it must call super.addowningOwningProperty(..). E.g.
 * the sub-class could connect to external resources upon the first subscriber.
 *
 * A sub-class may also overwrite the 'removeOwningProperty' function or
 * 'removeOwningPropertyById' function to add own functionality,
 * but it must call super.removeOwningProperty(..).
 * E.g. the sub-class could release held external resources upon loosing the
 * last subscriber.
 *
 */
class SubscribaleAbstract {
    /**
     * make sure to call super() from subclass constructor!
     *
     * @since 9
     */
    constructor() {
        this.owningProperties_ = new Set();
        
    }
    /**
    * A subsclass must call this function whenever one of its properties has
     * changed that is used to construct the UI.
     * @param propName name of the change property
     * @param newValue the property value after the change
     *
     * @since 9
     */
    notifyPropertyHasChanged(propName, newValue) {
        
        this.owningProperties_.forEach((subscribedId) => {
            var owningProperty = SubscriberManager.Find(subscribedId);
            if (owningProperty) {
                if ('objectPropertyHasChangedPU' in owningProperty) {
                    // PU code path
                    owningProperty.objectPropertyHasChangedPU(this, propName);
                }
                // FU code path
                if ('hasChanged' in owningProperty) {
                    owningProperty.hasChanged(newValue);
                }
                if ('propertyHasChanged' in owningProperty) {
                    owningProperty.propertyHasChanged(propName);
                }
            }
            else {
                stateMgmtConsole.error(`SubscribaleAbstract: notifyHasChanged: unknown subscriber.'${subscribedId}' error!.`);
            }
        });
    }
    /**
     * Method used by the framework to add subscribing decorated variables
     * Subclass may overwrite this function but must call the function of the base
     * class from its own implementation.
     * @param subscriber new subscriber that implements ISinglePropertyChangeSubscriber
     * and/or IMultiPropertiesChangeSubscriber interfaces
     *
     * @since 9
     */
    addOwningProperty(subscriber) {
        
        this.owningProperties_.add(subscriber.id__());
    }
    /**
     * Method used by the framework to ubsubscribing decorated variables
     * Subclass may overwrite this function but must call the function of the base
     * class from its own implementation.
     * @param subscriber subscriber that implements ISinglePropertyChangeSubscriber
     * and/or IMultiPropertiesChangeSubscriber interfaces
     *
     * @since 9
     */
    removeOwningProperty(property) {
        return this.removeOwningPropertyById(property.id__());
    }
    /**
     * Same as @see removeOwningProperty() but by Subscriber id.
     * @param subscriberId
    *
    * @since 9
     */
    removeOwningPropertyById(subscriberId) {
        
        this.owningProperties_.delete(subscriberId);
    }
}
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 * PersistentStorage
 *
 * Keeps current values of select AppStorage property properties persisted to file.
 *
 * since 9
 */
class PersistentStorage {
    /**
     * all following methods are framework internal
     */
    constructor() {
        this.links_ = new Map();
        this.id_ = SubscriberManager.MakeId();
        SubscriberManager.Add(this);
    }
    /**
     *
     * @param storage method to be used by the framework to set the backend
     * this is to be done during startup
     *
     * internal function, not part of the SDK
     *
     */
    static ConfigureBackend(storage) {
        PersistentStorage.Storage_ = storage;
    }
    /**
     * private, use static functions!
     */
    static GetOrCreate() {
        if (PersistentStorage.Instance_) {
            // already initialized
            return PersistentStorage.Instance_;
        }
        PersistentStorage.Instance_ = new PersistentStorage();
        return PersistentStorage.Instance_;
    }
    /**
     *
     * internal function, not part of the SDK
     */
    static AboutToBeDeleted() {
        if (!PersistentStorage.Instance_) {
            return;
        }
        PersistentStorage.GetOrCreate().aboutToBeDeleted();
        PersistentStorage.Instance_ = undefined;
    }
    /**
     * Add property 'key' to AppStorage properties whose current value will be
     * persistemt.
     * If AppStorage does not include this property it will be added and initializes
     * with given value
     *
     * @since 9
     *
     * @param key property name
     * @param defaultValue If AppStorage does not include this property it will be initialized with this value
     *
     */
    static PersistProp(key, defaultValue) {
        PersistentStorage.GetOrCreate().persistProp(key, defaultValue);
    }
    /**
     * Reverse of @see PersistProp
     * @param key no longer persist the property named key
     *
     * @since 9
     */
    static DeleteProp(key) {
        PersistentStorage.GetOrCreate().deleteProp(key);
    }
    /**
     * Persist given AppStorage properties with given names.
     * If a property does not exist in AppStorage, add it and initialize it with given value
     * works as @see PersistProp for multiple properties.
     *
     * @param properties
     *
     * @since 9
     *
     */
    static PersistProps(properties) {
        PersistentStorage.GetOrCreate().persistProps(properties);
    }
    /**
     * Inform persisted AppStorage property names
     * @returns array of AppStorage keys
     *
     * @since 9
     */
    static Keys() {
        let result = [];
        const it = PersistentStorage.GetOrCreate().keys();
        let val = it.next();
        while (!val.done) {
            result.push(val.value);
            val = it.next();
        }
        return result;
    }
    /**
      * This methid offers a way to force writing the property value with given
      * key to persistent storage.
      * In the general case this is unnecessary as the framework observed changes
      * and triggers writing to disk by itself. For nested objects (e.g. array of
      * objects) however changes of a property of a property as not observed. This
      * is the case where the application needs to signal to the framework.
      *
      * @param key property that has changed
      *
      * @since 9
      *
      */
    static NotifyHasChanged(propName) {
        
        PersistentStorage.Storage_.set(propName, PersistentStorage.GetOrCreate().links_.get(propName).get());
    }
    keys() {
        return this.links_.keys();
    }
    persistProp(propName, defaultValue) {
        if (this.persistProp1(propName, defaultValue)) {
            // persist new prop
            
            PersistentStorage.Storage_.set(propName, this.links_.get(propName).get());
        }
    }
    // helper function to persist a property
    // does everything except writing prop to disk
    persistProp1(propName, defaultValue) {
        if (defaultValue == null || defaultValue == undefined) {
            stateMgmtConsole.error(`PersistentStorage: persistProp for ${propName} called with 'null' or 'undefined' default value!`);
            return false;
        }
        if (this.links_.get(propName)) {
            stateMgmtConsole.warn(`PersistentStorage: persistProp: ${propName} is already persisted`);
            return false;
        }
        let link = AppStorage.Link(propName, this);
        if (link) {
            
            this.links_.set(propName, link);
        }
        else {
            let newValue = PersistentStorage.Storage_.get(propName);
            let returnValue;
            if (!newValue) {
                
                returnValue = defaultValue;
            }
            else {
                returnValue = newValue;
            }
            link = AppStorage.SetAndLink(propName, returnValue, this);
            this.links_.set(propName, link);
            
        }
        return true;
    }
    persistProps(properties) {
        properties.forEach(property => this.persistProp1(property.key, property.defaultValue));
        this.write();
    }
    deleteProp(propName) {
        let link = this.links_.get(propName);
        if (link) {
            link.aboutToBeDeleted();
            this.links_.delete(propName);
            PersistentStorage.Storage_.delete(propName);
            
        }
        else {
            stateMgmtConsole.warn(`PersistentStorage: '${propName}' is not a persisted property warning.`);
        }
    }
    write() {
        this.links_.forEach((link, propName, map) => {
            
            PersistentStorage.Storage_.set(propName, link.get());
        });
    }
    propertyHasChanged(info) {
        
        this.write();
    }
    syncPeerHasChanged(eventSource) {
        
        this.write();
    }
    // public required by the interface, use the static method instead!
    aboutToBeDeleted() {
        
        this.links_.forEach((val, key, map) => {
            
            val.aboutToBeDeleted();
        });
        this.links_.clear();
        SubscriberManager.Delete(this.id__());
        PersistentStorage.Storage_.clear();
    }
    id__() {
        return this.id_;
    }
}
PersistentStorage.Instance_ = undefined;
;
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 * Environment
 *
 * Injects device properties ("environment") into AppStorage
 *
 */
class Environment {
    constructor() {
        this.props_ = new Map();
        Environment.EnvBackend_.onValueChanged(this.onValueChanged.bind(this));
    }
    static GetOrCreate() {
        if (Environment.Instance_) {
            // already initialized
            return Environment.Instance_;
        }
        Environment.Instance_ = new Environment();
        return Environment.Instance_;
    }
    static ConfigureBackend(envBackend) {
        Environment.EnvBackend_ = envBackend;
    }
    static AboutToBeDeleted() {
        if (!Environment.Instance_) {
            return;
        }
        Environment.GetOrCreate().aboutToBeDeleted();
        Environment.Instance_ = undefined;
    }
    static EnvProp(key, value) {
        return Environment.GetOrCreate().envProp(key, value);
    }
    static EnvProps(props) {
        Environment.GetOrCreate().envProps(props);
    }
    static Keys() {
        return Environment.GetOrCreate().keys();
    }
    envProp(key, value) {
        let prop = AppStorage.Prop(key);
        if (prop) {
            stateMgmtConsole.warn(`Environment: envProp '${key}': Property already exists in AppStorage. Not using environment property.`);
            return false;
        }
        let tmp;
        switch (key) {
            case "accessibilityEnabled":
                tmp = Environment.EnvBackend_.getAccessibilityEnabled();
                break;
            case "colorMode":
                tmp = Environment.EnvBackend_.getColorMode();
                break;
            case "fontScale":
                tmp = Environment.EnvBackend_.getFontScale();
                break;
            case "fontWeightScale":
                tmp = Environment.EnvBackend_.getFontWeightScale().toFixed(2);
                break;
            case "layoutDirection":
                tmp = Environment.EnvBackend_.getLayoutDirection();
                break;
            case "languageCode":
                tmp = Environment.EnvBackend_.getLanguageCode();
                break;
            default:
                tmp = value;
        }
        prop = AppStorage.SetAndProp(key, tmp);
        this.props_.set(key, prop);
        
    }
    envProps(properties) {
        properties.forEach(property => {
            this.envProp(property.key, property.defaultValue);
            
        });
    }
    keys() {
        let result = [];
        const it = this.props_.keys();
        let val = it.next();
        while (!val.done) {
            result.push(val.value);
            val = it.next();
        }
        return result;
    }
    onValueChanged(key, value) {
        let ok = AppStorage.Set(key, value);
        if (ok) {
            
        }
        else {
            stateMgmtConsole.warn(`Environment: onValueChanged: error changing ${key}! See results above.`);
        }
    }
    aboutToBeDeleted() {
        this.props_.forEach((val, key, map) => {
            val.aboutToBeDeleted();
            AppStorage.Delete(key);
        });
    }
}
Environment.Instance_ = undefined;
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
 * state mgmt library uses its own class for logging
* allows to remap separately from other use of aceConsole
*
* everything in this file is framework internal
*/
class stateMgmtConsole {
    static log(...args) {
        aceConsole.log(...args);
    }
    static debug(...args) {
        aceConsole.debug(...args);
    }
    static info(...args) {
        aceConsole.info(...args);
    }
    static warn(...args) {
        aceConsole.warn(...args);
    }
    static error(...args) {
        aceConsole.error(...args);
    }
}
class stateMgmtTrace {
    static scopedTrace(codeBlock, arg1, ...args) {
        aceTrace.begin(arg1, ...args);
        let result = codeBlock();
        aceTrace.end();
        return result;
    }
}
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
class DistributedStorage {
    constructor(sessionId, notifier) {
        this.links_ = new Map();
        this.id_ = SubscriberManager.MakeId();
        SubscriberManager.Add(this);
        this.aviliable_ = false;
        this.notifier_ = notifier;
    }
    keys() {
        let result = [];
        const it = this.links_.keys();
        let val = it.next();
        while (!val.done) {
            result.push(val.value);
            val = it.next();
        }
        return result;
    }
    distributeProp(propName, defaultValue) {
        if (this.link(propName, defaultValue)) {
            
        }
    }
    distributeProps(properties) {
        properties.forEach(property => this.link(property.key, property.defaultValue));
    }
    link(propName, defaultValue) {
        if (defaultValue == null || defaultValue == undefined) {
            stateMgmtConsole.error(`DistributedStorage: linkProp for ${propName} called with 'null' or 'undefined' default value!`);
            return false;
        }
        if (this.links_.get(propName)) {
            stateMgmtConsole.warn(`DistributedStorage: linkProp: ${propName} is already exist`);
            return false;
        }
        let link = AppStorage.Link(propName, this);
        if (link) {
            
            this.links_.set(propName, link);
            this.setDistributedProp(propName, defaultValue);
        }
        else {
            let returnValue = defaultValue;
            if (this.aviliable_) {
                let newValue = this.getDistributedProp(propName);
                if (newValue == null) {
                    
                    this.setDistributedProp(propName, defaultValue);
                }
                else {
                    returnValue = newValue;
                }
            }
            link = AppStorage.SetAndLink(propName, returnValue, this);
            this.links_.set(propName, link);
            
        }
        return true;
    }
    deleteProp(propName) {
        let link = this.links_.get(propName);
        if (link) {
            link.aboutToBeDeleted();
            this.links_.delete(propName);
            if (this.aviliable_) {
                this.storage_.delete(propName);
            }
        }
        else {
            stateMgmtConsole.warn(`DistributedStorage: '${propName}' is not a distributed property warning.`);
        }
    }
    write(key) {
        let link = this.links_.get(key);
        if (link) {
            this.setDistributedProp(key, link.get());
        }
    }
    // public required by the interface, use the static method instead!
    aboutToBeDeleted() {
        
        this.links_.forEach((val, key, map) => {
            
            val.aboutToBeDeleted();
        });
        this.links_.clear();
        SubscriberManager.Delete(this.id__());
    }
    id__() {
        return this.id_;
    }
    propertyHasChanged(info) {
        
        this.write(info);
    }
    onDataOnChange(propName) {
        let link = this.links_.get(propName);
        let newValue = this.getDistributedProp(propName);
        if (link && newValue != null) {
            
            link.set(newValue);
        }
    }
    onConnected(status) {
        
        if (!this.aviliable_) {
            this.syncProp();
            this.aviliable_ = true;
        }
        if (this.notifier_ != null) {
            this.notifier_(status);
        }
    }
    syncProp() {
        this.links_.forEach((val, key) => {
            let newValue = this.getDistributedProp(key);
            if (newValue == null) {
                this.setDistributedProp(key, val.get());
            }
            else {
                val.set(newValue);
            }
        });
    }
    setDistributedProp(key, value) {
        if (!this.aviliable_) {
            stateMgmtConsole.warn(`DistributedStorage is not aviliable`);
            return;
        }
        stateMgmtConsole.error(`DistributedStorage value is object ${key}-${JSON.stringify(value)}`);
        if (typeof value == 'object') {
            this.storage_.set(key, JSON.stringify(value));
            return;
        }
        this.storage_.set(key, value);
    }
    getDistributedProp(key) {
        let value = this.storage_.get(key);
        if (typeof value == 'string') {
            try {
                let returnValue = JSON.parse(value);
                return returnValue;
            }
            finally {
                return value;
            }
        }
        return value;
    }
}
;
/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
/**
* @Observed class decorator
*
* usage:
*    @Observed class ClassA { ... }
*
* Causes every instance of decorated clss to be automatically wrapped inside an ObservedObject.
*
* Implemented by extending the decroaetd class by class named 'ObservableObjectClass'.
*
* It is permisstable to decorate the base and the extended class like thisNote: I
*   @Observed class ClassA { ...}
*   @Observed class ClassB extends ClassA { ... }
* and use
*   a = new ClassA();
*   b = new ClassB();
* Only one ES6 Proxy is added.
*
*
* Take note the decorator implementation extends the prototype chain.
*
* The prototype chain of a in above example is
*  - ObservableObjectClass prototype
*  - ClassA prototype
*  - Object prototype
*
* Snd the prototype chain of b is
*  - ObservableObjectClass prototype
*  - ClassB prototype
*  - ObservableObjectClass prototype
*  - ClassA prototype
*  - Object prototype
*
* The @Observed decorator is public, part of the SDK, starting from API 9.
*
*/
// define just once to get just one Symbol
const __IS_OBSERVED_PROXIED = Symbol("_____is_observed_proxied__");
function Observed(constructor_, _) {
    
    let ObservedClass = class extends constructor_ {
        constructor(...args) {
            super(...args);
            
            let isProxied = Reflect.has(this, __IS_OBSERVED_PROXIED);
            Object.defineProperty(this, __IS_OBSERVED_PROXIED, {
                value: true,
                enumerable: false,
                configurable: false,
                writable: false
            });
            if (isProxied) {
                
                return this;
            }
            else {
                
                return ObservedObject.createNewInternal(this, undefined);
            }
        }
    };
    return ObservedClass;
}
// force tsc to generate the __decorate data structure needed for @Observed
// tsc will not generate unless the @Observed class decorator is used at least once
let __IGNORE_FORCE_decode_GENERATION__ = class __IGNORE_FORCE_decode_GENERATION__ {
};
__IGNORE_FORCE_decode_GENERATION__ = __decorate([
    Observed
], __IGNORE_FORCE_decode_GENERATION__);
/**
 * class ObservedObject and supporting Handler classes,
 * Extends from ES6 Proxy. In adding to 'get' and 'set'
 * the clasess manage subscribers that receive notification
 * about proxies object being 'read' or 'changed'.
 *
 * These classes are framework internal / non-SDK
 *
 */
class SubscribableHandler {
    constructor(owningProperty) {
        this.owningProperties_ = new Set();
        if (owningProperty) {
            this.addOwningProperty(owningProperty);
        }
        
    }
    addOwningProperty(subscriber) {
        if (subscriber) {
            
            this.owningProperties_.add(subscriber.id__());
        }
        else {
            stateMgmtConsole.warn(`SubscribableHandler: addOwningProperty: undefined subscriber. - Internal error?`);
        }
    }
    /*
        the inverse function of createOneWaySync or createTwoWaySync
      */
    removeOwningProperty(property) {
        return this.removeOwningPropertyById(property.id__());
    }
    removeOwningPropertyById(subscriberId) {
        
        this.owningProperties_.delete(subscriberId);
    }
    notifyObjectPropertyHasChanged(propName, newValue) {
        
        this.owningProperties_.forEach((subscribedId) => {
            var owningProperty = SubscriberManager.Find(subscribedId);
            if (owningProperty) {
                if ('objectPropertyHasChangedPU' in owningProperty) {
                    // PU code path
                    owningProperty.objectPropertyHasChangedPU(this, propName);
                }
                // FU code path
                if ('hasChanged' in owningProperty) {
                    owningProperty.hasChanged(newValue);
                }
                if ('propertyHasChanged' in owningProperty) {
                    owningProperty.propertyHasChanged(propName);
                }
            }
            else {
                stateMgmtConsole.warn(`SubscribableHandler: notifyObjectPropertyHasChanged: unknown subscriber.'${subscribedId}' error!.`);
            }
        });
    }
    notifyObjectPropertyHasBeenRead(propName) {
        
        this.owningProperties_.forEach((subscribedId) => {
            var owningProperty = SubscriberManager.Find(subscribedId);
            if (owningProperty) {
                // PU code path
                if ('objectPropertyHasBeenReadPU' in owningProperty) {
                    owningProperty.objectPropertyHasBeenReadPU(this, propName);
                }
            }
        });
    }
    has(target, property) {
        
        return (property === ObservedObject.__IS_OBSERVED_OBJECT) ? true : Reflect.has(target, property);
    }
    get(target, property, receiver) {
        
        return (property === ObservedObject.__OBSERVED_OBJECT_RAW_OBJECT) ? target : Reflect.get(target, property, receiver);
    }
    set(target, property, newValue) {
        switch (property) {
            case SubscribableHandler.SUBSCRIBE:
                // assignment obsObj[SubscribableHandler.SUBSCRCRIBE] = subscriber
                this.addOwningProperty(newValue);
                return true;
                break;
            case SubscribableHandler.UNSUBSCRIBE:
                // assignment obsObj[SubscribableHandler.UNSUBSCRCRIBE] = subscriber
                this.removeOwningProperty(newValue);
                return true;
                break;
            default:
                if (Reflect.get(target, property) == newValue) {
                    return true;
                }
                
                Reflect.set(target, property, newValue);
                this.notifyObjectPropertyHasChanged(property.toString(), newValue);
                return true;
                break;
        }
        // unreachable
        return false;
    }
}
SubscribableHandler.SUBSCRIBE = Symbol("_____subscribe__");
SubscribableHandler.UNSUBSCRIBE = Symbol("_____unsubscribe__");
class SubscribableDateHandler extends SubscribableHandler {
    constructor(owningProperty) {
        super(owningProperty);
    }
    /**
     * Get trap for Date type proxy
     * Functions that modify Date in-place are intercepted and replaced with a function
     * that executes the original function and notifies the handler of a change.
     * @param target Original Date object
     * @param property
     * @returns
     */
    get(target, property) {
        const dateSetFunctions = new Set(["setFullYear", "setMonth", "setDate", "setHours", "setMinutes", "setSeconds",
            "setMilliseconds", "setTime", "setUTCFullYear", "setUTCMonth", "setUTCDate", "setUTCHours", "setUTCMinutes",
            "setUTCSeconds", "setUTCMilliseconds"]);
        let ret = super.get(target, property);
        if (typeof ret === "function" && property.toString() && dateSetFunctions.has(property.toString())) {
            const self = this;
            return function () {
                // execute original function with given arguments
                let result = ret.apply(this, arguments);
                self.notifyObjectPropertyHasChanged(property.toString(), this);
                return result;
            }.bind(target); // bind "this" to target inside the function
        }
        else if (typeof ret === "function") {
            ret = ret.bind(target);
        }
        return ret;
    }
}
class ExtendableProxy {
    constructor(obj, handler) {
        return new Proxy(obj, handler);
    }
}
class ObservedObject extends ExtendableProxy {
    /**
     * To create a new ObservableObject use CreateNew function
     *
     * constructor create a new ObservableObject and subscribe its owner to propertyHasChanged
     * notifications
     * @param obj  raw Object, if obj is a ObservableOject throws an error
     * @param objectOwner
     */
    constructor(obj, handler, objectOwningProperty) {
        super(obj, handler);
        if (ObservedObject.IsObservedObject(obj)) {
            stateMgmtConsole.error("ObservableOject constructor: INTERNAL ERROR: after jsObj is observedObject already");
        }
        if (objectOwningProperty != undefined) {
            this[SubscribableHandler.SUBSCRIBE] = objectOwningProperty;
        }
    } // end of constructor
    /**
     * Factory function for ObservedObjects /
     *  wrapping of objects for proxying
     *
     * @param rawObject unproxied Object or ObservedObject
     * @param objOwner owner of this Object to sign uop for propertyChange
     *          notifications
     * @returns the rawObject if object is already an ObservedObject,
     *          otherwise the newly created ObservedObject
     */
    static createNew(rawObject, owningProperty) {
        if (rawObject === null || rawObject === undefined) {
            stateMgmtConsole.error(`ObservedObject.CreateNew, input object must not be null or undefined.`);
            return rawObject;
        }
        if (ObservedObject.IsObservedObject(rawObject)) {
            ObservedObject.addOwningProperty(rawObject, owningProperty);
            return rawObject;
        }
        return ObservedObject.createNewInternal(rawObject, owningProperty);
    }
    static createNewInternal(rawObject, owningProperty) {
        let proxiedObject = new ObservedObject(rawObject, Array.isArray(rawObject) ? new class extends SubscribableHandler {
            constructor(owningProperty) {
                super(owningProperty);
                // In-place array modification functions
                // splice is also in-place modifying function, but we need to handle separately
                this.inPlaceModifications = new Set(["copyWithin", "fill", "reverse", "sort"]);
            }
            get(target, property, receiver) {
                let ret = super.get(target, property, receiver);
                if (ret && typeof ret === "function") {
                    const self = this;
                    const prop = property.toString();
                    // prop is the function name here
                    if (prop == "splice") {
                        // 'splice' self modifies the array, returns deleted array items
                        // means, alike other self-modifying functions, splice does not return the array itself.
                        return function () {
                            const result = ret.apply(target, arguments);
                            // prop is the function name here
                            // and result is the function return value
                            // functinon modifies none or more properties
                            self.notifyObjectPropertyHasChanged(prop, target);
                            return result;
                        }.bind(proxiedObject);
                    }
                    if (self.inPlaceModifications.has(prop)) {
                        // in place modfication function result == target, the raw array modified
                        
                        return function () {
                            const result = ret.apply(target, arguments);
                            // 'result' is the unproxied object               
                            // functinon modifies none or more properties
                            self.notifyObjectPropertyHasChanged(prop, result);
                            // returning the 'proxiedObject' ensures that when chain calls also 2nd function call
                            // operates on the proxied object.
                            return proxiedObject;
                        }.bind(proxiedObject);
                    }
                    // binding the proxiedObject ensures that modifying functions like push() operate on the 
                    // proxied array and each array change is notified.
                    return ret.bind(proxiedObject);
                }
                return ret;
            }
        }(owningProperty) // SubscribableArrayHandlerAnonymous
            : (rawObject instanceof Date)
                ? new SubscribableDateHandler(owningProperty)
                : new SubscribableHandler(owningProperty), owningProperty);
        return proxiedObject;
    }
    /*
      Return the unproxied object 'inside' the ObservedObject / the ES6 Proxy
      no set observation, no notification of changes!
      Use with caution, do not store any references
    */
    static GetRawObject(obj) {
        return !ObservedObject.IsObservedObject(obj) ? obj : obj[ObservedObject.__OBSERVED_OBJECT_RAW_OBJECT];
    }
    /**
     *
     * @param obj anything
     * @returns true if the parameter is an Object wrpped with a ObservedObject
     * Note: Since ES6 Proying is transparent, 'instance of' will not work. Use
     * this static function instead.
     */
    static IsObservedObject(obj) {
        return (obj && (typeof obj === "object") && Reflect.has(obj, ObservedObject.__IS_OBSERVED_OBJECT));
    }
    /**
     * add a subscriber to given ObservedObject
     * due to the proxy nature this static method approach needs to be used instead of a member
     * function
     * @param obj
     * @param subscriber
     * @returns false if given object is not an ObservedObject
     */
    static addOwningProperty(obj, subscriber) {
        if (!ObservedObject.IsObservedObject(obj) || subscriber == undefined) {
            return false;
        }
        obj[SubscribableHandler.SUBSCRIBE] = subscriber;
        return true;
    }
    /**
     * remove a subscriber to given ObservedObject
     * due to the proxy nature this static method approach needs to be used instead of a member
     * function
     * @param obj
     * @param subscriber
     * @returns false if given object is not an ObservedObject
     */
    static removeOwningProperty(obj, subscriber) {
        if (!ObservedObject.IsObservedObject(obj)) {
            return false;
        }
        obj[SubscribableHandler.UNSUBSCRIBE] = subscriber;
        return true;
    }
    /**
     * Utility function for debugging the prototype chain of given Object
     * The given object can be any Object, it is not required to be an ObservedObject
     * @param object
     * @returns multi-line string containing info about the prototype chain
     * on class in class hiararchy per line
     */
    static tracePrototypeChainOfObject(object) {
        let proto = Object.getPrototypeOf(object);
        let result = "";
        let sepa = "";
        while (proto) {
            result += `${sepa}${ObservedObject.tracePrototype(proto)}`;
            proto = Object.getPrototypeOf(proto);
            sepa = ",\n";
        }
        return result;
    }
    /**
     * Utility function for debugging all functions of given Prototype.
     * @returns string containing containing names of all functions and members of given Prototype
     */
    static tracePrototype(proto) {
        if (!proto) {
            return "";
        }
        let result = `${proto.constructor && proto.constructor.name ? proto.constructor.name : '<no class>'}: `;
        let sepa = "";
        for (let name of Object.getOwnPropertyNames(proto)) {
            result += `${sepa}${name}`;
            sepa = ", ";
        }
        ;
        return result;
    }
    /**
     * @Observed  decorator extends the decorated class. This function returns the prototype of the decorated class
     * @param proto
     * @returns prototype of the @Observed decorated class or 'proto' parameter if not  @Observed decorated
     */
    static getPrototypeOfObservedClass(proto) {
        return (proto.constructor && proto.constructor.name == "ObservedClass")
            ? Object.getPrototypeOf(proto.constructor.prototype)
            : proto;
    }
}
ObservedObject.__IS_OBSERVED_OBJECT = Symbol("_____is_observed_object__");
ObservedObject.__OBSERVED_OBJECT_RAW_OBJECT = Symbol("_____raw_object__");
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
/*
   manage subscriptions to a property
   managing the property is left to sub
   classes
   Extended by ObservedProperty, SyncedPropertyOneWay
   and SyncedPropertyTwoWay
*/
class ObservedPropertyAbstract extends SubscribedAbstractProperty {
    constructor(subscribeMe, info) {
        super();
        this.subscribers_ = new Set();
        this.id_ = SubscriberManager.MakeId();
        SubscriberManager.Add(this);
        if (subscribeMe) {
            this.subscribers_.add(subscribeMe.id__());
        }
        if (info) {
            this.info_ = info;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Delete(this.id__());
    }
    id__() {
        return this.id_;
    }
    info() {
        return this.info_;
    }
    setInfo(propName) {
        if (propName && propName != "") {
            this.info_ = propName;
        }
    }
    // Partial Update "*PU" classes will overwrite
    getUnmonitored() {
        return this.get();
    }
    subscribeMe(subscriber) {
        
        this.subscribers_.add(subscriber.id__());
    }
    /*
      the inverse function of createOneWaySync or createTwoWaySync
    */
    unlinkSuscriber(subscriberId) {
        this.subscribers_.delete(subscriberId);
    }
    notifyHasChanged(newValue) {
        
        this.subscribers_.forEach((subscribedId) => {
            var subscriber = SubscriberManager.Find(subscribedId);
            if (subscriber) {
                // FU code path
                if ('hasChanged' in subscriber) {
                    subscriber.hasChanged(newValue);
                }
                if ('propertyHasChanged' in subscriber) {
                    subscriber.propertyHasChanged(this.info_);
                }
                // PU code path, only used for ObservedPropertySimple/Object stored inside App/LocalStorage
                // ObservedPropertySimplePU/ObjectPU  used in all other PU cases, has its own notifyPropertyHasChangedPU()
                if ('syncPeerHasChanged' in subscriber) {
                    subscriber.syncPeerHasChanged(this);
                }
            }
            else {
                stateMgmtConsole.warn(`ObservedPropertyAbstract[${this.id__()}, '${this.info() || "unknown"}']: notifyHasChanged: unknown subscriber ID '${subscribedId}' error!`);
            }
        });
    }
    notifyPropertyRead() {
        
        this.subscribers_.forEach((subscribedId) => {
            var subscriber = SubscriberManager.Find(subscribedId);
            if (subscriber) {
                if ('propertyRead' in subscriber) {
                    subscriber.propertyRead(this.info_);
                }
            }
        });
    }
    /*
    return numebr of subscribers to this property
    mostly useful for unit testin
    */
    numberOfSubscrbers() {
        return this.subscribers_.size;
    }
    /**
     * provide a factory function that creates a SynchedPropertyXXXX of choice
     * that uses 'this' as source
     * @param factoryFunc
     * @returns
     */
    createSync(factoryFunc) {
        return factoryFunc(this);
    }
    /**
     * depreciated SDK function, not used anywhere by the framework
     */
    createTwoWaySync(subscribeMe, info) {
        stateMgmtConsole.warn("Using depreciated method 'createTwoWaySync'!");
        return this.createLink(subscribeMe, info);
    }
    /**
     * depreciated SDK function, not used anywhere by the framework
     */
    createOneWaySync(subscribeMe, info) {
        stateMgmtConsole.warn("Using depreciated method 'createOneWaySync' !");
        return this.createProp(subscribeMe, info);
    }
    /**
     * factory function for concrete 'object' or 'simple' ObservedProperty object
     * depending if value is Class object
     * or simple type (boolean | number | string)
     * @param value
     * @param owningView
     * @param thisPropertyName
     * @returns either
     */
    static CreateObservedObject(value, owningView, thisPropertyName) {
        return (typeof value === "object") ?
            new ObservedPropertyObject(value, owningView, thisPropertyName)
            : new ObservedPropertySimple(value, owningView, thisPropertyName);
    }
}
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 * ObservedPropertyObjectAbstract
 *
 * all definitions in this file are framework internal
 *
 * common base class of ObservedPropertyObject and
 * SyncedObjectPropertyTwoWay
 * adds the createObjectLink to the ObservedPropertyAbstract base
 */
class ObservedPropertyObjectAbstract extends ObservedPropertyAbstract {
    constructor(owningView, thisPropertyName) {
        super(owningView, thisPropertyName);
    }
}
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 *
 * ObservedPropertySimpleAbstract
 *
 * all definitions in this file are framework internal
 */
class ObservedPropertySimpleAbstract extends ObservedPropertyAbstract {
    constructor(owningView, propertyName) {
        super(owningView, propertyName);
    }
}
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
/**
 * ObservedPropertyObject
 *
 * all definitions in this file are framework internal
 *
 * class that holds an actual property value of type T
 * uses its base class to manage subscribers to this
 * property.
*/
class ObservedPropertyObject extends ObservedPropertyObjectAbstract {
    constructor(value, owningView, propertyName) {
        super(owningView, propertyName);
        this.setValueInternal(value);
    }
    aboutToBeDeleted(unsubscribeMe) {
        this.unsubscribeFromOwningProperty();
        if (unsubscribeMe) {
            this.unlinkSuscriber(unsubscribeMe.id__());
        }
        super.aboutToBeDeleted();
    }
    // notification from ObservedObject value one of its
    // props has chnaged. Implies the ObservedProperty has changed
    // Note: this function gets called when in this case:
    //       thisProp.aObsObj.aProp = 47  a object prop gets changed
    // It is NOT called when
    //    thisProp.aObsObj = new ClassA
    hasChanged(newValue) {
        
        this.notifyHasChanged(this.wrappedValue_);
    }
    unsubscribeFromOwningProperty() {
        if (this.wrappedValue_) {
            if (this.wrappedValue_ instanceof SubscribaleAbstract) {
                this.wrappedValue_.removeOwningProperty(this);
            }
            else {
                ObservedObject.removeOwningProperty(this.wrappedValue_, this);
            }
        }
    }
    /*
      actually update this.wrappedValue_
      called needs to do value change check
      and also notify with this.aboutToChange();
    */
    setValueInternal(newValue) {
        if (typeof newValue !== 'object') {
            
            return false;
        }
        this.unsubscribeFromOwningProperty();
        if (ObservedObject.IsObservedObject(newValue)) {
            
            ObservedObject.addOwningProperty(newValue, this);
            this.wrappedValue_ = newValue;
        }
        else if (newValue instanceof SubscribaleAbstract) {
            
            this.wrappedValue_ = newValue;
            this.wrappedValue_.addOwningProperty(this);
        }
        else {
            
            this.wrappedValue_ = ObservedObject.createNew(newValue, this);
        }
        return true;
    }
    get() {
        
        this.notifyPropertyRead();
        return this.wrappedValue_;
    }
    set(newValue) {
        if (this.wrappedValue_ == newValue) {
            
            return;
        }
        
        this.setValueInternal(newValue);
        this.notifyHasChanged(newValue);
    }
    /**
     * These functions are used
     *   LocalStorage.link  (also in partial update config)
     *   (FU)View.initializeConsumeinitializeConsume
     */
    createLink(subscribeOwner, linkPropName) {
        return new SynchedPropertyObjectTwoWay(this, subscribeOwner, linkPropName);
    }
    createProp(subscribeOwner, linkPropName) {
        throw new Error("Creating a 'Prop' property is unsupported for Object type property value.");
    }
}
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
/**
 * ObservedPropertySimple
 *
 * all definitions in this file are framework internal
 */
class ObservedPropertySimple extends ObservedPropertySimpleAbstract {
    constructor(value, owningView, propertyName) {
        super(owningView, propertyName);
        if (typeof value === "object") {
            throw new SyntaxError("ObservedPropertySimple value must not be an object");
        }
        this.setValueInternal(value);
    }
    aboutToBeDeleted(unsubscribeMe) {
        if (unsubscribeMe) {
            this.unlinkSuscriber(unsubscribeMe.id__());
        }
        super.aboutToBeDeleted();
    }
    hasChanged(newValue) {
        
        this.notifyHasChanged(this.wrappedValue_);
    }
    /*
      actually update this.wrappedValue_
      called needs to do value change check
      and also notify with this.aboutToChange();
    */
    setValueInternal(newValue) {
        
        this.wrappedValue_ = newValue;
    }
    get() {
        
        this.notifyPropertyRead();
        return this.wrappedValue_;
    }
    set(newValue) {
        if (this.wrappedValue_ == newValue) {
            
            return;
        }
        
        this.setValueInternal(newValue);
        this.notifyHasChanged(newValue);
    }
    /**
   * These functions are meant for use in connection with the App Stoage and
   * business logic implementation.
   * the created Link and Prop will update when 'this' property value
   * changes.
   */
    createLink(subscribeOwner, linkPropName) {
        return ((subscribeOwner !== undefined) && ("rerender" in subscribeOwner)) ?
            new SynchedPropertySimpleTwoWayPU(this, subscribeOwner, linkPropName) :
            new SynchedPropertySimpleTwoWay(this, subscribeOwner, linkPropName);
    }
    createProp(subscribeOwner, linkPropName) {
        return new SynchedPropertySimpleOneWaySubscribing(this, subscribeOwner, linkPropName);
    }
}
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 * SynchedPropertyObjectTwoWay
 *
 * all definitions in this file are framework internal
 */
class SynchedPropertyObjectTwoWay extends ObservedPropertyObjectAbstract {
    constructor(linkSource, owningChildView, thisPropertyName) {
        super(owningChildView, thisPropertyName);
        this.changeNotificationIsOngoing_ = false;
        this.linkedParentProperty_ = linkSource;
        if (this.linkedParentProperty_) {
            // register to the parent property
            this.linkedParentProperty_.subscribeMe(this);
        }
        // register to the ObservedObject
        ObservedObject.addOwningProperty(this.getObject(), this);
    }
    /*
    like a destructor, need to call this before deleting
    the property.
    */
    aboutToBeDeleted() {
        if (this.linkedParentProperty_) {
            // unregister from parent of this link
            this.linkedParentProperty_.unlinkSuscriber(this.id__());
            // unregister from the ObservedObject
            ObservedObject.removeOwningProperty(this.getObject(), this);
        }
        super.aboutToBeDeleted();
    }
    getObject() {
        this.notifyPropertyRead();
        return (this.linkedParentProperty_ ? this.linkedParentProperty_.get() : undefined);
    }
    setObject(newValue) {
        if (this.linkedParentProperty_) {
            this.linkedParentProperty_.set(newValue);
        }
    }
    // this object is subscriber to ObservedObject
    // will call this cb function when property has changed
    hasChanged(newValue) {
        if (!this.changeNotificationIsOngoing_) {
            
            this.notifyHasChanged(this.getObject());
        }
    }
    // get 'read through` from the ObservedProperty
    get() {
        
        return this.getObject();
    }
    // set 'writes through` to the ObservedProperty
    set(newValue) {
        if (this.getObject() == newValue) {
            
            return;
        }
        
        ObservedObject.removeOwningProperty(this.getObject(), this);
        // the purpose of the changeNotificationIsOngoing_ is to avoid 
        // circular notifications @Link -> source @State -> other but alos same @Link
        this.changeNotificationIsOngoing_ = true;
        this.setObject(newValue);
        ObservedObject.addOwningProperty(this.getObject(), this);
        this.notifyHasChanged(newValue);
        this.changeNotificationIsOngoing_ = false;
    }
    /**
   * These functions are meant for use in connection with the App Stoage and
   * business logic implementation.
   * the created Link and Prop will update when 'this' property value
   * changes.
   */
    createLink(subscribeOwner, linkPropName) {
        return new SynchedPropertyObjectTwoWay(this, subscribeOwner, linkPropName);
    }
    createProp(subscribeOwner, linkPropName) {
        throw new Error("Creating a 'Prop' property is unsupported for Object type property value.");
    }
}
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
/**
 * SynchedPropertySimpleOneWay
 *
 * all definitions in this file are framework internal
 */
class SynchedPropertySimpleOneWay extends ObservedPropertySimpleAbstract {
    constructor(value, subscribeMe, info) {
        super(subscribeMe, info);
        // add a test here that T is a simple type
        this.wrappedValue_ = value;
    }
    /*
      like a destructor, need to call this before deleting
      the property.
    */
    aboutToBeDeleted() {
        super.aboutToBeDeleted();
    }
    // get 'read through` from the ObservedProperty
    get() {
        
        this.notifyPropertyRead();
        return this.wrappedValue_;
    }
    set(newValue) {
        if (this.wrappedValue_ == newValue) {
            
            return;
        }
        
        this.wrappedValue_ = newValue;
        this.notifyHasChanged(newValue);
    }
    /**
     * These functions are meant for use in connection with the App Stoage and
     * business logic implementation.
     * the created Link and Prop will update when 'this' property value
     * changes.
     */
    createLink(subscribeOwner, linkPropName) {
        throw new Error("Can not create a 'Link' from a 'Prop' property. ");
    }
    createProp(subscribeOwner, linkPropName) {
        throw new Error("Method not supported, create a SynchedPropertySimpleOneWaySubscribing from, where to create a Prop.");
    }
}
/*
  This exrension of SynchedPropertySimpleOneWay needs to be used for AppStorage
  because it needs to be notified about the source property changing
  ( there is no re-render process as in Views to update the wrappedValue )
*/
class SynchedPropertySimpleOneWaySubscribing extends SynchedPropertySimpleOneWay {
    constructor(linkedProperty, subscribeMe, info) {
        super(linkedProperty.get(), subscribeMe, info);
        this.linkedParentProperty_ = linkedProperty;
        this.linkedParentProperty_.subscribeMe(this);
    }
    aboutToBeDeleted() {
        // unregister from parent of this prop
        this.linkedParentProperty_.unlinkSuscriber(this.id__());
        super.aboutToBeDeleted();
    }
    hasChanged(newValue) {
        
        this.set(newValue);
    }
    /**
     * These functions are meant for use in connection with the App Stoage and
     * business logic implementation.
     * the created Link and Prop will update when 'this' property value
     * changes.
     */
    createLink(subscribeOwner, linkPropName) {
        throw new Error("Can not create a 'Link' from a 'Prop' property. ");
    }
    createProp(subscribeOwner, propPropName) {
        return new SynchedPropertySimpleOneWaySubscribing(this, subscribeOwner, propPropName);
    }
}
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
/**
 * SynchedPropertySimpleTwoWay
 *
 * all definitions in this file are framework internal
 */
class SynchedPropertySimpleTwoWay extends ObservedPropertySimpleAbstract {
    constructor(source, owningView, owningViewPropNme) {
        super(owningView, owningViewPropNme);
        this.changeNotificationIsOngoing_ = false;
        this.source_ = source;
        this.source_.subscribeMe(this);
    }
    /*
    like a destructor, need to call this before deleting
    the property.
  */
    aboutToBeDeleted() {
        if (this.source_) {
            this.source_.unlinkSuscriber(this.id__());
            this.source_ = undefined;
        }
        super.aboutToBeDeleted();
    }
    // this object is subscriber to  SynchedPropertySimpleTwoWay
    // will call this cb function when property has changed
    // a set (newValue) is not done because get reads through for the source_
    hasChanged(newValue) {
        if (!this.changeNotificationIsOngoing_) {
            
            this.notifyHasChanged(newValue);
        }
    }
    // get 'read through` from the ObservedProperty
    get() {
        
        if (!this.source_) {
            stateMgmtConsole.error(`SynchedPropertySimpleTwoWay[${this.id__()}IP, '${this.info() || "unknown"}'] source_ is undefined: get value is undefined.`);
            return undefined;
        }
        this.notifyPropertyRead();
        return this.source_.get();
    }
    // set 'writes through` to the ObservedProperty
    set(newValue) {
        if (!this.source_) {
            stateMgmtConsole.error(`SynchedPropertySimpleTwoWay[${this.id__()}IP, '${this.info() || "unknown"}'] source_ is undefined: set '${newValue}' ignoring.`);
            return;
        }
        if (this.source_.get() == newValue) {
            
            return;
        }
        
        // the source_ ObservedProeprty will call: this.hasChanged(newValue);
        // the purpose of the changeNotificationIsOngoing_ is to avoid 
        // circular notifications @Link -> source @State -> other but alos same @Link
        this.changeNotificationIsOngoing_ = true;
        this.source_.set(newValue);
        this.notifyHasChanged(newValue);
        this.changeNotificationIsOngoing_ = false;
    }
    /**
  * These functions are meant for use in connection with the App Stoage and
  * business logic implementation.
  * the created Link and Prop will update when 'this' property value
  * changes.
  */
    createLink(subscribeOwner, linkPropName) {
        return new SynchedPropertySimpleTwoWay(this, subscribeOwner, linkPropName);
    }
    createProp(subscribeOwner, propPropName) {
        return new SynchedPropertySimpleOneWaySubscribing(this, subscribeOwner, propPropName);
    }
}
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
/**
 * SynchedPropertyNesedObject
 *
 * all definitions in this file are framework internal
 */
class SynchedPropertyNesedObject extends ObservedPropertyObjectAbstract {
    /**
     * Construct a Property of a su component that links to a variable of parent view that holds an ObservedObject
     * example
     *   this.b.$a with b of type PC and a of type C, or
     *   this.$b[5] with this.b of type PC and array item b[5] of type C;
     *
     * @param subscribeMe
     * @param propName
     */
    constructor(obsObject, owningChildView, propertyName) {
        super(owningChildView, propertyName);
        this.obsObject_ = obsObject;
        // register to the ObservedObject
        ObservedObject.addOwningProperty(this.obsObject_, this);
    }
    /*
    like a destructor, need to call this before deleting
    the property.
    */
    aboutToBeDeleted() {
        // unregister from the ObservedObject
        ObservedObject.removeOwningProperty(this.obsObject_, this);
        super.aboutToBeDeleted();
    }
    // this object is subscriber to ObservedObject
    // will call this cb function when property has changed
    hasChanged(newValue) {
        
        this.notifyHasChanged(this.obsObject_);
    }
    // get 'read through` from the ObservedProperty
    get() {
        
        this.notifyPropertyRead();
        return this.obsObject_;
    }
    // set 'writes through` to the ObservedProperty
    set(newValue) {
        if (this.obsObject_ == newValue) {
            
            return;
        }
        
        // unsubscribe from the old value ObservedObject
        ObservedObject.removeOwningProperty(this.obsObject_, this);
        this.obsObject_ = newValue;
        // subscribe to the new value ObservedObject
        ObservedObject.addOwningProperty(this.obsObject_, this);
        // notify value change to subscribing View
        this.notifyHasChanged(this.obsObject_);
    }
    /**
   * These functions are meant for use in connection with the App Stoage and
   * business logic implementation.
   * the created Link and Prop will update when 'this' property value
   * changes.
   */
    createLink(subscribeOwner, linkPropName) {
        throw new Error("Method not supported for property linking to a nested objects.");
    }
    createProp(subscribeOwner, linkPropName) {
        throw new Error("Creating a 'Prop' proerty is unsuppoeted for Object type prperty value.");
    }
}
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
// Nativeview
// implemented in C++  for release
// and in utest/view_native_mock.ts for testing
class View extends NativeViewFullUpdate {
    /**
     * Create a View
     *
     * 1. option: top level View, specify
     *    - compilerAssignedUniqueChildId must specify
     *    - parent=undefined
     *    - localStorage  must provide if @LocalSTorageLink/Prop variables are used
     *      in this View or descendant Views.
     *
     * 2. option: not a top level View
     *    - compilerAssignedUniqueChildId must specify
     *    - parent must specify
     *    - localStorage do not specify, will inherit from parent View.
     *
     * @param compilerAssignedUniqueChildId Tw
     * @param parent
     * @param localStorage
     */
    constructor(compilerAssignedUniqueChildId, parent, localStorage) {
        super(compilerAssignedUniqueChildId, parent);
        this.propsUsedForRender = new Set();
        this.isRenderingInProgress = false;
        this.watchedProps = new Map();
        // my LocalStorge instance, shared with ancestor Views.
        // create a default instance on demand if none is initialized
        this.localStoragebackStore_ = undefined;
        this.id_ = SubscriberManager.MakeId();
        this.providedVars_ = parent ? new Map(parent.providedVars_)
            : new Map();
        this.localStoragebackStore_ = undefined;
        if (parent) {
            // this View is not a top-level View
            
            this.setCardId(parent.getCardId());
            this.localStorage_ = parent.localStorage_;
        }
        else if (localStorage) {
            this.localStorage_ = localStorage;
            
        }
        SubscriberManager.Add(this);
        
    }
    get localStorage_() {
        if (!this.localStoragebackStore_) {
            
            this.localStoragebackStore_ = new LocalStorage({ /* emty */});
        }
        return this.localStoragebackStore_;
    }
    set localStorage_(instance) {
        if (!instance) {
            // setting to undefined not allowed
            return;
        }
        if (this.localStoragebackStore_) {
            stateMgmtConsole.error(`${this.constructor.name} is setting LocalStorage instance twice`);
        }
        this.localStoragebackStore_ = instance;
    }
    // globally unique id, this is different from compilerAssignedUniqueChildId!
    id__() {
        return this.id_;
    }
    // temporary function, do not use, it will be removed soon!
    // prupsoe is to allow eDSL transpiler to fix a bug that 
    // relies on this method
    id() {
        return this.id__();
    }
    propertyHasChanged(info) {
        if (info) {
            // need to sync container instanceId to switch instanceId in C++ side.
            this.syncInstanceId();
            if (this.propsUsedForRender.has(info)) {
                
                this.markNeedUpdate();
            }
            else {
                
            }
            let cb = this.watchedProps.get(info);
            if (cb) {
                
                cb.call(this, info);
            }
            this.restoreInstanceId();
        } // if info avail.
    }
    propertyRead(info) {
        
        if (info && (info != "unknown") && this.isRenderingInProgress) {
            this.propsUsedForRender.add(info);
        }
    }
    // for test purposes
    propertiesNeededToRender() {
        return this.propsUsedForRender;
    }
    aboutToRender() {
        
        // reset
        this.propsUsedForRender = new Set();
        this.isRenderingInProgress = true;
    }
    aboutToContinueRender() {
        // do not reset
        this.isRenderingInProgress = true;
    }
    onRenderDone() {
        this.isRenderingInProgress = false;
        
    }
    /**
     * Function to be called from the constructor of the sub component
     * to register a @Watch varibale
     * @param propStr name of the variable. Note from @Provide and @Consume this is
     *      the variable name and not the alias!
     * @param callback application defined member function of sub-class
     */
    declareWatch(propStr, callback) {
        this.watchedProps.set(propStr, callback);
    }
    /**
     * This View @Provide's a variable under given name
     * Call this function from the constructor of the sub class
     * @param providedPropName either the variable name or the alias defined as
     *        decorator param
     * @param store the backing store object for this variable (not the get/set variable!)
     */
    addProvidedVar(providedPropName, store) {
        if (this.providedVars_.has(providedPropName)) {
            throw new ReferenceError(`${this.constructor.name}: duplicate @Provide property with name ${providedPropName}.
      Property with this name is provided by one of the ancestor Views already.`);
        }
        this.providedVars_.set(providedPropName, store);
    }
    /**
     * Method for the sub-class to call from its constructor for resolving
     *       a @Consume variable and initializing its backing store
     *       with the yncedPropertyTwoWay<T> object created from the
     *       @Provide variable's backing store.
     * @param providedPropName the name of the @Provide'd variable.
     *     This is either the @Consume decortor parameter, or variable name.
     * @param consumeVarName the @Consume variable name (not the
     *            @Consume decortor parameter)
     * @returns initiaizing value of the @Consume backing store
     */
    initializeConsume(providedPropName, consumeVarName) {
        let providedVarStore = this.providedVars_.get(providedPropName);
        if (providedVarStore === undefined) {
            throw new ReferenceError(`${this.constructor.name}: missing @Provide property with name ${providedPropName}.
     Fail to resolve @Consume(${providedPropName}).`);
        }
        return providedVarStore.createLink(this, consumeVarName);
    }
}
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
 * ObservedPropertyAbstractPU aka ObservedPropertyAbstract for partial update
 *
 * all definitions in this file are framework internal
 */
class ObservedPropertyAbstractPU extends ObservedPropertyAbstract {
    constructor(subscribingView, viewName) {
        super(subscribingView, viewName);
        this.dependentElementIds_ = new Set();
    }
    notifyPropertyRead() {
        stateMgmtConsole.error(`ObservedPropertyAbstract[${this.id__()}, '${this.info() || "unknown"}']: \
        notifyPropertyRead, DO NOT USE with PU. Use notifyPropertyHasBeenReadPU`);
    }
    notifyPropertyHasBeenReadPU() {
        
        this.subscribers_.forEach((subscribedId) => {
            var subscriber = SubscriberManager.Find(subscribedId);
            if (subscriber) {
                if ('propertyHasBeenReadPU' in subscriber) {
                    subscriber.propertyHasBeenReadPU(this);
                }
            }
        });
        this.recordDependentUpdate();
    }
    notifyPropertyHasChangedPU() {
        
        this.subscribers_.forEach((subscribedId) => {
            var subscriber = SubscriberManager.Find(subscribedId);
            if (subscriber) {
                if ('viewPropertyHasChanged' in subscriber) {
                    subscriber.viewPropertyHasChanged(this.info_, this.dependentElementIds_);
                }
                else if ('syncPeerHasChanged' in subscriber) {
                    subscriber.syncPeerHasChanged(this);
                }
                else {
                    stateMgmtConsole.warn(`ObservedPropertyAbstract[${this.id__()}, '${this.info() || "unknown"}']: notifyPropertryHasChangedPU: unknown subscriber ID '${subscribedId}' error!`);
                }
            }
        });
    }
    markDependentElementsDirty(view) {
        // TODO ace-ets2bundle, framework, compilated apps need to update together
        // this function will be removed after a short transiition periode
        stateMgmtConsole.warn(`markDependentElementsDirty no longer supported. App will work ok, but
        please update your ace-ets2bundle and recompile your application!`);
    }
    /**
     * factory function for concrete 'object' or 'simple' ObservedProperty object
     * depending if value is Class object
     * or simple type (boolean | number | string)
     * @param value
     * @param owningView
     * @param thisPropertyName
     * @returns either
     */
    static CreateObservedObject(value, owningView, thisPropertyName) {
        return (typeof value === "object") ?
            new ObservedPropertyObject(value, owningView, thisPropertyName)
            : new ObservedPropertySimple(value, owningView, thisPropertyName);
    }
    /**
     * during 'get' access recording take note of the created component and its elmtId
     * and add this component to the list of components who are dependent on this property
     */
    recordDependentUpdate() {
        const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
        if (elmtId < 0) {
            // not access recording 
            return;
        }
        
        this.dependentElementIds_.add(elmtId);
    }
    purgeDependencyOnElmtId(rmElmtId) {
        
        this.dependentElementIds_.delete(rmElmtId);
    }
    SetPropertyUnchanged() {
        // function to be removed
        // keep it here until transpiler is updated.
    }
    // FIXME check, is this used from AppStorage.
    // unified Appstorage, what classes to use, and the API
    createLink(subscribeOwner, linkPropName) {
        throw new Error("Can not create a AppStorage 'Link' from a @State property. ");
    }
    createProp(subscribeOwner, linkPropName) {
        throw new Error("Can not create a AppStorage 'Prop' from a @State property. ");
    }
    /*
      Below empty functions required to keep as long as this class derives from FU version
      ObservedPropertyAbstract. Need to overwrite these functions to do nothing for PU
      */
    notifyHasChanged(_) {
        stateMgmtConsole.error(`ObservedPropertyAbstract[${this.id__()}, '${this.info() || "unknown"}']: \
          notifyHasChanged, DO NOT USE with PU. Use syncPeerHasChanged() or objectPropertyHasChangedPU()`);
    }
    hasChanged(_) {
        // unused for PU
        // need to overwrite impl of base class with empty function.
    }
    propertyHasChanged(_) {
        // unused for PU
        // need to overwrite impl of base class with empty function.
    }
    propertyRead(_) {
        // unused for PU
        // need to overwrite impl of base class with empty function.
    }
}
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 * ObservedPropertyObjectAbstractPU
 *
 * common bbase class of ObservedPropertyObjectPU and
 * SyncedObjectPropertyTwoWayPU
 * adds the createObjectLink to the ObservedPropertyAbstract base
 *
 * all definitions in this file are framework internal
 */
class ObservedPropertyObjectAbstractPU extends ObservedPropertyAbstractPU {
    constructor(owningView, thisPropertyName) {
        super(owningView, thisPropertyName);
    }
}
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 * ObservedPropertySimpleAbstractPU
 *
 * all definitions in this file are framework internal
 */
class ObservedPropertySimpleAbstractPU extends ObservedPropertyAbstractPU {
    constructor(owningView, propertyName) {
        super(owningView, propertyName);
    }
}
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
 * ObservedPropertyObjectPU
 * implementation of @State and @Provide decorated variables of type class object
 *
 * all definitions in this file are framework internal
 *
 * class that holds an actual property value of type T
 * uses its base class to manage subscribers to this
 * property.
*/
class ObservedPropertyObjectPU extends ObservedPropertyObjectAbstractPU {
    constructor(value, owningView, propertyName) {
        super(owningView, propertyName);
        this.setValueInternal(value);
    }
    aboutToBeDeleted(unsubscribeMe) {
        this.unsubscribeFromOwningProperty();
        if (unsubscribeMe) {
            this.unlinkSuscriber(unsubscribeMe.id__());
        }
        super.aboutToBeDeleted();
    }
    /**
     * Called by a SynchedPropertyObjectTwoWayPU (@Link, @Consume) that uses this as sync peer when it has changed
     * @param eventSource
     */
    syncPeerHasChanged(eventSource) {
        
        this.notifyPropertyHasChangedPU();
    }
    /**
     * Wraped ObservedObjectPU has changed
     * @param souceObject
     * @param changedPropertyName
     */
    objectPropertyHasChangedPU(souceObject, changedPropertyName) {
        
        this.notifyPropertyHasChangedPU();
    }
    objectPropertyHasBeenReadPU(souceObject, changedPropertyName) {
        
        this.notifyPropertyHasBeenReadPU();
    }
    unsubscribeFromOwningProperty() {
        if (this.wrappedValue_) {
            if (this.wrappedValue_ instanceof SubscribaleAbstract) {
                this.wrappedValue_.removeOwningProperty(this);
            }
            else {
                ObservedObject.removeOwningProperty(this.wrappedValue_, this);
            }
        }
    }
    /*
      actually update this.wrappedValue_
      called needs to do value change check
      and also notify with this.aboutToChange();
    */
    setValueInternal(newValue) {
        if (typeof newValue !== 'object') {
            
            return false;
        }
        this.unsubscribeFromOwningProperty();
        if (ObservedObject.IsObservedObject(newValue)) {
            
            ObservedObject.addOwningProperty(newValue, this);
            this.wrappedValue_ = newValue;
        }
        else if (newValue instanceof SubscribaleAbstract) {
            
            this.wrappedValue_ = newValue;
            this.wrappedValue_.addOwningProperty(this);
        }
        else {
            
            this.wrappedValue_ = ObservedObject.createNew(newValue, this);
        }
        return true;
    }
    get() {
        
        this.notifyPropertyHasBeenReadPU();
        return this.wrappedValue_;
    }
    getUnmonitored() {
        
        // unmonitored get access , no call to otifyPropertyRead !
        return this.wrappedValue_;
    }
    set(newValue) {
        if (this.wrappedValue_ == newValue) {
            
            return;
        }
        
        this.setValueInternal(newValue);
        this.notifyPropertyHasChangedPU();
    }
}
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
class ObservedPropertySimplePU extends ObservedPropertySimpleAbstractPU {
    constructor(value, owningView, propertyName) {
        super(owningView, propertyName);
        if (typeof value === "object") {
            throw new SyntaxError("ObservedPropertySimple value must not be an object");
        }
        this.setValueInternal(value);
    }
    aboutToBeDeleted(unsubscribeMe) {
        if (unsubscribeMe) {
            this.unlinkSuscriber(unsubscribeMe.id__());
        }
        super.aboutToBeDeleted();
    }
    /**
   * Called by a @Link - SynchedPropertySimpleTwoWay that uses this as sync peer when it has changed
   * @param eventSource
   */
    syncPeerHasChanged(eventSource) {
        
        this.notifyPropertyHasChangedPU();
    }
    /*
      actually update this.wrappedValue_
      called needs to do value change check
      and also notify with this.aboutToChange();
    */
    setValueInternal(newValue) {
        
        this.wrappedValue_ = newValue;
    }
    getUnmonitored() {
        
        // unmonitored get access , no call to otifyPropertyRead !
        return this.wrappedValue_;
    }
    get() {
        
        this.notifyPropertyHasBeenReadPU();
        return this.wrappedValue_;
    }
    set(newValue) {
        if (this.wrappedValue_ == newValue) {
            
            return;
        }
        
        this.setValueInternal(newValue);
        this.notifyPropertyHasChangedPU();
    }
}
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
class SynchedPropertyObjectOneWayPU extends ObservedPropertyObjectAbstractPU {
    constructor(source, owningChildView, thisPropertyName) {
        super(owningChildView, thisPropertyName);
        if (source && (typeof (source) === "object") && ("subscribeMe" in source)) {
            // code path for @(Local)StorageProp, the souce is a ObservedPropertyObject in aLocalStorage)
            this.source_ = source;
            this.sourceIsOwnObject = false;
            // subscribe to receive value change updates from LocalStorage source property
            this.source_.subscribeMe(this);
        }
        else {
            // code path for @Prop
            if (!ObservedObject.IsObservedObject(source)) {
                stateMgmtConsole.warn(`@Prop ${this.info()}  Provided source object's class 
           lacks @Observed class decorator. Object property changes will not be observed.`);
            }
            
            this.source_ = new ObservedPropertyObjectPU(source, this, thisPropertyName);
            this.sourceIsOwnObject = true;
        }
        // deep copy source Object and wrap it
        this.setWrappedValue(this.source_.get());
        
    }
    /*
    like a destructor, need to call this before deleting
    the property.
    */
    aboutToBeDeleted() {
        if (this.source_) {
            this.source_.unlinkSuscriber(this.id__());
            if (this.sourceIsOwnObject == true && this.source_.numberOfSubscrbers() == 0) {
                
                this.source_.aboutToBeDeleted();
            }
            this.source_ = undefined;
        }
        super.aboutToBeDeleted();
    }
    syncPeerHasChanged(eventSource) {
        if (eventSource && this.source_ == eventSource) {
            // defensive programming: should always be the case!
            
            const newValue = this.source_.getUnmonitored();
            if (typeof newValue == "object") {
                
                this.setWrappedValue(newValue);
                this.notifyPropertyHasChangedPU();
            }
        }
        else {
            stateMgmtConsole.warn(`SynchedPropertyNesedObjectPU[${this.id__()}]: syncPeerHasChanged Unexpected situation. Ignorning event.`);
        }
    }
    /**
     * event emited by wrapped ObservedObject, when one of its property values changes
     * @param souceObject
     * @param changedPropertyName
     */
    objectPropertyHasChangedPU(souceObject, changedPropertyName) {
        
        this.notifyPropertyHasChangedPU();
    }
    objectPropertyHasBeenReadPU(souceObject, changedPropertyName) {
        
        this.notifyPropertyHasBeenReadPU();
    }
    getUnmonitored() {
        
        // unmonitored get access , no call to notifyPropertyRead !
        return this.wrappedValue_;
    }
    // get 'read through` from the ObservedObject
    get() {
        
        this.notifyPropertyHasBeenReadPU();
        return this.wrappedValue_;
    }
    // assignment to local variable in the form of this.aProp = <object value>
    // set 'writes through` to the ObservedObject
    set(newValue) {
        if (this.wrappedValue_ == newValue) {
            
            return;
        }
        
        if (!ObservedObject.IsObservedObject(newValue)) {
            stateMgmtConsole.warn(`@Prop ${this.info()} Set: Provided new object's class 
         lacks @Observed class decorator. Object property changes will not be observed.`);
        }
        this.setWrappedValue(newValue);
        this.notifyPropertyHasChangedPU();
    }
    reset(sourceChangedValue) {
        
        // if set causes an actual change, then, ObservedPropertyObject source_ will call syncPeerHasChanged
        this.source_.set(sourceChangedValue);
    }
    setWrappedValue(value) {
        let rawValue = ObservedObject.GetRawObject(value);
        let copy;
        // FIXME: Proper object deep copy missing here!
        if (rawValue instanceof Array) {
            copy = ObservedObject.createNew([...rawValue], this);
        }
        else {
            copy = ObservedObject.createNew(Object.assign({}, rawValue), this);
        }
        Object.setPrototypeOf(copy, Object.getPrototypeOf(rawValue));
        ObservedObject.addOwningProperty(this.wrappedValue_, this);
        this.wrappedValue_ = copy;
    }
}
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
 * SynchedPropertyObjectTwoWayPU
 * implementation of @Link and @Consume decorated variables of type class object
 *
 * all definitions in this file are framework internal
 */
class SynchedPropertyObjectTwoWayPU extends ObservedPropertyObjectAbstractPU {
    constructor(linkSource, owningChildView, thisPropertyName) {
        super(owningChildView, thisPropertyName);
        this.changeNotificationIsOngoing_ = false;
        this.linkedParentProperty_ = linkSource;
        if (this.linkedParentProperty_) {
            // register to the parent property
            this.linkedParentProperty_.subscribeMe(this);
        }
        // register to the ObservedObject
        ObservedObject.addOwningProperty(this.linkedParentProperty_.get(), this);
    }
    /*
    like a destructor, need to call this before deleting
    the property.
    */
    aboutToBeDeleted() {
        // unregister from parent of this link
        if (this.linkedParentProperty_) {
            this.linkedParentProperty_.unlinkSuscriber(this.id__());
            // unregister from the ObservedObject
            ObservedObject.removeOwningProperty(this.linkedParentProperty_.getUnmonitored(), this);
        }
        super.aboutToBeDeleted();
    }
    setObject(newValue) {
        if (!this.linkedParentProperty_) {
            stateMgmtConsole.warn(`SynchedPropertyObjectTwoWayPU[${this.id__()}, '${this.info() || "unknown"}']: setObject, no linked parent property.`);
            return;
        }
        this.linkedParentProperty_.set(newValue);
    }
    /**
     * Called when sync peer ObservedPropertyObject or SynchedPropertyObjectTwoWay has chnaged value
     * that peer can be in either parent or child component if 'this' is used for a @Link
     * that peer can be in either acestor or descendant component if 'this' is used for a @Consume
     * @param eventSource
     */
    syncPeerHasChanged(eventSource) {
        if (!this.changeNotificationIsOngoing_) {
            
            this.notifyPropertyHasChangedPU();
        }
    }
    /**
     * called when wrapped ObservedObject has changed poperty
     * @param souceObject
     * @param changedPropertyName
     */
    objectPropertyHasChangedPU(souceObject, changedPropertyName) {
        
        this.notifyPropertyHasChangedPU();
    }
    objectPropertyHasBeenReadPU(souceObject, changedPropertyName) {
        
        this.notifyPropertyHasBeenReadPU();
    }
    getUnmonitored() {
        
        // unmonitored get access , no call to otifyPropertyRead !
        return (this.linkedParentProperty_ ? this.linkedParentProperty_.getUnmonitored() : undefined);
    }
    // get 'read through` from the ObservedProperty
    get() {
        
        this.notifyPropertyHasBeenReadPU();
        return this.getUnmonitored();
    }
    // set 'writes through` to the ObservedProperty
    set(newValue) {
        if (this.getUnmonitored() == newValue) {
            
            return;
        }
        
        ObservedObject.removeOwningProperty(this.getUnmonitored(), this);
        // avoid circular notifications @Link -> source @State -> other but also back to same @Link
        this.changeNotificationIsOngoing_ = true;
        this.setObject(newValue);
        ObservedObject.addOwningProperty(this.getUnmonitored(), this);
        this.notifyPropertyHasChangedPU();
        this.changeNotificationIsOngoing_ = false;
    }
}
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
 * SynchedPropertySimpleOneWayPU
 * implementation of @Prop decorated variable of types boolean | number | string | enum
 *
 * all definitions in this file are framework internal
 */
class SynchedPropertySimpleOneWayPU extends ObservedPropertySimpleAbstractPU {
    constructor(source, subscribeMe, thisPropertyName) {
        super(subscribeMe, thisPropertyName);
        if (source && (typeof (source) === "object") && ("notifyHasChanged" in source) && ("subscribeMe" in source)) {
            // code path for @(Local)StorageProp
            this.source_ = source;
            this.sourceIsOwnObject = false;
            // subscribe to receive value chnage updates from LocalStorge source property
            this.source_.subscribeMe(this);
        }
        else {
            // code path for @Prop
            this.source_ = new ObservedPropertySimplePU(source, this, thisPropertyName);
            this.sourceIsOwnObject = true;
        }
        // use own backing store for value to avoid
        // value changes to be propagated back to source
        this.wrappedValue_ = this.source_.getUnmonitored();
    }
    /*
      like a destructor, need to call this before deleting
      the property.
    */
    aboutToBeDeleted() {
        if (this.source_) {
            this.source_.unlinkSuscriber(this.id__());
            if (this.sourceIsOwnObject == true && this.source_.numberOfSubscrbers() == 0) {
                
                this.source_.aboutToBeDeleted();
            }
            this.source_ = undefined;
            this.sourceIsOwnObject = false;
        }
        super.aboutToBeDeleted();
    }
    syncPeerHasChanged(eventSource) {
        if (eventSource && (eventSource == this.source_)) {
            // defensive, should always be the case
            
            this.sourceHasChanged(eventSource);
        }
    }
    sourceHasChanged(eventSource) {
        
        this.wrappedValue_ = eventSource.getUnmonitored();
        this.notifyPropertyHasChangedPU();
    }
    getUnmonitored() {
        
        // unmonitored get access , no call to otifyPropertyRead !
        return this.wrappedValue_;
    }
    // get 'read through` from the ObservedProperty
    get() {
        
        this.notifyPropertyHasBeenReadPU();
        return this.wrappedValue_;
    }
    set(newValue) {
        if (this.wrappedValue_ == newValue) {
            
            return;
        }
        
        this.wrappedValue_ = newValue;
        this.notifyPropertyHasChangedPU();
    }
    reset(sourceChangedValue) {
        
        // if set causes an actual change, then, ObservedPropertySimple source_ will call hasChanged
        this.source_.set(sourceChangedValue);
    }
}
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
 * SynchedPropertySimpleTwoWayPU
 * implementation of @Link and @Consume decorated variables of types boolean | number | string | enum
 *
 * all definitions in this file are framework internal
 */
class SynchedPropertySimpleTwoWayPU extends ObservedPropertySimpleAbstractPU {
    constructor(source, owningView, owningViewPropNme) {
        super(owningView, owningViewPropNme);
        this.changeNotificationIsOngoing_ = false;
        this.source_ = source;
        this.source_.subscribeMe(this);
    }
    /*
    like a destructor, need to call this before deleting
    the property.
  */
    aboutToBeDeleted() {
        if (this.source_) {
            this.source_.unlinkSuscriber(this.id__());
            this.source_ = undefined;
        }
        super.aboutToBeDeleted();
    }
    /**
     * Called when sync peer ObservedPropertySimple or SynchedPropertySimpletTwoWay has chnaged value
     * that peer can be in either parent or child component if 'this' is used for a @Link
     * that peer can be in either acestor or descendant component if 'this' is used for a @Consume
     * @param eventSource
     */
    syncPeerHasChanged(eventSource) {
        if (!this.changeNotificationIsOngoing_) {
            
            this.notifyPropertyHasChangedPU();
        }
    }
    getUnmonitored() {
        
        return (this.source_ ? this.source_.getUnmonitored() : undefined);
    }
    // get 'read through` from the ObservedProperty
    get() {
        
        this.notifyPropertyHasBeenReadPU();
        return this.getUnmonitored();
    }
    // set 'writes through` to the ObservedProperty
    set(newValue) {
        if (!this.source_) {
            
            return;
        }
        if (this.source_.get() == newValue) {
            
            return;
        }
        
        // avoid circular notifications @Link -> source @State -> other but also to same @Link
        this.changeNotificationIsOngoing_ = true;
        // the source_ ObservedProeprty will call: this.hasChanged(newValue);
        this.source_.set(newValue);
        this.notifyPropertyHasChangedPU();
        this.changeNotificationIsOngoing_ = false;
    }
}
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
 * SynchedPropertyNesedObjectPU
 * implementation of @ObjectLink decorated variables
 *
 * all definitions in this file are framework internal
 *
 */
class SynchedPropertyNesedObjectPU extends ObservedPropertyObjectAbstractPU {
    /**
     * Construct a Property of a su component that links to a variable of parent view that holds an ObservedObject
     * example
     *   this.b.$a with b of type PC and a of type C, or
     *   this.$b[5] with this.b of type PC and array item b[5] of type C;
     *
     * @param subscribeMe
     * @param propName
     */
    constructor(obsObject, owningChildView, propertyName) {
        super(owningChildView, propertyName);
        this.obsObject_ = obsObject;
        // register to the ObservedObject
        ObservedObject.addOwningProperty(this.obsObject_, this);
    }
    /*
    like a destructor, need to call this before deleting
    the property.
    */
    aboutToBeDeleted() {
        // unregister from the ObservedObject
        ObservedObject.removeOwningProperty(this.obsObject_, this);
        super.aboutToBeDeleted();
    }
    objectPropertyHasChangedPU(eventSource, changedPropertyName) {
        
        this.notifyPropertyHasChangedPU();
    }
    objectPropertyHasBeenReadPU(souceObject, changedPropertyName) {
        
        this.notifyPropertyHasBeenReadPU();
    }
    getUnmonitored() {
        // 
        // unmonitored get access , no call to otifyPropertyRead !
        return this.obsObject_;
    }
    // get 'read through` from the ObservedProperty
    get() {
        
        // this.notifyPropertyRead();
        this.notifyPropertyHasBeenReadPU();
        return this.obsObject_;
    }
    // set 'writes through` to the ObservedProperty
    set(newValue) {
        if (this.obsObject_ == newValue) {
            
            return;
        }
        
        // unsubscribe from the old value ObservedObject
        ObservedObject.removeOwningProperty(this.obsObject_, this);
        this.obsObject_ = newValue;
        // subscribe to the new value ObservedObject
        ObservedObject.addOwningProperty(this.obsObject_, this);
        // notify value change to subscribing View
        this.notifyPropertyHasChangedPU();
    }
}
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
 *
 *  * ViewPU - View for Partial Update
 *
* all definitions in this file are framework internal
*/
// denotes a missing elemntId, this is the case during initial render
const UndefinedElmtId = -1;
// Nativeview
// implemented in C++  for release
// and in utest/view_native_mock.ts for testing
class ViewPU extends NativeViewPartialUpdate {
    /**
     * Create a View
     *
     * 1. option: top level View, specify
     *    - compilerAssignedUniqueChildId must specify
     *    - parent=undefined
     *    - localStorage  must provide if @LocalSTorageLink/Prop variables are used
     *      in this View or descendant Views.
     *
     * 2. option: not a top level View
     *    - compilerAssignedUniqueChildId must specify
     *    - parent must specify
     *    - localStorage do not specify, will inherit from parent View.
     *
    */
    constructor(parent, localStorage, elmtId = -1) {
        super();
        this.parent_ = undefined;
        this.childrenWeakrefMap_ = new Map();
        this.watchedProps = new Map();
        // Set of dependent elmtIds that need partial update
        // during next re-render
        this.dirtDescendantElementIds_ = new Set();
        // registry of update functions
        // the key is the elementId of the Component/Element that's the result of this function
        this.updateFuncByElmtId = new Map();
        // my LocalStorge instance, shared with ancestor Views.
        // create a default instance on demand if none is initialized
        this.localStoragebackStore_ = undefined;
        // if set use the elmtId also as the ViewPU object's subscribable id.
        // these matching is requiremrnt for updateChildViewById(elmtId) being able to
        // find the child ViewPU object by given elmtId
        this.id_ = elmtId == -1 ? SubscriberManager.MakeId() : elmtId;
        this.providedVars_ = parent ? new Map(parent.providedVars_)
            : new Map();
        this.localStoragebackStore_ = undefined;
        if (parent) {
            // this View is not a top-level View
            
            this.setCardId(parent.getCardId());
            this.localStorage_ = parent.localStorage_;
            parent.addChild(this);
        }
        else if (localStorage) {
            this.localStorage_ = localStorage;
            
        }
        SubscriberManager.Add(this);
        
    }
    get localStorage_() {
        if (!this.localStoragebackStore_) {
            
            this.localStoragebackStore_ = new LocalStorage({ /* emty */});
        }
        return this.localStoragebackStore_;
    }
    set localStorage_(instance) {
        if (!instance) {
            // setting to undefined not allowed
            return;
        }
        if (this.localStoragebackStore_) {
            stateMgmtConsole.error(`${this.constructor.name} is setting LocalStorage instance twice`);
        }
        this.localStoragebackStore_ = instance;
    }
    // globally unique id, this is different from compilerAssignedUniqueChildId!
    id__() {
        return this.id_;
    }
    // super class will call this function from
    // its aboutToBeDeleted implementation
    aboutToBeDeletedInternal() {
        // When a custom component is deleted, need to notify the C++ side to clean the corresponding deletion cache Map,
        // because after the deletion, can no longer clean the RemoveIds cache on the C++ side through the
        // updateDirtyElements function.
        let removedElmtIds = [];
        this.updateFuncByElmtId.forEach((value, key) => {
            this.purgeVariableDependenciesOnElmtId(key);
            removedElmtIds.push(key);
        });
        this.deletedElmtIdsHaveBeenPurged(removedElmtIds);
        this.updateFuncByElmtId.clear();
        this.watchedProps.clear();
        this.providedVars_.clear();
        if (this.parent_) {
            this.parent_.removeChild(this);
        }
    }
    setParent(parent) {
        if (this.parent_ && parent) {
            stateMgmtConsole.warn(`ViewPU('${this.constructor.name}', ${this.id__()}).setChild: changing parent to '${parent.constructor.name}', id ${parent.id__()} (unsafe operation)`);
        }
        this.parent_ = parent;
    }
    /**
     * add given child and set 'this' as its parent
     * @param child child to add
     * @returns returns false if child with given child's id already exists
     *
     * framework internal function
     * Note: Use of WeakRef ensures child and parent do not generate a cycle dependency.
     * The add. Set<ids> is required to reliably tell what children still exist.
     */
    addChild(child) {
        if (this.childrenWeakrefMap_.has(child.id__())) {
            stateMgmtConsole.warn(`ViewPU('${this.constructor.name}', ${this.id__()}).addChild '${child.constructor.name}' id already exists ${child.id__()} !`);
            return false;
        }
        this.childrenWeakrefMap_.set(child.id__(), new WeakRef(child));
        child.setParent(this);
        return true;
    }
    /**
     * remove given child and remove 'this' as its parent
     * @param child child to add
     * @returns returns false if child with given child's id does not exist
     */
    removeChild(child) {
        const hasBeenDeleted = this.childrenWeakrefMap_.delete(child.id__());
        if (!hasBeenDeleted) {
            stateMgmtConsole.warn(`ViewPU('${this.constructor.name}', ${this.id__()}).removeChild '${child.constructor.name}', child id ${child.id__()} not known!`);
        }
        else {
            child.setParent(undefined);
        }
        return hasBeenDeleted;
    }
    /**
     * Retrieve child by given id
     * @param id
     * @returns child if in map and weak ref can still be downreferenced
     */
    getChildById(id) {
        const childWeakRef = this.childrenWeakrefMap_.get(id);
        return childWeakRef ? childWeakRef.deref() : undefined;
    }
    updateStateVars(params) {
        stateMgmtConsole.warn("ViewPU.updateStateVars unimplemented. Pls upgrade to latest eDSL transpiler version.");
    }
    initialRenderView() {
        this.initialRender();
    }
    UpdateElement(elmtId) {
        // do not process an Element that has been marked to be deleted
        const updateFunc = this.updateFuncByElmtId.get(elmtId);
        if ((updateFunc == undefined) || (typeof updateFunc !== "function")) {
            stateMgmtConsole.error(`${this.constructor.name}[${this.id__()}]: update function of ElementId ${elmtId} not found, internal error!`);
        }
        else {
            
            updateFunc(elmtId, /* isFirstRender */ false);
            // continue in native JSView
            // Finish the Update in JSView::JsFinishUpdateFunc
            // this function appends no longer used elmtIds (as recrded by VSP) to the given allRmElmtIds array
            this.finishUpdateFunc(elmtId);
            
        }
    }
    /**
     * force a complete rerender / update by executing all update functions
     * exec a regular rerender first
     *
     * @param deep recurse all children as well
     *
     * framework internal functions, apps must not call
     */
    forceCompleteRerender(deep = false) {
        stateMgmtConsole.warn(`ViewPU('${this.constructor.name}', ${this.id__()}).forceCompleteRerender - start.`);
        // request list of all (gloabbly) deleted elmtIds;
        let deletedElmtIds = [];
        this.getDeletedElemtIds(deletedElmtIds);
        // see which elmtIds are managed by this View
        // and clean up all book keeping for them
        this.purgeDeletedElmtIds(deletedElmtIds);
        Array.from(this.updateFuncByElmtId.keys()).sort(ViewPU.compareNumber).forEach(elmtId => this.UpdateElement(elmtId));
        if (deep) {
            this.childrenWeakrefMap_.forEach((weakRefChild) => {
                const child = weakRefChild.deref();
                if (child) {
                    child.forceCompleteRerender(true);
                }
            });
        }
        stateMgmtConsole.warn(`ViewPU('${this.constructor.name}', ${this.id__()}).forceCompleteRerender - end`);
    }
    /**
     * force a complete rerender / update on specific node by executing update function.
     *
     * @param elmtId which node needs to update.
     *
     * framework internal functions, apps must not call
     */
    forceRerenderNode(elmtId) {
        // request list of all (gloabbly) deleted elmtIds;
        let deletedElmtIds = [];
        this.getDeletedElemtIds(deletedElmtIds);
        // see which elmtIds are managed by this View
        // and clean up all book keeping for them
        this.purgeDeletedElmtIds(deletedElmtIds);
        this.UpdateElement(elmtId);
        // remove elemtId from dirtDescendantElementIds.
        this.dirtDescendantElementIds_.delete(elmtId);
    }
    updateStateVarsOfChildByElmtId(elmtId, params) {
        
        if (elmtId < 0) {
            stateMgmtConsole.warn(`ViewPU('${this.constructor.name}', ${this.id__()}).updateChildViewById(${elmtId}) - invalid elmtId - internal error!`);
            return;
        }
        let child = this.getChildById(elmtId);
        if (!child) {
            stateMgmtConsole.warn(`ViewPU('${this.constructor.name}', ${this.id__()}).updateChildViewById(${elmtId}) - no child with this elmtId - internal error!`);
            return;
        }
        child.updateStateVars(params);
        
    }
    // implements IMultiPropertiesChangeSubscriber
    viewPropertyHasChanged(varName, dependentElmtIds) {
        stateMgmtTrace.scopedTrace(() => {
            
            this.syncInstanceId();
            if (dependentElmtIds.size && !this.isFirstRender()) {
                if (!this.dirtDescendantElementIds_.size) {
                    // mark Composedelement dirty when first elmtIds are added
                    // do not need to do this every time
                    this.markNeedUpdate();
                }
                
                const union = new Set([...this.dirtDescendantElementIds_, ...dependentElmtIds]);
                this.dirtDescendantElementIds_ = union;
                
            }
            let cb = this.watchedProps.get(varName);
            if (cb) {
                
                cb.call(this, varName);
            }
            this.restoreInstanceId();
        }, "ViewPU.viewPropertyHasChanged", this.constructor.name, varName, dependentElmtIds.size);
    }
    /**
     * Function to be called from the constructor of the sub component
     * to register a @Watch varibale
     * @param propStr name of the variable. Note from @Provide and @Consume this is
     *      the variable name and not the alias!
     * @param callback application defined member function of sub-class
     */
    declareWatch(propStr, callback) {
        this.watchedProps.set(propStr, callback);
    }
    /**
     * This View @Provide's a variable under given name
     * Call this function from the constructor of the sub class
     * @param providedPropName either the variable name or the alias defined as
     *        decorator param
     * @param store the backing store object for this variable (not the get/set variable!)
     */
    addProvidedVar(providedPropName, store) {
        if (this.providedVars_.has(providedPropName)) {
            throw new ReferenceError(`${this.constructor.name}: duplicate @Provide property with name ${providedPropName}.
      Property with this name is provided by one of the ancestor Views already.`);
        }
        this.providedVars_.set(providedPropName, store);
    }
    /**
     * Method for the sub-class to call from its constructor for resolving
     *       a @Consume variable and initializing its backing store
     *       with the yncedPropertyTwoWay<T> object created from the
     *       @Provide variable's backing store.
     * @param providedPropName the name of the @Provide'd variable.
     *     This is either the @Consume decortor parameter, or variable name.
     * @param consumeVarName the @Consume variable name (not the
     *            @Consume decortor parameter)
     * @returns initiaizing value of the @Consume backing store
     */
    initializeConsume(providedPropName, consumeVarName) {
        let providedVarStore = this.providedVars_.get(providedPropName);
        if (providedVarStore === undefined) {
            throw new ReferenceError(`${this.constructor.name}: missing @Provide property with name ${providedPropName}.
     Fail to resolve @Consume(${providedPropName}).`);
        }
        return providedVarStore.createSync((source) => (source instanceof ObservedPropertySimple)
            ? new SynchedPropertySimpleTwoWayPU(source, this, consumeVarName)
            : new SynchedPropertyObjectTwoWayPU(source, this, consumeVarName));
    }
    /**
     * given the elmtid of a child or child of child within this custom component
     * remember this component needs a partial update
     * @param elmtId
     */
    markElemenDirtyById(elmtId) {
        // TODO ace-ets2bundle, framework, compilated apps need to update together
        // this function will be removed after a short transiition periode
        stateMgmtConsole.error(`markElemenDirtyById no longer supported.
        Please update your ace-ets2bundle and recompile your application!`);
    }
    /**
     * For each recorded dirty Element in this custom component
     * run its update function
     *
     */
    updateDirtyElements() {
        do {
            
            // request list of all (gloabbly) deleteelmtIds;
            let deletedElmtIds = [];
            this.getDeletedElemtIds(deletedElmtIds);
            // see which elmtIds are managed by this View
            // and clean up all book keeping for them
            this.purgeDeletedElmtIds(deletedElmtIds);
            // process all elmtIds marked as needing update in ascending order.
            // ascending order ensures parent nodes will be updated before their children
            // prior cleanup ensure no already deleted Elements have their update func executed
            Array.from(this.dirtDescendantElementIds_).sort(ViewPU.compareNumber).forEach(elmtId => {
                this.UpdateElement(elmtId);
                this.dirtDescendantElementIds_.delete(elmtId);
            });
        } while (this.dirtDescendantElementIds_.size);
    }
    //  given a list elementIds removes these from state variables dependency list and from elmtId -> updateFunc map
    purgeDeletedElmtIds(rmElmtIds) {
        if (rmElmtIds.length == 0) {
            return;
        }
        
        // rmElmtIds is the array of ElemntIds that
        let removedElmtIds = [];
        rmElmtIds.forEach((elmtId) => {
            // remove entry from Map elmtId -> update function
            if (this.updateFuncByElmtId.delete(elmtId)) {
                // for each state var, remove dependent elmtId (if present)
                // purgeVariableDependenciesOnElmtId needs to be generated by the compiler
                this.purgeVariableDependenciesOnElmtId(elmtId);
                // keep track of elmtId that has been de-registered
                removedElmtIds.push(elmtId);
            }
        });
        this.deletedElmtIdsHaveBeenPurged(removedElmtIds);
        
        
    }
    // the current executed update function
    observeComponentCreation(compilerAssignedUpdateFunc) {
        const elmtId = ViewStackProcessor.AllocateNewElmetIdForNextComponent();
        
        compilerAssignedUpdateFunc(elmtId, /* is first rneder */ true);
        this.updateFuncByElmtId.set(elmtId, compilerAssignedUpdateFunc);
        
    }
    // performs the update on a branch within if() { branch } else if (..) { branch } else { branch }
    ifElseBranchUpdateFunction(branchId, branchfunc) {
        const oldBranchid = If.getBranchId();
        if (branchId == oldBranchid) {
            
            return;
        }
        If.branchId(branchId);
        branchfunc();
    }
    /**
     Partial updates for ForEach.
     * @param elmtId ID of element.
     * @param itemArray Array of items for use of itemGenFunc.
     * @param itemGenFunc Item generation function to generate new elements. If index parameter is
     *                    given set itemGenFuncUsesIndex to true.
     * @param idGenFunc   ID generation function to generate unique ID for each element. If index parameter is
     *                    given set idGenFuncUsesIndex to true.
     * @param itemGenFuncUsesIndex itemGenFunc optional index parameter is given or not.
     * @param idGenFuncUsesIndex idGenFunc optional index parameter is given or not.
     */
    forEachUpdateFunction(elmtId, itemArray, itemGenFunc, idGenFunc, itemGenFuncUsesIndex = false, idGenFuncUsesIndex = false) {
        
        if (itemArray === null || itemArray === undefined) {
            stateMgmtConsole.error(`ForEach input array is null or undefined error.`);
            return;
        }
        if (itemGenFunc === null || itemGenFunc === undefined) {
            stateMgmtConsole.error(`Error: Item generation function not defined in forEach function.`);
            return;
        }
        if (idGenFunc === undefined) {
            
            idGenFuncUsesIndex = true;
            // catch possible error caused by Stringify and re-throw an Error with a meaningful (!) error message
            idGenFunc = (item, index) => {
                try {
                    return `${index}__${JSON.stringify(item)}`;
                }
                catch (e) {
                    throw new Error(`${this.constructor.name}[${this.id__()}]: ForEach id ${elmtId}: use of default id generator function not possble on provided data structure. Need to specify id generator function (ForEach 3rd parameter).`);
                }
            };
        }
        let diffIndexArray = []; // New indexes compared to old one.
        let newIdArray = [];
        let idDuplicates = [];
        const arr = itemArray; // just to trigger a 'get' onto the array
        // ID gen is with index.
        if (idGenFuncUsesIndex) {
            
            // Create array of new ids.
            arr.forEach((item, indx) => {
                newIdArray.push(idGenFunc(item, indx));
            });
        }
        else {
            // Create array of new ids.
            
            arr.forEach((item, index) => {
                newIdArray.push(`${itemGenFuncUsesIndex ? index + '_' : ''}` + idGenFunc(item));
            });
        }
        // Set new array on C++ side.
        // C++ returns array of indexes of newly added array items.
        // these are indexes in new child list.
        ForEach.setIdArray(elmtId, newIdArray, diffIndexArray, idDuplicates);
        // Its error if there are duplicate IDs.
        if (idDuplicates.length > 0) {
            idDuplicates.forEach((indx) => {
                stateMgmtConsole.error(`Error: ${newIdArray[indx]} generated for ${indx}${indx < 4 ? indx == 2 ? "nd" : "rd" : "th"} array item ${arr[indx]}.`);
            });
            stateMgmtConsole.error(`Ids generated by the ForEach id gen function must be unique, error.`);
        }
        
        // Item gen is with index.
        
        // Create new elements if any.
        diffIndexArray.forEach((indx) => {
            ForEach.createNewChildStart(newIdArray[indx], this);
            if (itemGenFuncUsesIndex) {
                itemGenFunc(arr[indx], indx);
            }
            else {
                itemGenFunc(arr[indx]);
            }
            ForEach.createNewChildFinish(newIdArray[indx], this);
        });
    }
    /**
       * CreateStorageLink and CreateStorageLinkPU are used by the implementation of @StorageLink and
       * @LocalStotrageLink in full update and partial update solution respectively.
       * These are not part of the public AppStorage API , apps should not use.
       * @param storagePropName - key in LocalStorage
       * @param defaultValue - value to use when creating a new prop in the LocalStotage
       * @param owningView - the View/ViewPU owning the @StorageLink/@LocalStorageLink variable
       * @param viewVariableName -  @StorageLink/@LocalStorageLink variable name
       * @returns SynchedPropertySimple/ObjectTwoWay/PU
       */
    createStorageLink(storagePropName, defaultValue, viewVariableName) {
        return AppStorage.__CreateSync(storagePropName, defaultValue, (source) => (source === undefined)
            ? undefined
            : (source instanceof ObservedPropertySimple)
                ? new SynchedPropertySimpleTwoWayPU(source, this, viewVariableName)
                : new SynchedPropertyObjectTwoWayPU(source, this, viewVariableName));
    }
    createStorageProp(storagePropName, defaultValue, viewVariableName) {
        return AppStorage.__CreateSync(storagePropName, defaultValue, (source) => (source === undefined)
            ? undefined
            : (source instanceof ObservedPropertySimple)
                ? new SynchedPropertySimpleOneWayPU(source, this, viewVariableName)
                : new SynchedPropertyObjectOneWayPU(source, this, viewVariableName));
    }
    createLocalStorageLink(storagePropName, defaultValue, viewVariableName) {
        return this.localStorage_.__createSync(storagePropName, defaultValue, (source) => (source === undefined)
            ? undefined
            : (source instanceof ObservedPropertySimple)
                ? new SynchedPropertySimpleTwoWayPU(source, this, viewVariableName)
                : new SynchedPropertyObjectTwoWayPU(source, this, viewVariableName));
    }
    createLocalStorageProp(storagePropName, defaultValue, viewVariableName) {
        return this.localStorage_.__createSync(storagePropName, defaultValue, (source) => (source === undefined)
            ? undefined
            : (source instanceof ObservedPropertySimple)
                ? new SynchedPropertySimpleOneWayPU(source, this, viewVariableName)
                : new SynchedPropertyObjectOneWayPU(source, this, viewVariableName));
    }
}
// Array.sort() converts array items to string to compare them, sigh!
ViewPU.compareNumber = (a, b) => {
    return (a < b) ? -1 : (a > b) ? 1 : 0;
};
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
 *
 *  * ViewPU - View for Partial Update
 *
* all definitions in this file are framework internal
*/
/**
    given parameters for calling a @Builder function
    this function wraps the Object of type T inside a ES6 Proxy.
    Each param, i.e. Object property is either a function or a value.
    If it is a function the function can either return a value of expected
    parameter type or an ObservedPropertyabstract<T> where T is the exected
    parameter type. The latter is the case when passing a state variable by
    reference.

    Two purposes:
    1 - @Builder function boxy accesses params a '$$.paramA'
        However paramA can be a function, so to obtain the value the
        access would need to be '$$.param()' The proxy executes
        the function and return s the result
    2 - said function returns to ObservedPropertyAbstract backing store of
        a calling @Component state variable (whenever the state var is
        provided to the @Builder function). For this case the proxy can provide
        - the value by executing paramA() to return the ObservedPropertyAbstract
          and further (monitored!) get() to read its value
        - when requested to return '__param1' it returns the ObservedPropertyAbstract
          object. The scenario is to use to init a @Link source.
  */
function makeBuilderParameterProxy(builderName, source) {
    return new Proxy(source, {
        set(target, prop, val) {
            throw Error(`@Builder '${builderName}': Invalid attempt to set(write to) parameter '${prop.toString()}' error!`);
        },
        get(target, prop) {
            const prop1 = prop.toString().trim().startsWith("__")
                ? prop.toString().trim().substring(2)
                : prop.toString().trim();
            
            if (!(typeof target === "object") && (prop1 in target)) {
                throw Error(`@Builder '${builderName}': '${prop1}' used but not a function parameter error!`);
            }
            const value = target[prop1];
            if (typeof value !== "function") {
                
                return value;
            }
            const funcRet = value();
            if ((typeof funcRet === "object") && ('get' in funcRet)) {
                if (prop1 !== prop) {
                    
                    return funcRet;
                }
                else {
                    
                    const result = funcRet.get();
                    
                    return result;
                }
            }
            
            return funcRet;
        } // get
    }); // new Proxy
}
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

PersistentStorage.ConfigureBackend(new Storage());
Environment.ConfigureBackend(new EnvironmentSetting());

