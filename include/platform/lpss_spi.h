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
#ifndef __LPSS_SPI_H__
#define __LPSS_SPI_H__

#define BIT(nr)         (1UL << (nr))

#define SPI_CS_CONTROL_SW_MODE (1<<0)
#define SPI_CS_CONTROL_CS_HIGH (1<<1)
#define SPI_CS0_POL_HI (1<<12)
#define SPI_CS1_POL_HI (1<<13)

#define SPI_CR0           (0x00)  /* control register0 */
#define SPI_CR1           (0x04)  /* control register1 */
#define SPI_SR            (0x08)  /* status register */
#define SPI_DR            (0x10)  /* data register */

#define SPI_CLK            (0x200) /* clock register */
#define SPI_RESET          (0x204) /* reset register */
#define SPI_CC             (0x224) /* chip select */

#define SPI_CR0_SSE   (1 << 7)    /* bus enable register */
#define SPI_CR0_EDSS  (1 << 20)   /* extended data size select */
#define SPI_CR0_DataSize(x)  ((x) - 1)    /* data size select */

#define SPI_SR_TNF    (1 << 2)    /* tx FIFO not full */
#define SPI_SR_RNE    (1 << 3)    /* rx FIFO not empty */
#define SPI_SR_BSY    (1 << 4)    /* bus busy */

uint32_t lpss_spi_read(uint32_t reg);
void lpss_spi_write(uint32_t reg, uint32_t val);
void spi_mmu_init(void);

#endif
