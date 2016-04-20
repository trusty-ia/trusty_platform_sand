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
#include <stdarg.h>
#include <reg.h>
#include <stdio.h>
#include <kernel/thread.h>
#include <arch/x86.h>
#include <lib/cbuf.h>
#include <platform/interrupts.h>
#include <platform/debug.h>
#include <string.h>
#include <platform/lkguest.h>

void tc_flush(void)
{
    lkguest_teewrite(&teecons_buff);

    memset(teecons_buff.msg, 0, TEE_DBG_BUFFERSIZE);
    teecons_buff.len = 0;
}

void tee_putc(int c)
{
    struct conbuff *p = &teecons_buff;

    p->msg[p->len++] = c;
    if (c == '\r' || c == '\n' || p->len > TEE_DBG_BUFFERSIZE - 2) {
        p->msg[p->len] = '\0';
        tc_flush();
    }

}

void platform_dputc(char c)
{
    tee_putc((int)c);
}

int platform_dgetc(char *c, bool wait)
{
    return 0;
}

