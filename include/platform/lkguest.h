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
#ifndef __PLATFORM_LKGUEST_H
#define __PLATFORM_LKGUEST_H

#include <debug.h>

#define TEE_DBG_BUFFERSIZE             4096

typedef enum {
    TEE_DBG_BUFF_VALID,
    TEE_DBG_BUFF_INVALID
} tee_dbg_buf_status;

struct conbuff {
    unsigned int cmd;
    unsigned int status;
    char msg[TEE_DBG_BUFFERSIZE];
    int len;    /* len > 0 means pending message */
};

void lkguest_teewrite(struct conbuff* buf);
void disable_tee_buffer(print_callback_t *cb);

#endif
