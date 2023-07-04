/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_RUNTIME_EXCEPTIONS_H_
#define PANDA_RUNTIME_EXCEPTIONS_H_

#include "runtime/include/class-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/language_context.h"
#include "runtime/include/method.h"

namespace panda {

void ThrowException(const LanguageContext &ctx, ManagedThread *thread, const uint8_t *mutf8_name,
                    const uint8_t *mutf8_msg);

void ThrowNullPointerException();
// This function could be used in case there are no managed stack frames.
// For example when the main thread creates an instance of VM and calls
// a native function which throws an exception. In this case there is no need to
// get current executing method from the managed stack.
void ThrowNullPointerException(const LanguageContext &ctx, ManagedThread *thread);

void ThrowStackOverflowException(ManagedThread *thread);

void ThrowArrayIndexOutOfBoundsException(coretypes::array_ssize_t idx, coretypes::array_size_t length);
void ThrowArrayIndexOutOfBoundsException(coretypes::array_ssize_t idx, coretypes::array_size_t length,
                                         const LanguageContext &ctx, ManagedThread *thread);

void ThrowIndexOutOfBoundsException(coretypes::array_ssize_t idx, coretypes::array_ssize_t length);

void ThrowIllegalStateException(const PandaString &msg);

void ThrowStringIndexOutOfBoundsException(coretypes::array_ssize_t idx, coretypes::array_size_t length);

void ThrowNegativeArraySizeException(coretypes::array_ssize_t size);

void ThrowNegativeArraySizeException(const PandaString &msg);

void ThrowArithmeticException();

void ThrowClassCastException(const Class *dst_type, const Class *src_type);

void ThrowAbstractMethodError(const Method *method);

void ThrowIncompatibleClassChangeErrorForMethodConflict(const Method *method);

void ThrowArrayStoreException(const Class *array_class, const Class *element_class);

void ThrowArrayStoreException(const PandaString &msg);

void ThrowRuntimeException(const PandaString &msg);

void ThrowFileNotFoundException(const PandaString &msg);

void ThrowIOException(const PandaString &msg);

void ThrowIllegalArgumentException(const PandaString &msg);

void ThrowClassCircularityError(const PandaString &class_name, const LanguageContext &ctx);

void ThrowOutOfMemoryError(ManagedThread *thread, const PandaString &msg);

void ThrowOutOfMemoryError(const PandaString &msg);

void FindCatchBlockInCallStack(ManagedThread *thread);

void FindCatchBlockInCFrames(ManagedThread *thread, StackWalker *stack, Frame *orig_frame);

void ThrowIllegalAccessException(const PandaString &msg);

void ThrowUnsupportedOperationException();

void ThrowVerificationException(const PandaString &msg);

void ThrowVerificationException(const LanguageContext &ctx, const PandaString &msg);

void ThrowInstantiationError(const PandaString &msg);

void ThrowNoClassDefFoundError(const PandaString &msg);

void ThrowTypedErrorDyn(const std::string &msg);

void ThrowReferenceErrorDyn(const std::string &msg);

void ThrowIllegalMonitorStateException(const PandaString &msg);

void ThrowCloneNotSupportedException();

}  // namespace panda

#endif  // PANDA_RUNTIME_EXCEPTIONS_H_
