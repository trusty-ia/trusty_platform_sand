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
#include <err.h>
#include <debug.h>
#include <string.h>
#include <platform/lpss_spi.h>
#include <platform/spi.h>
#include <platform/sand_defs.h>

static struct spi_tran_data spi_drv_data;

static inline struct spi_tran_data *dev_get_drvdata(void)
{
    return &spi_drv_data;
}

static inline int check_tx_fifo(void)
{

    int timeout = TIMEOUT;

    while (!(lpss_spi_read(SPI_SR) & SPI_SR_TNF)) {
        if (!--timeout)
            return ERR_TIMED_OUT ;
        __asm__ __volatile__("pause":::"memory");
    }
    return NO_ERROR;
}

static inline int check_rx_fifo(void)
{

    int timeout = TIMEOUT;

    while (!(lpss_spi_read(SPI_SR) & SPI_SR_RNE)) {
        if (!--timeout)
            return ERR_TIMED_OUT ;
        __asm__ __volatile__("pause":::"memory");
    }
    return NO_ERROR;
}


static int null_writer(struct spi_tran_data *drv_data)
{
    int ret;

    ret = check_tx_fifo();
    if (ret != NO_ERROR)
        return ret;

    lpss_spi_write(SPI_DR, 0);
    return ret;
}

static int null_reader(struct spi_tran_data *drv_data)
{
    int ret;

    ret = check_rx_fifo();
    if (ret != NO_ERROR)
        return ret;

    lpss_spi_read(SPI_DR);
    return ret;
}

static int u8_writer(struct spi_tran_data *drv_data)
{
    int ret;

    ret = check_tx_fifo();
    if (ret != NO_ERROR)
        return ret;

    lpss_spi_write(SPI_DR, *(uint8_t *)(drv_data->tx));
    ++drv_data->tx;
    --drv_data->tx_length;
    return ret;
}

static int u8_reader(struct spi_tran_data *drv_data)
{
    int ret;

    ret = check_rx_fifo();
    if (ret != NO_ERROR)
        return ret;

    *(uint8_t *)(drv_data->rx) = lpss_spi_read(SPI_DR);
    ++drv_data->rx;
    --drv_data->rx_length;
    return ret;
}

static int u16_writer(struct spi_tran_data *drv_data)
{
    int ret;

    ret = check_tx_fifo();
    if (ret != NO_ERROR)
        return ret;

    lpss_spi_write(SPI_DR, *(uint16_t *)(drv_data->tx));
    drv_data->tx += 2;
    drv_data->tx_length -= 2;
    return ret;
}

static int u16_reader(struct spi_tran_data *drv_data)
{
    int ret;

    ret = check_rx_fifo();
    if (ret != NO_ERROR)
        return ret;

    *(uint16_t *)(drv_data->rx) = lpss_spi_read(SPI_DR);
    drv_data->rx += 2;
    drv_data->rx_length -= 2;
    return ret;
}

static int u32_writer(struct spi_tran_data *drv_data)
{
    int ret;

    ret = check_tx_fifo();
    if (ret != NO_ERROR)
        return ret;

    lpss_spi_write(SPI_DR, *(uint32_t *)(drv_data->tx));
    drv_data->tx += 4;
    drv_data->tx_length -= 4;
    return ret;
}

static int u32_reader(struct spi_tran_data *drv_data)
{
    int ret;

    ret = check_rx_fifo();
    if (ret != NO_ERROR)
        return ret;

    *(uint32_t *)(drv_data->rx) = lpss_spi_read(SPI_DR);
    drv_data->rx += 4;
    drv_data->rx_length -= 4;
    return ret;
}

void spi_set_cs(uint8_t flag)
{
    uint32_t val;

    val = lpss_spi_read(SPI_CC);
    if(flag)
        val &= ~SPI_CS_CONTROL_CS_HIGH;
    else
        val |= SPI_CS_CONTROL_CS_HIGH;
    lpss_spi_write(SPI_CC, val);
}

static int spi_check_busy(void)
{
    int timeout = TIMEOUT;

    do {
        __asm__ __volatile__("pause":::"memory");
    } while ((lpss_spi_read(SPI_SR) & SPI_SR_BSY) && --timeout);

    return timeout;
}

void spi_init(void)
{
    uint32_t sscr0, scc;
    struct spi_tran_data *spi_init_data = dev_get_drvdata();
    uint32_t bits = spi_init_data->bits_per_word = 8;

    lpss_spi_write(SPI_RESET, 0x7);
    lpss_spi_write(SPI_CLK, 0x1003);
    lpss_spi_write(SPI_CR0, 0);   //stop ssp
    sscr0 = 0x1300 | SPI_CR0_DataSize(bits > 16 ? bits - 16 : bits) | SPI_CR0_SSE
        | (bits > 16 ? SPI_CR0_EDSS : 0);
    lpss_spi_write(SPI_CR1, 0);
    scc = lpss_spi_read(SPI_CC);
    scc |= SPI_CS_CONTROL_SW_MODE;
    lpss_spi_write(SPI_CC, scc);
    lpss_spi_write(SPI_CR0, sscr0);   //enable ssp

}

int spi_read(uint8_t *rx_buff, uint32_t rx_bytes)
{
    int rc;
    struct spi_tran_data *spi_transfer = dev_get_drvdata();

    if((!rx_buff)||(rx_bytes < 1))
        return ERR_INVALID_ARGS;

    if(!spi_check_busy())
        return ERR_BUSY;

    spi_transfer->rx = (void *)rx_buff;
    spi_transfer->rx_end = spi_transfer->rx + rx_bytes;
    spi_transfer->rx_length = rx_bytes;
    if (spi_transfer->bits_per_word <= 8) {
        spi_transfer->read = u8_reader;
        spi_transfer->write = null_writer;
    } else if (spi_transfer->bits_per_word <= 16) {
        spi_transfer->read = u16_reader;
        spi_transfer->write = null_writer;
    } else if (spi_transfer->bits_per_word <= 32) {
        spi_transfer->read = u32_reader;
        spi_transfer->write = null_writer;
    }

    spi_set_cs(1);

    while(spi_transfer->rx != spi_transfer->rx_end) {
        rc = spi_transfer->write(spi_transfer);
        if(rc < 0) {
            dprintf(CRITICAL, "spi write failed with error %d in spi_read\n", rc);
            goto err;
        }

        rc = spi_transfer->read(spi_transfer);
        if(rc < 0) {
            dprintf(CRITICAL, "spi read failed with error %d in spi_read\n", rc);
            goto err;
        }
    }

    spi_set_cs(0);
    return NO_ERROR;

err:
    spi_set_cs(0);
    return rc;
}

int spi_write(uint8_t *tx_buff, uint32_t tx_bytes)
{
    int rc;
    struct spi_tran_data *spi_transfer = dev_get_drvdata();

    if((!tx_buff)||(tx_bytes < 1))
        return ERR_INVALID_ARGS;

    if(!spi_check_busy())
        return ERR_BUSY;

    spi_transfer->tx = (void *)tx_buff;
    spi_transfer->tx_end = spi_transfer->tx + tx_bytes;
    spi_transfer->tx_length = tx_bytes;

    if (spi_transfer->bits_per_word <= 8) {
        spi_transfer->read = null_reader;
        spi_transfer->write = u8_writer;
    } else if (spi_transfer->bits_per_word <= 16) {
        spi_transfer->read = null_reader;
        spi_transfer->write = u16_writer;
    } else if (spi_transfer->bits_per_word <= 32) {
        spi_transfer->read = null_reader;
        spi_transfer->write = u32_writer;
    }

    spi_set_cs(1);

    while(spi_transfer->tx != spi_transfer->tx_end) {
        rc = spi_transfer->write(spi_transfer);
        if(rc < 0) {
            dprintf(CRITICAL, "spi write failed with error %d in spi_write\n", rc);
            goto err;
        }

        rc = spi_transfer->read(spi_transfer);
        if(rc < 0) {
            dprintf(CRITICAL, "spi read failed with error %d in spi_write\n", rc);
            goto err;
        }
    }

    spi_set_cs(0);
    return NO_ERROR;

err:
    spi_set_cs(0);
    return rc;
}

int spi_writeread(uint8_t *tx_buff, uint32_t tx_bytes, uint8_t *rx_buff, uint32_t rx_bytes)
{
    int rc;
    uint32_t total_length = tx_bytes + rx_bytes;
    struct spi_tran_data *spi_transfer = dev_get_drvdata();

    if((!rx_buff)||(rx_bytes < 1)||(!tx_buff)||(tx_bytes < 1))
        return ERR_INVALID_ARGS;

    if(!spi_check_busy())
        return ERR_BUSY;

    spi_transfer->rx = (void *)rx_buff;
    spi_transfer->rx_end = spi_transfer->rx + total_length;
    spi_transfer->rx_length = total_length;
    spi_transfer->tx = (void *)tx_buff;
    spi_transfer->tx_end = spi_transfer->tx + total_length;
    spi_transfer->tx_length = total_length;

    if (spi_transfer->bits_per_word <= 8) {
        spi_transfer->read = u8_reader;
        spi_transfer->write = u8_writer;
    } else if (spi_transfer->bits_per_word <= 16) {
        spi_transfer->read = u16_reader;
        spi_transfer->write = u16_writer;
    } else if (spi_transfer->bits_per_word <= 32) {
        spi_transfer->read = u32_reader;
        spi_transfer->write = u32_writer;
    }

    spi_set_cs(1);

    while(total_length) {
        rc = spi_transfer->write(spi_transfer);
        if(rc < 0) {
            dprintf(CRITICAL, "spi write failed with error %d in spi_writeread\n", rc);
            goto err;
        }

        rc = spi_transfer->read(spi_transfer);
        if(rc < 0) {
            dprintf(CRITICAL, "spi read failed with error %d in spi_writeread\n", rc);
            goto err;
        }

        total_length--;
    }

    spi_set_cs(0);
    return NO_ERROR;

err:
    spi_set_cs(0);
    return rc;
}

int spi_destroy(void)
{
    //TBD
    return NO_ERROR;
}

int spi_select_low_freq(uint32_t low_freq)
{
    //TBD
    return NO_ERROR;
}

int spi_select_high_freq(uint32_t high_freq)
{
    //TBD
    return NO_ERROR;
}
