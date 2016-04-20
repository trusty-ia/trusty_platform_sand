/*******************************************************************************
 * Copyright (c) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/
#include <sys/types.h>
#include <platform/lkguest.h>

#define TRUSTY_VMCALL_PUTS 0x74727502 // "tru" is 0x747275
void rm_write(const char *buf)
{
    __asm__ __volatile__("vmcall" :: "a" (TRUSTY_VMCALL_PUTS), "d" (buf));
}

void lkguest_teewrite(struct conbuff* buf)
{
    if (buf->len > 0) {
        rm_write(buf->msg);
        buf->len = 0;
    }
}
