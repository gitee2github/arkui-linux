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

  protected storage_: Map<string, ObservedPropertyAbstract<any>>;

  /**
   * Construct new instance of LocalStorage
   * initialzie with all properties and their values that Object.keys(params) returns
   * Property values must not be undefined.
   * @param initializingProperties Object containing keys and values. @see set() for valid values
   * 
   * @since 9
   */
  constructor(initializingProperties: Object = {}) {
    super();
    stateMgmtConsole.debug(`${this.constructor.name} constructor.`)
    this.storage_ = new Map<string, ObservedPropertyAbstract<any>>();
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
  public initializeProps(initializingProperties: Object = {}) {
    stateMgmtConsole.debug(`${this.constructor.name} initializing with Object keys: [${Object.keys(initializingProperties)}].`)
    this.storage_.clear();
    Object.keys(initializingProperties).filter((propName) => initializingProperties[propName] != undefined).forEach((propName) =>
      this.addNewPropertyInternal(propName, initializingProperties[propName])
    );
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
  public aboutToBeDeleted(): boolean {
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
  public has(propName: string): boolean {
    return this.storage_.has(propName);
  }


  /**
   * Provide names of all properties in LocalStorage
   * same as ES6 Map.prototype.keys()
   * @returns return a Map Iterator
   * 
   * @since 9
  */
  public keys(): IterableIterator<string> {
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
  public size(): number {
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
  public get<T>(propName: string): T | undefined {
    var p: ObservedPropertyAbstract<T> | undefined = this.storage_.get(propName);
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
  public set<T>(propName: string, newValue: T): boolean {
    if (newValue == undefined) {
      stateMgmtConsole.warn(`${this.constructor.name}: set('${propName}') with newValue == undefined not allowed.`);
      return false;
    }
    var p: ObservedPropertyAbstract<T> | undefined = this.storage_.get(propName);
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
  public setOrCreate<T>(propName: string, newValue: T): boolean {
    if (newValue == undefined) {
      stateMgmtConsole.warn(`${this.constructor.name}: setOrCreate('${propName}') with newValue == undefined not allowed.`);
      return false;
    }

    var p: ObservedPropertyAbstract<T> = this.storage_.get(propName);
    if (p) {
      stateMgmtConsole.debug(`${this.constructor.name}.setOrCreate(${propName}, ${newValue}) update existing property`);
      p.set(newValue);
    } else {
      stateMgmtConsole.debug(`${this.constructor.name}.setOrCreate(${propName}, ${newValue}) create new entry and set value`);
      this.addNewPropertyInternal<T>(propName, newValue);
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
  private addNewPropertyInternal<T>(propName: string, value: T): ObservedPropertyAbstract<T> {
    const newProp = (typeof value === "object") ?
      new ObservedPropertyObject<T>(value, undefined, propName)
      : new ObservedPropertySimple<T>(value, undefined, propName);
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
  public link<T>(propName: string, linkUser?: IPropertySubscriber, subscribersName?: string): SubscribedAbstractProperty<T> | undefined {
    var p: ObservedPropertyAbstract<T> | undefined = this.storage_.get(propName);
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
  public setAndLink<T>(propName: string, defaultValue: T, linkUser?: IPropertySubscriber, subscribersName?: string): SubscribedAbstractProperty<T> {
    var p: ObservedPropertyAbstract<T> | undefined = this.storage_.get(propName);
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
  public prop<S>(propName: string, propUser?: IPropertySubscriber, subscribersName?: string): SubscribedAbstractProperty<S> | undefined {
    var p: ObservedPropertyAbstract<S> | undefined = this.storage_.get(propName);
    if (p == undefined) {
      stateMgmtConsole.warn(`${this.constructor.name}: prop: no property ${propName} error.`);
      return undefined;
    }
    let propResult = p.createProp(propUser, propName)
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
  public setAndProp<S>(propName: string, defaultValue: S, propUser?: IPropertySubscriber, subscribersName?: string): SubscribedAbstractProperty<S> {
    var p: ObservedPropertyAbstract<S> | undefined = this.storage_.get(propName);
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
  public delete(propName: string): boolean {
    var p: ObservedPropertyAbstract<any> | undefined = this.storage_.get(propName);
    if (p) {
      if (p.numberOfSubscrbers()) {
        stateMgmtConsole.error(`${this.constructor.name}: Attempt to delete property ${propName} that has \
          ${p.numberOfSubscrbers()} subscribers. Subscribers need to unsubscribe before prop deletion.`);
        return false;
      }
      p.aboutToBeDeleted();
      this.storage_.delete(propName);
      return true;
    } else {
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
  protected clear(): boolean {
    for (let propName of this.keys()) {
      var p: ObservedPropertyAbstract<any> = this.storage_.get(propName);
      if (p.numberOfSubscrbers()) {
        stateMgmtConsole.error(`${this.constructor.name}.deleteAll: Attempt to delete property ${propName} that \
          has ${p.numberOfSubscrbers()} subscribers. Subscribers need to unsubscribe before prop deletion.
          Any @Component instance with a @StorageLink/Prop or @LocalStorageLink/Prop is a subscriber.`);
        return false;
      }
    }
    for (let propName of this.keys()) {
      var p: ObservedPropertyAbstract<any> = this.storage_.get(propName);
      p.aboutToBeDeleted();
    }
    this.storage_.clear();
    stateMgmtConsole.debug(`${this.constructor.name}.deleteAll: success`);
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
  public subscribeToChangesOf<T>(propName: string, subscriber: ISinglePropertyChangeSubscriber<T>): boolean {
    var p: ObservedPropertyAbstract<T> | undefined = this.storage_.get(propName);
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
  public unsubscribeFromChangesOf(propName: string, subscriberId: number): boolean {
    var p: ObservedPropertyAbstract<any> | undefined = this.storage_.get(propName);
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
  public numberOfSubscrbersTo(propName: string): number | undefined {
    var p: ObservedPropertyAbstract<any> | undefined = this.storage_.get(propName);
    if (p) {
      return p.numberOfSubscrbers();
    }
    return undefined;
  }

  public __createSync<T>(storagePropName: string, defaultValue: T,
    factoryFunc: SynchedPropertyFactoryFunc): ObservedPropertyAbstract<T> {

    let p: ObservedPropertyAbstract<T> = this.storage_.get(storagePropName);
    if (p == undefined) {
      // property named 'storagePropName' not yet in storage
      // add new property to storage
      if (defaultValue === undefined) {
        stateMgmtConsole.error(`${this.constructor.name}.__createSync(${storagePropName}, non-existing property and undefined default value. ERROR.`);
        return undefined;
      }

      p = this.addNewPropertyInternal<T>(storagePropName, defaultValue);
    }

    return factoryFunc(p);
  }
}