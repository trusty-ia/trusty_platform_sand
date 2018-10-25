/*******************************************************************************
 * Copyright (c) 2016 Intel Corporation
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
#ifndef __SAND_SPI_H__
#define __SAND_SPI_H__

#include <stdint.h>

#define TIMEOUT 400000

struct spi_tran_data{
    void *tx;
    void *tx_end;
    void *rx;
    void *rx_end;
    uint32_t rx_length;
    uint32_t tx_length;
    uint32_t bits_per_word;
    int (*write)(struct spi_tran_data *drv_data);
    int (*read)(struct spi_tran_data *drv_data);
};

void spi_init(void);
void spi_set_cs(uint8_t flag);
int spi_read(uint8_t *rx_buff, uint32_t rx_bytes);
int spi_write(uint8_t *tx_buff, uint32_t tx_bytes);
int spi_writeread(uint8_t *tx_buff, uint32_t tx_bytes, uint8_t *rx_buff, uint32_t rx_bytes);

#endif
