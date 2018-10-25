/*
 * Copyright (c) 2018 Intel Corporation
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
 */

#define __NR_get_device_info				0xa0
#ifdef SPI_CONTROLLER
#define __NR_trusty_spi_init 				0xa1
#define __NR_trusty_spi_set_cs				0xa2
#define __NR_trusty_spi_read				0xa3
#define __NR_trusty_spi_write				0xa4
#define __NR_trusty_spi_writeread			0xa5
#endif

#ifndef ASSEMBLY

__BEGIN_CDECLS
#include "trusty_device_info.h"

long get_device_info (trusty_device_info_t *info);
#ifdef SPI_CONTROLLER
void trusty_spi_init(void);
void trusty_spi_set_cs(uint8_t flag);
int trusty_spi_read(uint8_t *rx_buff, uint32_t rx_bytes);
int trusty_spi_write(uint8_t *tx_buff, uint32_t tx_bytes);
int trusty_spi_writeread(uint8_t *tx_buff, uint32_t tx_bytes, uint8_t *rx_buff, uint32_t rx_bytes);
#endif

__END_CDECLS

#endif
