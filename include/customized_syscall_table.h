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


DEF_SYSCALL(0xa0, get_device_info, long, 1, trusty_device_info_t *info)
#ifdef SPI_CONTROLLER
DEF_SYSCALL(0xa1, trusty_spi_init, void, 0)
DEF_SYSCALL(0xa2, trusty_spi_set_cs, void, 1, uint8_t flag)
DEF_SYSCALL(0xa3, trusty_spi_read, int, 2, uint8_t *rx_buff, uint32_t rx_bytes)
DEF_SYSCALL(0xa4, trusty_spi_write, int, 2, uint8_t *tx_buff, uint32_t tx_bytes)
DEF_SYSCALL(0xa5, trusty_spi_writeread, int, 4, uint8_t *tx_buff, uint32_t tx_bytes, uint8_t *rx_buff, uint32_t rx_bytes)
#endif
