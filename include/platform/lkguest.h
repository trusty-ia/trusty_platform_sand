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

#define TEE_TO_RM                      0x20000
#define TEE_DBG_BUFFERSIZE             80
#define TEEWR_RM_DBG_MESG              (TEE_TO_RM)
#define TEE_MASK32                     0xFFFFFFFF
#define TEE_MASK16                     0xFFFF

struct conbuff {
    unsigned int cmd;
    unsigned int status;
    char msg[TEE_DBG_BUFFERSIZE];
    int len;    /* len > 0 means pending message */
};

struct conbuff teecons_buff;

void lkguest_teewrite(struct conbuff* buf);

#endif
