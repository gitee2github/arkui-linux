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

#ifndef ECMASCRIPT_JS_HCLASS_H
#define ECMASCRIPT_JS_HCLASS_H

#include "ecmascript/ecma_macros.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/mem/tagged_object.h"
#include "ecmascript/mem/barriers.h"
#include "ecmascript/mem/slots.h"
#include "ecmascript/mem/visitor.h"
#include "ecmascript/property_attributes.h"

#include "libpandabase/utils/bit_field.h"

/*
 *                         JS Object and JS HClass Layout
 *
 *      Properties                         JS Object                    JS HClass
 *      +------------+                     +------------+               +------------------+
 *      |arrayHClass + <---------|         |JS HClass   +-------------->|   meta hclass    |
 *      +------------+           |         +------------+               +------------------+
 *      | property 0 |           |         |Hash        |               |   hclass level   |
 *      +------------+           |         +------------+               +------------------+
 *      | property 1 |           |-------  |Properties  |               |   supers[]       |
 *      +------------+                     +------------+               +------------------+
 *      |...         |           |-------  |Elements    |               |   vtable[]       |
 *      +------------+           |         +------------+               +------------------+
 *                               |         |inl-prop-0  |               |   prototype      |
 *      Elements                 |         +------------+               +------------------+
 *      +------------+           |         |inl-prop-1  |               |   layout         |
 *      |arrayHClass + <---------|         +------------+               +------------------+
 *      +------------+                     |...         |               |   transitions    |
 *      | value 0    |                     +------------+               +------------------+
 *      +------------+                                                  |    paratent      |
 *      | value 1    |                                                  +------------------+
 *      +------------+                                                  |ProtoChangeMarker |
 *      |...         |                                                  +------------------+
 *      +------------+                                                  |    EnumCache     |
 *                                                                      +------------------+
 *
 *                          Proto: [[Prototype]] in Ecma spec
 *                          Layout: record key and attr
 *                          ProtoChangeMarker, ProtoChangeDetails: monitor [[prototype]] chain
 *                          EnumCache: use for for-in syntax
 *
 */
namespace panda::ecmascript {
class ProtoChangeDetails;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define JSTYPE_DECL       /* //////////////////////////////////////////////////////////////////////////////-PADDING */ \
    INVALID = 0,          /* //////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_OBJECT,        /* JS_OBJECT_FIRST ////////////////////////////////////////////////////////////////////// */ \
        JS_REALM,         /* //////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_FUNCTION_BASE, /* //////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_FUNCTION,      /* //////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_PROXY_REVOC_FUNCTION,       /* /////////////////////////////////////////////////////////////////-PADDING */ \
        JS_PROMISE_REACTIONS_FUNCTION, /* /////////////////////////////////////////////////////////////////-PADDING */ \
        JS_PROMISE_EXECUTOR_FUNCTION,  /* /////////////////////////////////////////////////////////////////-PADDING */ \
        JS_ASYNC_FROM_SYNC_ITER_UNWARP_FUNCTION,  /* //////////////////////////////////////////////////////-PADDING */ \
        JS_PROMISE_ALL_RESOLVE_ELEMENT_FUNCTION,  /* //////////////////////////////////////////////////////-PADDING */ \
        JS_ASYNC_GENERATOR_RESUME_NEXT_RETURN_PROCESSOR_RST_FTN, /* ///////////////////////////////////////-PADDING */ \
        JS_PROMISE_ANY_REJECT_ELEMENT_FUNCTION,  /* ///////////////////////////////////////////////////////-PADDING */ \
        JS_PROMISE_ALL_SETTLED_ELEMENT_FUNCTION,  /* //////////////////////////////////////////////////////-PADDING */ \
        JS_PROMISE_FINALLY_FUNCTION,  /* //////////////////////////////////////////////////////////////////-PADDING */ \
        JS_PROMISE_VALUE_THUNK_OR_THROWER_FUNCTION,  /* ///////////////////////////////////////////////////-PADDING */ \
        JS_GENERATOR_FUNCTION, /* /////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_ASYNC_GENERATOR_FUNCTION,  /* //////////////////////////////////////////////////////////////////-PADDING */ \
        JS_ASYNC_FUNCTION, /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_INTL_BOUND_FUNCTION, /* ////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_ASYNC_AWAIT_STATUS_FUNCTION, /* ////////////////////////////////////////////////////////////////-PADDING */ \
        JS_BOUND_FUNCTION, /*  //////////////////////////////////////////////////////////////////////////////////// */ \
                                                                                                                       \
        JS_ERROR,           /* JS_ERROR_FIRST /////////////////////////////////////////////////////////////-PADDING */ \
        JS_EVAL_ERROR,      /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_RANGE_ERROR,     /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_REFERENCE_ERROR, /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_TYPE_ERROR,      /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_AGGREGATE_ERROR, /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_URI_ERROR,       /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_SYNTAX_ERROR,    /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_OOM_ERROR,       /* JS_ERROR_LAST /////////////////////////////////////////////////////////////////////// */\
                                                                                                                       \
        JS_REG_EXP,  /* ///////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_SET,      /* ///////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_MAP,      /* ///////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_WEAK_MAP, /* ///////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_WEAK_SET, /* ///////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_WEAK_REF, /* ///////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_FINALIZATION_REGISTRY, /* //////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_DATE,     /* ///////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_ITERATOR, /* ///////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_ASYNCITERATOR, /* //////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_ASYNC_FROM_SYNC_ITERATOR, /* ///////////////////////////////////////////////////////////////////-PADDING */ \
        JS_FORIN_ITERATOR,       /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_MAP_ITERATOR,         /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_SET_ITERATOR,         /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_REG_EXP_ITERATOR,        /* ////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_ARRAYLIST_ITERATOR, /* /////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_DEQUE_ITERATOR,   /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_HASHMAP_ITERATOR, /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_HASHSET_ITERATOR, /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_LIGHT_WEIGHT_MAP_ITERATOR, /* //////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_LIGHT_WEIGHT_SET_ITERATOR,  /* /////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_PLAIN_ARRAY_ITERATOR,  /* //////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_QUEUE_ITERATOR,   /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_STACK_ITERATOR,   /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_TREEMAP_ITERATOR, /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_TREESET_ITERATOR, /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_VECTOR_ITERATOR,  /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_LINKED_LIST_ITERATOR, /* ///////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_LIST_ITERATOR,    /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_ARRAY_ITERATOR,       /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_STRING_ITERATOR,      /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_INTL, /* ///////////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_LOCALE, /* /////////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_DATE_TIME_FORMAT, /* ///////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_RELATIVE_TIME_FORMAT, /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_NUMBER_FORMAT, /* //////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_COLLATOR, /* ///////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_PLURAL_RULES, /* ///////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_DISPLAYNAMES, /* ///////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_LIST_FORMAT,  /* ///////////////////////////////////////////////////////////////////////////////-PADDING */ \
                                                                                                                       \
        JS_ARRAY_BUFFER, /* ///////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_SHARED_ARRAY_BUFFER, /* ////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_PROMISE,      /* ///////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_DATA_VIEW,    /* /////////////////////////////////////////////////////////////////////////////////////// */ \
        JS_ARGUMENTS, /* //////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_GENERATOR_OBJECT,  /* //////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_ASYNC_GENERATOR_OBJECT,  /* ////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_ASYNC_FUNC_OBJECT, /* //////////////////////////////////////////////////////////////////////////-PADDING */ \
                                                                                                                       \
        /* SPECIAL indexed objects begin, DON'T CHANGE HERE ///////////////////////////////////////////////-PADDING */ \
        JS_ARRAY,       /* ////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_ARRAY_LIST, /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_LIGHT_WEIGHT_MAP,      /* //////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_LIGHT_WEIGHT_SET, /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_VECTOR,     /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_LINKED_LIST, /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_LIST,       /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_HASH_MAP,   /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_HASH_SET,   /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_TREE_MAP,   /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_TREE_SET,   /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_DEQUE,      /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_STACK,      /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_PLAIN_ARRAY, /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_API_QUEUE,      /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_TYPED_ARRAY, /* JS_TYPED_ARRAY_FIRST /////////////////////////////////////////////////////////////////// */ \
        JS_INT8_ARRAY,  /* ////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_UINT8_ARRAY, /* ////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_UINT8_CLAMPED_ARRAY, /* ////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_INT16_ARRAY,         /* ////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_UINT16_ARRAY,        /* ////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_INT32_ARRAY,         /* ////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_UINT32_ARRAY,        /* ////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_FLOAT32_ARRAY,       /* ////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_FLOAT64_ARRAY,       /* ////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_BIGINT64_ARRAY,      /* ////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_BIGUINT64_ARRAY,     /* JS_TYPED_ARRAY_LAST ///////////////////////////////////////////////////////////// */\
        JS_PRIMITIVE_REF, /* number\boolean\string. SPECIAL indexed objects end, DON'T CHANGE HERE ////////-PADDING */ \
        JS_MODULE_NAMESPACE, /* ///////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_CJS_MODULE, /* /////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_CJS_EXPORTS, /* ////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_CJS_REQUIRE, /* ////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_GLOBAL_OBJECT, /* JS_OBJECT_LAST/////////////////////////////////////////////////////////////////-PADDING */\
        JS_PROXY, /* ECMA_OBJECT_LAST ////////////////////////////////////////////////////////////////////////////// */\
                                                                                                                       \
        HCLASS,       /* //////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        STRING,       /* //////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        BIGINT,       /* //////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        TAGGED_ARRAY, /* //////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        BYTE_ARRAY,   /* //////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        LEXICAL_ENV,  /* //////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        TAGGED_DICTIONARY, /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        CONSTANT_POOL, /* /////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        COW_TAGGED_ARRAY, /* //////////////////////////////////////////////////////////////////////////////-PADDING */ \
        LINKED_NODE,  /* //////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        RB_TREENODE,  /* //////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        FREE_OBJECT_WITH_ONE_FIELD, /* ////////////////////////////////////////////////////////////////////-PADDING */ \
        FREE_OBJECT_WITH_NONE_FIELD, /* ///////////////////////////////////////////////////////////////////-PADDING */ \
        FREE_OBJECT_WITH_TWO_FIELD, /* ////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_NATIVE_POINTER, /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        GLOBAL_ENV,        /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        ACCESSOR_DATA,     /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        INTERNAL_ACCESSOR, /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        SYMBOL, /* ////////////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        JS_GENERATOR_CONTEXT, /* //////////////////////////////////////////////////////////////////////////-PADDING */ \
        PROTOTYPE_HANDLER,    /* //////////////////////////////////////////////////////////////////////////-PADDING */ \
        TRANSITION_HANDLER,   /* //////////////////////////////////////////////////////////////////////////-PADDING */ \
        TRANS_WITH_PROTO_HANDLER,    /* ///////////////////////////////////////////////////////////////////-PADDING */ \
        STORE_TS_HANDLER,       /* ////////////////////////////////////////////////////////////////////////-PADDING */ \
        PROPERTY_BOX, /* /////////////////////////////////////////////////////////////////////////////////-PADDING */  \
        PROTO_CHANGE_MARKER, /* ///////////////////////////////////////////////////////////////////////////-PADDING */ \
        PROTOTYPE_INFO,     /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        TEMPLATE_MAP,       /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        PROGRAM,       /* /////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        METHOD,     /* ////////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        CLASS_LITERAL,      /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
                                                                                                                       \
        PROMISE_CAPABILITY, /* JS_RECORD_FIRST //////////////////////////////////////////////////////////////////// */ \
        PROMISE_RECORD,     /* ////////////////////////////////////////////////////////////////////////////-PADDING */ \
        RESOLVING_FUNCTIONS_RECORD, /* ////////////////////////////////////////////////////////////////////-PADDING */ \
        PROMISE_REACTIONS,          /* ////////////////////////////////////////////////////////////////////-PADDING */ \
        ASYNC_GENERATOR_REQUEST, /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        ASYNC_ITERATOR_RECORD,   /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        PROMISE_ITERATOR_RECORD,    /* ////////////////////////////////////////////////////////////////////-PADDING */ \
        MICRO_JOB_QUEUE, /* /////////////////////////////////////////////////////////////////////////////-PADDING */   \
        PENDING_JOB,     /* /////////////////////////////////////////////////////////////////////////////-PADDING */   \
        MODULE_RECORD, /* //////////////////////////////////////////////////////////////////////////////-PADDING */    \
        SOURCE_TEXT_MODULE_RECORD, /* //////////////////////////////////////////////////////////////////-PADDING */    \
        IMPORTENTRY_RECORD, /* /////////////////////////////////////////////////////////////////////////-PADDING */    \
        LOCAL_EXPORTENTRY_RECORD, /* ///////////////////////////////////////////////////////////////////-PADDING */    \
        INDIRECT_EXPORTENTRY_RECORD, /* ////////////////////////////////////////////////////////////////-PADDING */    \
        STAR_EXPORTENTRY_RECORD, /* ////////////////////////////////////////////////////////////////////-PADDING */    \
        RESOLVEDBINDING_RECORD, /* /////////////////////////////////////////////////////////////////////-PADDING */    \
        RESOLVEDINDEXBINDING_RECORD, /* ////////////////////////////////////////////////////////////////-PADDING */    \
        CELL_RECORD,          /* //////////////////////////////////////////////////////////////////////////-PADDING */ \
        COMPLETION_RECORD, /* JS_RECORD_LAST /////////////////////////////////////////////////////////////////////// */\
        MACHINE_CODE_OBJECT,                                                                                           \
        CLASS_INFO_EXTRACTOR, /* //////////////////////////////////////////////////////////////////////////-PADDING */ \
        TS_ARRAY_TYPE,  /* ////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        TS_UNION_TYPE,  /* ////////////////////////////////////////////////////////////////////////////////-PADDING */ \
        TS_FUNCTION_TYPE,  /* /////////////////////////////////////////////////////////////////////////////-PADDING */ \
        TS_OBJECT_TYPE,  /* ///////////////////////////////////////////////////////////////////////////////-PADDING */ \
        TS_CLASS_TYPE,    /* //////////////////////////////////////////////////////////////////////////////-PADDING */ \
        TS_CLASS_INSTANCE_TYPE,  /* ///////////////////////////////////////////////////////////////////////-PADDING */ \
        TS_INTERFACE_TYPE,    /* //////////////////////////////////////////////////////////////////////////-PADDING */ \
        TS_ITERATOR_INSTANCE_TYPE,    /* //////////////////////////////////////////////////////////////////-PADDING */ \
                                                                                                                       \
        AOT_LITERAL_INFO, /* //////////////////////////////////////////////////////////////////////////////-PADDING */ \
        TYPE_LAST = AOT_LITERAL_INFO, /* //////////////////////////////////////////////////////////////////-PADDING */ \
                                                                                                                       \
        JS_FUNCTION_FIRST = JS_FUNCTION, /* ///////////////////////////////////////////////////////////////-PADDING */ \
        JS_FUNCTION_LAST = JS_ASYNC_AWAIT_STATUS_FUNCTION, /* //////////////////////////////////////////////-PADDING */\
                                                                                                                       \
        JS_OBJECT_FIRST = JS_OBJECT, /* ///////////////////////////////////////////////////////////////////-PADDING */ \
        JS_OBJECT_LAST = JS_GLOBAL_OBJECT, /* //////////////////////////////////////////////////////////////-PADDING */\
                                                                                                                       \
        ECMA_OBJECT_FIRST = JS_OBJECT, /* /////////////////////////////////////////////////////////////////-PADDING */ \
        ECMA_OBJECT_LAST = JS_PROXY,    /* /////////////////////////////////////////////////////////////////-PADDING */\
                                                                                                                       \
        JS_ERROR_FIRST = JS_ERROR,      /* ////////////////////////////////////////////////////////////////-PADDING */ \
        JS_ERROR_LAST = JS_OOM_ERROR,    /* ////////////////////////////////////////////////////////////////-PADDING */\
                                                                                                                       \
        JS_ITERATOR_FIRST = JS_ITERATOR,      /* //////////////////////////////////////////////////////////-PADDING */ \
        JS_ITERATOR_LAST = JS_STRING_ITERATOR, /* //////////////////////////////////////////////////////////-PADDING */\
                                                                                                                       \
        JS_RECORD_FIRST = PROMISE_CAPABILITY, /* //////////////////////////////////////////////////////////-PADDING */ \
        JS_RECORD_LAST = COMPLETION_RECORD,    /* ///////////////////////////////////////////////////////-PADDING */   \
                                                                                                                       \
        JS_TYPED_ARRAY_FIRST = JS_TYPED_ARRAY, /* /////////////////////////////////////////////////////////-PADDING */ \
        JS_TYPED_ARRAY_LAST = JS_BIGUINT64_ARRAY, /* ///////////////////////////////////////////////////////-PADDING */\
                                                                                                                       \
        MODULE_RECORD_FIRST = MODULE_RECORD, /* ///////////////////////////////////////////////////////////-PADDING */ \
        MODULE_RECORD_LAST = SOURCE_TEXT_MODULE_RECORD, /* ////////////////////////////////////////////////-PADDING */ \
                                                                                                                       \
        TS_TYPE_FIRST = TS_ARRAY_TYPE, /* /////////////////////////////////////////////////////////////////-PADDING */ \
        TS_TYPE_LAST = TS_ITERATOR_INSTANCE_TYPE /* ///////////////////////////////////////////////////////-PADDING */


enum class JSType : uint8_t {
    JSTYPE_DECL,
};

class JSHClass : public TaggedObject {
public:
    static constexpr int TYPE_BITFIELD_NUM = 8;
    using ObjectTypeBits = BitField<JSType, 0, TYPE_BITFIELD_NUM>;  // 8
    using CallableBit = ObjectTypeBits::NextFlag;
    using ConstructorBit = CallableBit::NextFlag;      // 10
    using ExtensibleBit = ConstructorBit::NextFlag;
    using IsPrototypeBit = ExtensibleBit::NextFlag;
    using ElementRepresentationBits = IsPrototypeBit::NextField<Representation, 3>;        // 3 means next 3 bit
    using DictionaryElementBits = ElementRepresentationBits::NextFlag;                     // 16
    using IsDictionaryBit = DictionaryElementBits::NextFlag;                               // 17
    using IsStableElementsBit = IsDictionaryBit::NextFlag;                                 // 18
    using HasConstructorBits = IsStableElementsBit::NextFlag;                              // 19
    using IsLiteralBit = HasConstructorBits::NextFlag;                                     // 20
    using ClassConstructorBit = IsLiteralBit::NextFlag;                                    // 21
    using ClassPrototypeBit = ClassConstructorBit::NextFlag;                               // 22
    using GlobalConstOrBuiltinsObjectBit = ClassPrototypeBit::NextFlag;                    // 23
    using IsTSBit = GlobalConstOrBuiltinsObjectBit::NextFlag;                              // 24

    static constexpr int DEFAULT_CAPACITY_OF_IN_OBJECTS = 4;
    static constexpr int MAX_CAPACITY_OF_OUT_OBJECTS =
        PropertyAttributes::MAX_CAPACITY_OF_PROPERTIES - DEFAULT_CAPACITY_OF_IN_OBJECTS;
    static constexpr int OFFSET_MAX_OBJECT_SIZE_IN_WORDS_WITHOUT_INLINED = 5;
    static constexpr int OFFSET_MAX_OBJECT_SIZE_IN_WORDS =
        PropertyAttributes::OFFSET_BITFIELD_NUM + OFFSET_MAX_OBJECT_SIZE_IN_WORDS_WITHOUT_INLINED;
    static constexpr int MAX_OBJECT_SIZE_IN_WORDS = (1U << OFFSET_MAX_OBJECT_SIZE_IN_WORDS) - 1;

    using NumberOfPropsBits = BitField<uint32_t, 0, PropertyAttributes::OFFSET_BITFIELD_NUM>; // 10
    using InlinedPropsStartBits = NumberOfPropsBits::NextField<uint32_t,
            OFFSET_MAX_OBJECT_SIZE_IN_WORDS_WITHOUT_INLINED>; // 15
    using ObjectSizeInWordsBits = InlinedPropsStartBits::NextField<uint32_t, OFFSET_MAX_OBJECT_SIZE_IN_WORDS>; // 30

    static JSHClass *Cast(const TaggedObject *object);

    inline size_t SizeFromJSHClass(TaggedObject *header);
    inline bool HasReferenceField();

    // size need to add inlined property numbers
    void Initialize(const JSThread *thread, uint32_t size, JSType type, uint32_t inlinedProps);

    static JSHandle<JSHClass> Clone(const JSThread *thread, const JSHandle<JSHClass> &jshclass,
                                    bool withoutInlinedProperties = false);
    static JSHandle<JSHClass> CloneWithoutInlinedProperties(const JSThread *thread, const JSHandle<JSHClass> &jshclass);

    static void TransitionElementsToDictionary(const JSThread *thread, const JSHandle<JSObject> &obj);
    static JSHandle<JSHClass> SetPropertyOfObjHClass(const JSThread *thread, JSHandle<JSHClass> &jshclass,
                                                     const JSHandle<JSTaggedValue> &key,
                                                     const PropertyAttributes &attr);
    static void AddProperty(const JSThread *thread, const JSHandle<JSObject> &obj, const JSHandle<JSTaggedValue> &key,
                            const PropertyAttributes &attr);

    static JSHandle<JSHClass> TransitionExtension(const JSThread *thread, const JSHandle<JSHClass> &jshclass);
    static JSHandle<JSHClass> TransitionProto(const JSThread *thread, const JSHandle<JSHClass> &jshclass,
                                              const JSHandle<JSTaggedValue> &proto);
    static JSHandle<JSHClass> TransProtoWithoutLayout(const JSThread *thread, const JSHandle<JSHClass> &jshclass,
                                              const JSHandle<JSTaggedValue> &proto);
    static void TransitionToDictionary(const JSThread *thread, const JSHandle<JSObject> &obj);

    static JSHandle<JSTaggedValue> EnableProtoChangeMarker(const JSThread *thread, const JSHandle<JSHClass> &jshclass);

    static void NotifyHclassChanged(const JSThread *thread, JSHandle<JSHClass> oldHclass, JSHandle<JSHClass> newHclass);

    static void RegisterOnProtoChain(const JSThread *thread, const JSHandle<JSHClass> &jshclass);

    static bool UnregisterOnProtoChain(const JSThread *thread, const JSHandle<JSHClass> &jshclass);

    static JSHandle<ProtoChangeDetails> GetProtoChangeDetails(const JSThread *thread,
                                                              const JSHandle<JSHClass> &jshclass);

    static JSHandle<ProtoChangeDetails> GetProtoChangeDetails(const JSThread *thread, const JSHandle<JSObject> &obj);

    inline void UpdatePropertyMetaData(const JSThread *thread, const JSTaggedValue &key,
                                      const PropertyAttributes &metaData);

    static void NoticeRegisteredUser(const JSThread *thread, const JSHandle<JSHClass> &jshclass);

    static void NoticeThroughChain(const JSThread *thread, const JSHandle<JSHClass> &jshclass);

    static void RefreshUsers(const JSThread *thread, const JSHandle<JSHClass> &oldHclass,
                             const JSHandle<JSHClass> &newHclass);

    inline void ClearBitField()
    {
        SetBitField(0UL);
        SetBitField1(0UL);
    }

    inline JSType GetObjectType() const
    {
        uint32_t bits = GetBitField();
        return ObjectTypeBits::Decode(bits);
    }

    inline void SetObjectType(JSType type)
    {
        uint32_t bits = GetBitField();
        uint32_t newVal = ObjectTypeBits::Update(bits, type);
        SetBitField(newVal);
    }

    inline void SetCallable(bool flag)
    {
        CallableBit::Set<uint32_t>(flag, GetBitFieldAddr());
    }

    inline void SetConstructor(bool flag) const
    {
        ConstructorBit::Set<uint32_t>(flag, GetBitFieldAddr());
    }

    inline void SetExtensible(bool flag) const
    {
        ExtensibleBit::Set<uint32_t>(flag, GetBitFieldAddr());
    }

    inline void SetIsPrototype(bool flag) const
    {
        IsPrototypeBit::Set<uint32_t>(flag, GetBitFieldAddr());
    }

    inline void SetIsLiteral(bool flag) const
    {
        IsLiteralBit::Set<uint32_t>(flag, GetBitFieldAddr());
    }

    inline void SetClassConstructor(bool flag) const
    {
        ClassConstructorBit::Set<uint32_t>(flag, GetBitFieldAddr());
    }

    inline void SetClassPrototype(bool flag) const
    {
        ClassPrototypeBit::Set<uint32_t>(flag, GetBitFieldAddr());
    }

    inline void SetGlobalConstOrBuiltinsObject(bool flag) const
    {
        GlobalConstOrBuiltinsObjectBit::Set<uint32_t>(flag, GetBitFieldAddr());
    }

    inline void SetIsDictionaryMode(bool flag) const
    {
        IsDictionaryBit::Set<uint32_t>(flag, GetBitFieldAddr());
    }

    inline void SetTS(bool flag) const
    {
        IsTSBit::Set<uint32_t>(flag, GetBitFieldAddr());
    }

    inline bool IsJSObject() const
    {
        JSType jsType = GetObjectType();
        return (JSType::JS_OBJECT_FIRST <= jsType && jsType <= JSType::JS_OBJECT_LAST);
    }

    inline bool IsECMAObject() const
    {
        JSType jsType = GetObjectType();
        return (JSType::ECMA_OBJECT_FIRST <= jsType && jsType <= JSType::ECMA_OBJECT_LAST);
    }

    inline bool IsRealm() const
    {
        return GetObjectType() == JSType::JS_REALM;
    }

    inline bool IsHClass() const
    {
        return GetObjectType() == JSType::HCLASS;
    }

    inline bool IsString() const
    {
        return GetObjectType() == JSType::STRING;
    }

    inline bool IsBigInt() const
    {
        return GetObjectType() == JSType::BIGINT;
    }

    inline bool IsSymbol() const
    {
        return GetObjectType() == JSType::SYMBOL;
    }

    inline bool IsStringOrSymbol() const
    {
        JSType jsType = GetObjectType();
        return (jsType == JSType::STRING) || (jsType == JSType::SYMBOL);
    }

    inline bool IsTaggedArray() const
    {
        JSType jsType = GetObjectType();
        switch (jsType) {
            case JSType::TAGGED_ARRAY:
            case JSType::TAGGED_DICTIONARY:
            case JSType::LEXICAL_ENV:
            case JSType::CONSTANT_POOL:
            case JSType::AOT_LITERAL_INFO:
            case JSType::COW_TAGGED_ARRAY:
                return true;
            default:
                return false;
        }
    }

    inline bool IsByteArray() const
    {
        return GetObjectType() == JSType::BYTE_ARRAY;
    }

    inline bool IsConstantPool() const
    {
        return GetObjectType() == JSType::CONSTANT_POOL;
    }

    inline bool IsDictionary() const
    {
        return GetObjectType() == JSType::TAGGED_DICTIONARY;
    }

    inline bool IsCOWArray() const
    {
        // Copy On Write ARRAY.
        return GetObjectType() == JSType::COW_TAGGED_ARRAY;
    }

    inline bool IsJSNativePointer() const
    {
        return GetObjectType() == JSType::JS_NATIVE_POINTER;
    }

    inline bool IsJSSymbol() const
    {
        return GetObjectType() == JSType::SYMBOL;
    }

    inline bool IsJSArray() const
    {
        return GetObjectType() == JSType::JS_ARRAY;
    }

    inline bool IsTypedArray() const
    {
        JSType jsType = GetObjectType();
        return (JSType::JS_TYPED_ARRAY_FIRST < jsType && jsType <= JSType::JS_TYPED_ARRAY_LAST);
    }

    inline bool HasOrdinaryGet() const
    {
        return (IsTypedArray() || IsSpecialContainer() || IsModuleNamespace());
    }

    inline bool IsJSTypedArray() const
    {
        return GetObjectType() == JSType::JS_TYPED_ARRAY;
    }

    inline bool IsJSInt8Array() const
    {
        return GetObjectType() == JSType::JS_INT8_ARRAY;
    }

    inline bool IsJSUint8Array() const
    {
        return GetObjectType() == JSType::JS_UINT8_ARRAY;
    }

    inline bool IsJSUint8ClampedArray() const
    {
        return GetObjectType() == JSType::JS_UINT8_CLAMPED_ARRAY;
    }

    inline bool IsJSInt16Array() const
    {
        return GetObjectType() == JSType::JS_INT16_ARRAY;
    }

    inline bool IsJSUint16Array() const
    {
        return GetObjectType() == JSType::JS_UINT16_ARRAY;
    }

    inline bool IsJSInt32Array() const
    {
        return GetObjectType() == JSType::JS_INT32_ARRAY;
    }

    inline bool IsJSUint32Array() const
    {
        return GetObjectType() == JSType::JS_UINT32_ARRAY;
    }

    inline bool IsJSFloat32Array() const
    {
        return GetObjectType() == JSType::JS_FLOAT32_ARRAY;
    }

    inline bool IsJSFloat64Array() const
    {
        return GetObjectType() == JSType::JS_FLOAT64_ARRAY;
    }

    inline bool IsJSBigInt64Array() const
    {
        return GetObjectType() == JSType::JS_BIGINT64_ARRAY;
    }

    inline bool IsJSBigUint64Array() const
    {
        return GetObjectType() == JSType::JS_BIGUINT64_ARRAY;
    }

    inline bool IsJsGlobalEnv() const
    {
        return GetObjectType() == JSType::GLOBAL_ENV;
    }

    inline bool IsJSFunctionBase() const
    {
        JSType jsType = GetObjectType();
        return jsType >= JSType::JS_FUNCTION_BASE && jsType <= JSType::JS_BOUND_FUNCTION;
    }

    inline bool IsJsBoundFunction() const
    {
        return GetObjectType() == JSType::JS_BOUND_FUNCTION;
    }

    inline bool IsJSIntlBoundFunction() const
    {
        return GetObjectType() == JSType::JS_INTL_BOUND_FUNCTION;
    }

    inline bool IsJSProxyRevocFunction() const
    {
        return GetObjectType() == JSType::JS_PROXY_REVOC_FUNCTION;
    }

    inline bool IsJSAsyncFunction() const
    {
        return GetObjectType() == JSType::JS_ASYNC_FUNCTION;
    }

    inline bool IsJSAsyncAwaitStatusFunction() const
    {
        return GetObjectType() == JSType::JS_ASYNC_AWAIT_STATUS_FUNCTION;
    }

    inline bool IsJSPromiseReactionFunction() const
    {
        return GetObjectType() == JSType::JS_PROMISE_REACTIONS_FUNCTION;
    }

    inline bool IsJSPromiseExecutorFunction() const
    {
        return GetObjectType() == JSType::JS_PROMISE_EXECUTOR_FUNCTION;
    }

    inline bool IsJSAsyncFromSyncIterUnwarpFunction() const
    {
        return GetObjectType() == JSType::JS_ASYNC_FROM_SYNC_ITER_UNWARP_FUNCTION;
    }

    inline bool IsJSPromiseAllResolveElementFunction() const
    {
        return GetObjectType() == JSType::JS_PROMISE_ALL_RESOLVE_ELEMENT_FUNCTION;
    }

    inline bool IsJSAsyncGeneratorResNextRetProRstFtn() const
    {
        return GetObjectType() == JSType::JS_ASYNC_GENERATOR_RESUME_NEXT_RETURN_PROCESSOR_RST_FTN;
    }

    inline bool IsJSPromiseAnyRejectElementFunction() const
    {
        return GetObjectType() == JSType::JS_PROMISE_ANY_REJECT_ELEMENT_FUNCTION;
    }

    inline bool IsJSPromiseAllSettledElementFunction() const
    {
        return GetObjectType() == JSType::JS_PROMISE_ALL_SETTLED_ELEMENT_FUNCTION;
    }

    inline bool IsJSPromiseFinallyFunction() const
    {
        return GetObjectType() == JSType::JS_PROMISE_FINALLY_FUNCTION;
    }

    inline bool IsJSPromiseValueThunkOrThrowerFunction() const
    {
        return GetObjectType() == JSType::JS_PROMISE_VALUE_THUNK_OR_THROWER_FUNCTION;
    }

    inline bool IsMicroJobQueue() const
    {
        return GetObjectType() == JSType::MICRO_JOB_QUEUE;
    }

    inline bool IsPendingJob() const
    {
        return GetObjectType() == JSType::PENDING_JOB;
    }

    inline bool IsJsPrimitiveRef() const
    {
        return GetObjectType() == JSType::JS_PRIMITIVE_REF;
    };

    bool IsJSSet() const
    {
        return GetObjectType() == JSType::JS_SET;
    }

    bool IsJSMap() const
    {
        return GetObjectType() == JSType::JS_MAP;
    }

    bool IsJSWeakMap() const
    {
        return GetObjectType() == JSType::JS_WEAK_MAP;
    }

    bool IsJSWeakSet() const
    {
        return GetObjectType() == JSType::JS_WEAK_SET;
    }

    bool IsJSWeakRef() const
    {
        return GetObjectType() == JSType::JS_WEAK_REF;
    }

    bool IsJSFinalizationRegistry() const
    {
        return GetObjectType() == JSType::JS_FINALIZATION_REGISTRY;
    }

    bool IsJSFunction() const
    {
        return GetObjectType() >= JSType::JS_FUNCTION_FIRST && GetObjectType() <= JSType::JS_FUNCTION_LAST;
    }

    inline bool IsJSError() const
    {
        JSType jsType = GetObjectType();
        return jsType >= JSType::JS_ERROR_FIRST && jsType <= JSType::JS_ERROR_LAST;
    }

    inline bool IsArguments() const
    {
        return GetObjectType() == JSType::JS_ARGUMENTS;
    }

    inline bool IsDate() const
    {
        return GetObjectType() == JSType::JS_DATE;
    }

    inline bool IsJSRegExp() const
    {
        return GetObjectType() == JSType::JS_REG_EXP;
    }

    inline bool IsJSProxy() const
    {
        return GetObjectType() == JSType::JS_PROXY;
    }

    inline bool IsJSLocale() const
    {
        return GetObjectType() == JSType::JS_LOCALE;
    }

    inline bool IsJSIntl() const
    {
        return GetObjectType() == JSType::JS_INTL;
    }

    inline bool IsJSDateTimeFormat() const
    {
        return GetObjectType() == JSType::JS_DATE_TIME_FORMAT;
    }

    inline bool IsJSRelativeTimeFormat() const
    {
        return GetObjectType() == JSType::JS_RELATIVE_TIME_FORMAT;
    }

    inline bool IsJSNumberFormat() const
    {
        return GetObjectType() == JSType::JS_NUMBER_FORMAT;
    }

    inline bool IsJSCollator() const
    {
        return GetObjectType() == JSType::JS_COLLATOR;
    }

    inline bool IsJSPluralRules() const
    {
        return GetObjectType() == JSType::JS_PLURAL_RULES;
    }

    inline bool IsJSDisplayNames() const
    {
        return GetObjectType() == JSType::JS_DISPLAYNAMES;
    }

    inline bool IsJSListFormat() const
    {
        return GetObjectType() == JSType::JS_LIST_FORMAT;
    }

    inline bool IsMethod() const
    {
        return GetObjectType() == JSType::METHOD;
    }

    inline bool IsClassLiteral() const
    {
        return GetObjectType() == JSType::CLASS_LITERAL;
    }

    // non ECMA standard jsapi containers.
    inline bool IsSpecialContainer() const
    {
        return GetObjectType() >= JSType::JS_API_ARRAY_LIST && GetObjectType() <= JSType::JS_API_QUEUE;
    }

    inline bool IsJSAPIArrayList() const
    {
        return GetObjectType() == JSType::JS_API_ARRAY_LIST;
    }

    inline bool IsJSAPIArrayListIterator() const
    {
        return GetObjectType() == JSType::JS_API_ARRAYLIST_ITERATOR;
    }
    inline bool IsJSAPILightWeightMap() const
    {
        return GetObjectType() == JSType::JS_API_LIGHT_WEIGHT_MAP;
    }
    inline bool IsJSAPILightWeightMapIterator() const
    {
        return GetObjectType() == JSType::JS_API_LIGHT_WEIGHT_MAP_ITERATOR;
    }
    inline bool IsJSAPILightWeightSet() const
    {
        return GetObjectType() == JSType::JS_API_LIGHT_WEIGHT_SET;
    }
    inline bool IsJSAPILightWeightSetIterator() const
    {
        return GetObjectType() == JSType::JS_API_LIGHT_WEIGHT_SET_ITERATOR;
    }
    inline bool IsJSAPIStack() const
    {
        return GetObjectType() == JSType::JS_API_STACK;
    }
    inline bool IsJSAPIDeque() const
    {
        return GetObjectType() == JSType::JS_API_DEQUE;
    }
    inline bool IsLinkedNode() const
    {
        return GetObjectType() == JSType::LINKED_NODE;
    }

    inline bool IsRBTreeNode() const
    {
        return GetObjectType() == JSType::RB_TREENODE;
    }

    inline bool IsJSAPIHashMap() const
    {
        return GetObjectType() == JSType::JS_API_HASH_MAP;
    }

    inline bool IsJSAPIHashSet() const
    {
        return GetObjectType() == JSType::JS_API_HASH_SET;
    }

    inline bool IsJSAPIHashMapIterator() const
    {
        return GetObjectType() == JSType::JS_API_HASHMAP_ITERATOR;
    }

    inline bool IsJSAPIHashSetIterator() const
    {
        return GetObjectType() == JSType::JS_API_HASHSET_ITERATOR;
    }
    inline bool IsJSAPIQueue() const
    {
        return GetObjectType() == JSType::JS_API_QUEUE;
    }

    inline bool IsJSAPIPlainArray() const
    {
        return GetObjectType() == JSType::JS_API_PLAIN_ARRAY;
    }

    inline bool IsJSAPIQueueIterator() const
    {
        return GetObjectType() == JSType::JS_API_QUEUE_ITERATOR;
    }
    inline bool IsJSAPIList() const
    {
        return GetObjectType() == JSType::JS_API_LIST;
    }
    inline bool IsJSAPILinkedList() const
    {
        return GetObjectType() == JSType::JS_API_LINKED_LIST;
    }
    inline bool IsJSAPITreeMap() const
    {
        return GetObjectType() == JSType::JS_API_TREE_MAP;
    }

    inline bool IsJSAPITreeSet() const
    {
        return GetObjectType() == JSType::JS_API_TREE_SET;
    }

    inline bool IsJSAPITreeMapIterator() const
    {
        return GetObjectType() == JSType::JS_API_TREEMAP_ITERATOR;
    }

    inline bool IsJSAPITreeSetIterator() const
    {
        return GetObjectType() == JSType::JS_API_TREESET_ITERATOR;
    }
    inline bool IsJSAPIVector() const
    {
        return GetObjectType() == JSType::JS_API_VECTOR;
    }
    inline bool IsJSAPIVectorIterator() const
    {
        return GetObjectType() == JSType::JS_API_VECTOR_ITERATOR;
    }

    inline bool IsAccessorData() const
    {
        return GetObjectType() == JSType::ACCESSOR_DATA;
    }

    inline bool IsInternalAccessor() const
    {
        return GetObjectType() == JSType::INTERNAL_ACCESSOR;
    }

    inline bool IsIterator() const
    {
        JSType jsType = GetObjectType();
        return jsType >= JSType::JS_ITERATOR_FIRST && jsType <= JSType::JS_ITERATOR_LAST;
    }

    inline bool IsAsyncIterator() const
    {
        return GetObjectType() == JSType::JS_ASYNCITERATOR;
    }

    inline bool IsAsyncFromSyncIterator() const
    {
        return GetObjectType() == JSType::JS_ASYNC_FROM_SYNC_ITERATOR;
    }

    inline bool IsForinIterator() const
    {
        return GetObjectType() == JSType::JS_FORIN_ITERATOR;
    }

    inline bool IsStringIterator() const
    {
        return GetObjectType() == JSType::JS_STRING_ITERATOR;
    }

    inline bool IsArrayBuffer() const
    {
        return GetObjectType() == JSType::JS_ARRAY_BUFFER;
    }

    inline bool IsSharedArrayBuffer() const
    {
        return GetObjectType() == JSType::JS_SHARED_ARRAY_BUFFER;
    }

    inline bool IsDataView() const
    {
        return GetObjectType() == JSType::JS_DATA_VIEW;
    }

    inline bool IsJSSetIterator() const
    {
        return GetObjectType() == JSType::JS_SET_ITERATOR;
    }

    inline bool IsJSRegExpIterator() const
    {
        return GetObjectType() == JSType::JS_REG_EXP_ITERATOR;
    }

    inline bool IsJSMapIterator() const
    {
        return GetObjectType() == JSType::JS_MAP_ITERATOR;
    }

    inline bool IsJSArrayIterator() const
    {
        return GetObjectType() == JSType::JS_ARRAY_ITERATOR;
    }

    inline bool IsJSAPIPlainArrayIterator() const
    {
        return GetObjectType() == JSType::JS_API_PLAIN_ARRAY_ITERATOR;
    }

    inline bool IsJSAPIDequeIterator() const
    {
        return GetObjectType() == JSType::JS_API_DEQUE_ITERATOR;
    }

    inline bool IsJSAPIStackIterator() const
    {
        return GetObjectType() == JSType::JS_API_STACK_ITERATOR;
    }

    inline bool IsJSAPILinkedListIterator() const
    {
        return GetObjectType() == JSType::JS_API_LINKED_LIST_ITERATOR;
    }

    inline bool IsJSAPIListIterator() const
    {
        return GetObjectType() == JSType::JS_API_LIST_ITERATOR;
    }

    inline bool IsPrototypeHandler() const
    {
        return GetObjectType() == JSType::PROTOTYPE_HANDLER;
    }

    inline bool IsTransitionHandler() const
    {
        return GetObjectType() == JSType::TRANSITION_HANDLER;
    }

    inline bool IsTransWithProtoHandler() const
    {
        return GetObjectType() == JSType::TRANS_WITH_PROTO_HANDLER;
    }

    inline bool IsStoreTSHandler() const
    {
        return GetObjectType() == JSType::STORE_TS_HANDLER;
    }

    inline bool IsPropertyBox() const
    {
        return GetObjectType() == JSType::PROPERTY_BOX;
    }
    inline bool IsProtoChangeMarker() const
    {
        return GetObjectType() == JSType::PROTO_CHANGE_MARKER;
    }

    inline bool IsProtoChangeDetails() const
    {
        return GetObjectType() == JSType::PROTOTYPE_INFO;
    }

    inline bool IsProgram() const
    {
        return GetObjectType() == JSType::PROGRAM;
    }

    inline bool IsClassInfoExtractor() const
    {
        return GetObjectType() == JSType::CLASS_INFO_EXTRACTOR;
    }

    inline bool IsCallable() const
    {
        uint32_t bits = GetBitField();
        return CallableBit::Decode(bits);
    }

    inline bool IsConstructor() const
    {
        uint32_t bits = GetBitField();
        return ConstructorBit::Decode(bits);
    }

    inline bool IsExtensible() const
    {
        uint32_t bits = GetBitField();
        return ExtensibleBit::Decode(bits);
    }

    inline bool IsPrototype() const
    {
        uint32_t bits = GetBitField();
        return IsPrototypeBit::Decode(bits);
    }

    inline bool IsLiteral() const
    {
        uint32_t bits = GetBitField();
        return IsLiteralBit::Decode(bits);
    }

    inline bool IsClassConstructor() const
    {
        uint32_t bits = GetBitField();
        return ClassConstructorBit::Decode(bits);
    }

    inline bool IsJSGlobalObject() const
    {
        return GetObjectType() == JSType::JS_GLOBAL_OBJECT;
    }

    inline bool IsClassPrototype() const
    {
        uint32_t bits = GetBitField();
        return ClassPrototypeBit::Decode(bits);
    }

    inline bool IsGlobalConstOrBuiltinsObject() const
    {
        uint32_t bits = GetBitField();
        return GlobalConstOrBuiltinsObjectBit::Decode(bits);
    }

    inline bool IsDictionaryMode() const
    {
        uint32_t bits = GetBitField();
        return IsDictionaryBit::Decode(bits);
    }

    // created from TypeScript Types
    inline bool IsTS() const
    {
        uint32_t bits = GetBitField();
        return IsTSBit::Decode(bits);
    }

    inline bool IsGeneratorFunction() const
    {
        return GetObjectType() == JSType::JS_GENERATOR_FUNCTION;
    }

    inline bool IsAsyncGeneratorFunction() const
    {
        return GetObjectType() == JSType::JS_ASYNC_GENERATOR_FUNCTION;
    }

    inline bool IsGeneratorObject() const
    {
        JSType jsType = GetObjectType();
        return jsType == JSType::JS_GENERATOR_OBJECT || jsType == JSType::JS_ASYNC_FUNC_OBJECT;
    }

    inline bool IsAsyncGeneratorObject() const
    {
        JSType jsType = GetObjectType();
        return jsType == JSType::JS_ASYNC_GENERATOR_OBJECT;
    }

    inline bool IsGeneratorContext() const
    {
        return GetObjectType() == JSType::JS_GENERATOR_CONTEXT;
    }

    inline bool IsAsyncGeneratorRequest() const
    {
        JSType jsType = GetObjectType();
        return jsType == JSType::ASYNC_GENERATOR_REQUEST;
    }

    inline bool IsAsyncIteratorRecord() const
    {
        JSType jsType = GetObjectType();
        return jsType == JSType::ASYNC_ITERATOR_RECORD;
    }

    inline bool IsAsyncFuncObject() const
    {
        return GetObjectType() == JSType::JS_ASYNC_FUNC_OBJECT;
    }

    inline bool IsJSPromise() const
    {
        return GetObjectType() == JSType::JS_PROMISE;
    }

    inline bool IsResolvingFunctionsRecord() const
    {
        return GetObjectType() == JSType::RESOLVING_FUNCTIONS_RECORD;
    }

    inline bool IsPromiseRecord() const
    {
        return GetObjectType() == JSType::PROMISE_RECORD;
    }

    inline bool IsPromiseIteratorRecord() const
    {
        return GetObjectType() == JSType::PROMISE_ITERATOR_RECORD;
    }

    inline bool IsPromiseCapability() const
    {
        return GetObjectType() == JSType::PROMISE_CAPABILITY;
    }

    inline bool IsPromiseReaction() const
    {
        return GetObjectType() == JSType::PROMISE_REACTIONS;
    }

    inline bool IsCellRecord() const
    {
        return GetObjectType() == JSType::CELL_RECORD;
    }

    inline bool IsCompletionRecord() const
    {
        return GetObjectType() == JSType::COMPLETION_RECORD;
    }

    inline bool IsRecord() const
    {
        JSType jsType = GetObjectType();
        return jsType >= JSType::JS_RECORD_FIRST && jsType <= JSType::JS_RECORD_LAST;
    }

    inline bool IsTemplateMap() const
    {
        return GetObjectType() == JSType::TEMPLATE_MAP;
    }

    inline bool IsFreeObject() const
    {
        JSType t = GetObjectType();
        return (JSType::FREE_OBJECT_WITH_ONE_FIELD <= t) && (t <= JSType::FREE_OBJECT_WITH_TWO_FIELD);
    }

    inline bool IsFreeObjectWithShortField() const
    {
        switch (GetObjectType()) {
            case JSType::FREE_OBJECT_WITH_ONE_FIELD:
            case JSType::FREE_OBJECT_WITH_NONE_FIELD:
                return true;
            default:
                return false;
        }
    }

    inline bool IsFreeObjectWithOneField() const
    {
        return GetObjectType() == JSType::FREE_OBJECT_WITH_ONE_FIELD;
    }

    inline bool IsFreeObjectWithNoneField() const
    {
        return GetObjectType() == JSType::FREE_OBJECT_WITH_NONE_FIELD;
    }

    inline bool IsFreeObjectWithTwoField() const
    {
        return GetObjectType() == JSType::FREE_OBJECT_WITH_TWO_FIELD;
    }

    inline bool IsMachineCodeObject() const
    {
        return GetObjectType() == JSType::MACHINE_CODE_OBJECT;
    }

    inline bool IsTSType() const
    {
        JSType jsType = GetObjectType();
        return jsType >= JSType::TS_TYPE_FIRST && jsType <= JSType::TS_TYPE_LAST;
    }

    inline bool IsTSObjectType() const
    {
        return GetObjectType() == JSType::TS_OBJECT_TYPE;
    }

    inline bool IsTSClassType() const
    {
        return GetObjectType() == JSType::TS_CLASS_TYPE;
    }

    inline bool IsTSInterfaceType() const
    {
        return GetObjectType() == JSType::TS_INTERFACE_TYPE;
    }

    inline bool IsTSUnionType() const
    {
        return GetObjectType() == JSType::TS_UNION_TYPE;
    }

    inline bool IsTSClassInstanceType() const
    {
        return GetObjectType() == JSType::TS_CLASS_INSTANCE_TYPE;
    }

    inline bool IsTSFunctionType() const
    {
        return GetObjectType() == JSType::TS_FUNCTION_TYPE;
    }

    inline bool IsTSArrayType() const
    {
        return GetObjectType() == JSType::TS_ARRAY_TYPE;
    }

    inline bool IsTSIteratorInstanceType() const
    {
        return GetObjectType() == JSType::TS_ITERATOR_INSTANCE_TYPE;
    }

    inline bool IsAOTLiteralInfo() const
    {
        return GetObjectType() == JSType::AOT_LITERAL_INFO;
    }

    inline bool IsModuleRecord() const
    {
        JSType jsType = GetObjectType();
        return jsType >= JSType::MODULE_RECORD_FIRST && jsType <= JSType::MODULE_RECORD_LAST;
    }

    inline bool IsSourceTextModule() const
    {
        return GetObjectType() == JSType::SOURCE_TEXT_MODULE_RECORD;
    }

    inline bool IsCjsExports() const
    {
        return GetObjectType() == JSType::JS_CJS_EXPORTS;
    }

    inline bool IsCjsModule() const
    {
        return GetObjectType() == JSType::JS_CJS_MODULE;
    }

    inline bool IsCjsRequire() const
    {
        return GetObjectType() == JSType::JS_CJS_REQUIRE;
    }

    inline bool IsImportEntry() const
    {
        return GetObjectType() == JSType::IMPORTENTRY_RECORD;
    }

    inline bool IsLocalExportEntry() const
    {
        return GetObjectType() == JSType::LOCAL_EXPORTENTRY_RECORD;
    }

    inline bool IsIndirectExportEntry() const
    {
        return GetObjectType() == JSType::INDIRECT_EXPORTENTRY_RECORD;
    }

    inline bool IsStarExportEntry() const
    {
        return GetObjectType() == JSType::STAR_EXPORTENTRY_RECORD;
    }

    inline bool IsResolvedBinding() const
    {
        return GetObjectType() == JSType::RESOLVEDBINDING_RECORD;
    }

    inline bool IsResolvedIndexBinding() const
    {
        return GetObjectType() == JSType::RESOLVEDINDEXBINDING_RECORD;
    }

    inline bool IsModuleNamespace() const
    {
        return GetObjectType() == JSType::JS_MODULE_NAMESPACE;
    }

    inline void SetElementRepresentation(Representation representation)
    {
        uint32_t bits = GetBitField();
        uint32_t newVal = ElementRepresentationBits::Update(bits, representation);
        SetBitField(newVal);
    }

    inline Representation GetElementRepresentation() const
    {
        uint32_t bits = GetBitField();
        return ElementRepresentationBits::Decode(bits);
    }

    inline void UpdateRepresentation(JSTaggedValue value)
    {
        Representation rep = PropertyAttributes::UpdateRepresentation(GetElementRepresentation(), value);
        SetElementRepresentation(rep);
    }

    inline void SetIsDictionaryElement(bool value)
    {
        uint32_t newVal = DictionaryElementBits::Update(GetBitField(), value);
        SetBitField(newVal);
    }
    inline bool IsDictionaryElement() const
    {
        return DictionaryElementBits::Decode(GetBitField());
    }
    inline void SetIsStableElements(bool value)
    {
        uint32_t newVal = IsStableElementsBit::Update(GetBitField(), value);
        SetBitField(newVal);
    }
    inline bool IsStableElements() const
    {
        return IsStableElementsBit::Decode(GetBitField());
    }
    inline bool IsStableJSArguments() const
    {
        uint32_t bits = GetBitField();
        auto type = ObjectTypeBits::Decode(bits);
        return IsStableElementsBit::Decode(bits) && (type == JSType::JS_ARGUMENTS);
    }
    inline bool IsStableJSArray() const
    {
        uint32_t bits = GetBitField();
        auto type = ObjectTypeBits::Decode(bits);
        return IsStableElementsBit::Decode(bits) && (type == JSType::JS_ARRAY);
    }
    inline void SetHasConstructor(bool value)
    {
        JSTaggedType newVal = HasConstructorBits::Update(GetBitField(), value);
        SetBitField(newVal);
    }
    inline bool HasConstructor() const
    {
        return HasConstructorBits::Decode(GetBitField());
    }

    inline void SetNumberOfProps(uint32_t num)
    {
        uint32_t bits = GetBitField1();
        uint32_t newVal = NumberOfPropsBits::Update(bits, num);
        SetBitField1(newVal);
    }

    inline void IncNumberOfProps()
    {
        ASSERT(NumberOfProps() < PropertyAttributes::MAX_CAPACITY_OF_PROPERTIES);
        SetNumberOfProps(NumberOfProps() + 1);
    }

    inline uint32_t NumberOfProps() const
    {
        uint32_t bits = GetBitField1();
        return NumberOfPropsBits::Decode(bits);
    }

    inline int32_t GetNextInlinedPropsIndex() const
    {
        uint32_t inlinedProperties = GetInlinedProperties();
        uint32_t numberOfProps = NumberOfProps();
        if (numberOfProps < inlinedProperties) {
            return numberOfProps;
        }
        return -1;
    }

    inline int32_t GetNextNonInlinedPropsIndex() const
    {
        uint32_t inlinedProperties = GetInlinedProperties();
        uint32_t numberOfProps = NumberOfProps();
        if (numberOfProps >= inlinedProperties) {
            return numberOfProps - inlinedProperties;
        }
        return -1;
    }

    inline uint32_t GetObjectSize() const
    {
        uint32_t bits = GetBitField1();
        return ObjectSizeInWordsBits::Decode(bits) * JSTaggedValue::TaggedTypeSize();
    }

    inline void SetObjectSize(uint32_t num)
    {
        ASSERT((num / JSTaggedValue::TaggedTypeSize()) <= MAX_OBJECT_SIZE_IN_WORDS);
        uint32_t bits = GetBitField1();
        uint32_t newVal = ObjectSizeInWordsBits::Update(bits, num / JSTaggedValue::TaggedTypeSize());
        SetBitField1(newVal);
    }

    inline uint32_t GetInlinedPropertiesOffset(uint32_t index) const
    {
        ASSERT(index < GetInlinedProperties());
        return GetInlinedPropertiesIndex(index) * JSTaggedValue::TaggedTypeSize();
    }

    inline uint32_t GetInlinedPropertiesIndex(uint32_t index) const
    {
        ASSERT(index < GetInlinedProperties());
        uint32_t bits = GetBitField1();
        return InlinedPropsStartBits::Decode(bits) + index;
    }

    inline void SetInlinedPropsStart(uint32_t num)
    {
        uint32_t bits = GetBitField1();
        uint32_t newVal = InlinedPropsStartBits::Update(bits, num / JSTaggedValue::TaggedTypeSize());
        SetBitField1(newVal);
    }

    inline uint32_t GetInlinedPropsStartSize() const
    {
        uint32_t bits = GetBitField1();
        return InlinedPropsStartBits::Decode(bits) * JSTaggedValue::TaggedTypeSize();
    }

    inline uint32_t GetInlinedProperties() const
    {
        JSType type = GetObjectType();
        if (JSType::JS_OBJECT_FIRST <= type && type <= JSType::JS_OBJECT_LAST) {
            uint32_t bits = GetBitField1();
            return static_cast<uint32_t>(ObjectSizeInWordsBits::Decode(bits) - InlinedPropsStartBits::Decode(bits));
        } else {
            return 0;
        }
    }

    static constexpr size_t PROTOTYPE_OFFSET = TaggedObjectSize();
    ACCESSORS(Proto, PROTOTYPE_OFFSET, LAYOUT_OFFSET);
    ACCESSORS(Layout, LAYOUT_OFFSET, TRANSTIONS_OFFSET);
    ACCESSORS(Transitions, TRANSTIONS_OFFSET, PROTO_CHANGE_MARKER_OFFSET);
    ACCESSORS(ProtoChangeMarker, PROTO_CHANGE_MARKER_OFFSET, PROTO_CHANGE_DETAILS_OFFSET);
    ACCESSORS(ProtoChangeDetails, PROTO_CHANGE_DETAILS_OFFSET, ENUM_CACHE_OFFSET);
    ACCESSORS(EnumCache, ENUM_CACHE_OFFSET, BIT_FIELD_OFFSET);
    ACCESSORS_PRIMITIVE_FIELD(BitField, uint32_t, BIT_FIELD_OFFSET, BIT_FIELD1_OFFSET);
    ACCESSORS_PRIMITIVE_FIELD(BitField1, uint32_t, BIT_FIELD1_OFFSET, LAST_OFFSET)
    DEFINE_ALIGN_SIZE(LAST_OFFSET);

    void SetPrototype(const JSThread *thread, JSTaggedValue proto);
    void SetPrototype(const JSThread *thread, const JSHandle<JSTaggedValue> &proto);
    void ShouldUpdateProtoClass(const JSThread *thread, const JSHandle<JSTaggedValue> &proto);
    inline JSTaggedValue GetPrototype() const
    {
        return GetProto();
    }

    inline JSHClass *FindTransitions(const JSTaggedValue &key, const JSTaggedValue &attributes);

    DECL_DUMP()

    static CString DumpJSType(JSType type);

    DECL_VISIT_OBJECT(PROTOTYPE_OFFSET, BIT_FIELD_OFFSET);

private:
    static inline void AddTransitions(const JSThread *thread, const JSHandle<JSHClass> &parent,
                                      const JSHandle<JSHClass> &child, const JSHandle<JSTaggedValue> &key,
                                      PropertyAttributes attr);
    static inline void AddExtensionTransitions(const JSThread *thread, const JSHandle<JSHClass> &parent,
                                               const JSHandle<JSHClass> &child, const JSHandle<JSTaggedValue> &key);
    static inline void AddProtoTransitions(const JSThread *thread, const JSHandle<JSHClass> &parent,
                                           const JSHandle<JSHClass> &child, const JSHandle<JSTaggedValue> &key,
                                           const JSHandle<JSTaggedValue> &proto);
    inline JSHClass *FindProtoTransitions(const JSTaggedValue &key, const JSTaggedValue &proto);

    inline void Copy(const JSThread *thread, const JSHClass *jshclass);

    uint32_t *GetBitFieldAddr() const
    {
        return reinterpret_cast<uint32_t *>(ToUintPtr(this) + BIT_FIELD_OFFSET);
    }
    friend class RuntimeStubs;
};
static_assert(JSHClass::BIT_FIELD_OFFSET % static_cast<uint8_t>(MemAlignment::MEM_ALIGN_OBJECT) == 0);
}  // namespace panda::ecmascript

#endif  // ECMASCRIPT_JS_HCLASS_H
