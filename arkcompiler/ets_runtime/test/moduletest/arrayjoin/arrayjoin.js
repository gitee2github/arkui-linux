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
 * @tc.name:arrayjoin
 * @tc.desc:test Array.join
 * @tc.type: FUNC
 * @tc.require: issueI5NO8G
 */
var a = new Array(1).join("  ");
print(a.length);
var str1 = JSON.stringify(Array(3).join("0"));
print(str1);
var str2 = JSON.stringify(new Array(3).join("0"));
print(str2);
const arr = []
arr.length = 3
var str3 = JSON.stringify(arr.join("0"));
print(str3)