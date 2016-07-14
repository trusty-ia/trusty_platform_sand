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
#include <string.h>

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

extern struct conbuff teecons_buff;

void disable_tee_buffer(print_callback_t *cb)
{
	struct conbuff *tee_buf = &teecons_buff;

	if (tee_buf->len != 0) {
		/* print to any registered loggers */
		if (cb->print)
			cb->print(cb, tee_buf->msg, tee_buf->len);

		/* already regiester print_cb, no need to print log to buffer
		* now, so set the status as INVALID */
		tee_buf->status = TEE_DBG_BUFF_INVALID;
		tee_buf->len = 0;
		memset(tee_buf->msg, 0, TEE_DBG_BUFFERSIZE);
	}
}
