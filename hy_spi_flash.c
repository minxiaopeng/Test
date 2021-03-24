/******************************************************************************
 * File Name   : hy_spi_flash.c
 *
 * Author      : lizhiye
 *
 * Version     : 1.0
 *
 * Date        : 2019-09-18
 *
 * DESCRIPTION : -
 *
 * --------------------
 * Copyright 2009-2019 Hymost Co.,Ltd.
 *
 ******************************************************************************/

#include "hy_public.h"

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(1);  /**< SPI instance. */
static volatile bool spi_xfer_done = false;  /**< Flag used to indicate that SPI instance completed the transfer. */
volatile uint8_t flash_wait_time = 0;
volatile uint8_t flash_write_read_time = 0;

void spi_flash_event_handler(nrf_drv_spi_evt_t const *p_event, void *p_context)
{
    spi_xfer_done = true;
}

uint32_t spi_flash_write_reg(uint8_t addr)
{
    uint32_t ret;
    uint8_t value = 0;
    spi_xfer_done = false;
    flash_write_read_time = 4;
    ret = nrf_drv_spi_transfer(&spi, &addr, 1, &value, 0);
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("write reg fail ret=%d", ret);
        return ret;
    }
    while ((spi_xfer_done == false) && (flash_write_read_time));
    /*[HM003-51] add littlefs filesystem for spi flash by lizhiye at 20191014 begin*/
    if (flash_write_read_time == 0)
    {
        return NRFX_ERROR_TIMEOUT;
    }
    else
    {
        return NRFX_SUCCESS;
    }
    /*[HM003-51] add littlefs filesystem for spi flash by lizhiye at 20191014 end*/
}

uint32_t spi_flash_read_reg(uint8_t *value)
{
    ret_code_t ret = NRFX_SUCCESS;
    uint8_t addr = 0xFF;
    spi_xfer_done = false;
/*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 begin*/
    flash_write_read_time = 8;
/*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 end*/
    ret = nrf_drv_spi_transfer(&spi, &addr, 0, value, 1);
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("read reg fail ret=%d", ret);
        return ret;
    }
    while ((spi_xfer_done == false) && (flash_write_read_time));
    /*[HM003-51] add littlefs filesystem for spi flash by lizhiye at 20191014 begin*/
    if (flash_write_read_time == 0)
    {
        return NRFX_ERROR_TIMEOUT;
    }
    else
    {
        return NRFX_SUCCESS;
    }
    /*[HM003-51] add littlefs filesystem for spi flash by lizhiye at 20191014 end*/
}

uint32_t spi_flash_write_enable(void)
{
    uint32_t ret = NRFX_SUCCESS;
    nrf_gpio_pin_clear(SPI_SS_PIN);
    ret = spi_flash_write_reg(FLASH_WRITE_ENABLE);
    nrf_gpio_pin_set(SPI_SS_PIN);
    return ret;
}

uint32_t spi_flash_wait_busy(void)
{
    uint32_t ret = NRFX_SUCCESS;
    uint8_t status = 0;

    nrf_gpio_pin_clear(SPI_SS_PIN);
    ret = spi_flash_write_reg(FLASH_READ_STATUS_REG);
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("write reg status reg fail ret=%d", ret);
        return ret;
    }

    do
    {
        ret = spi_flash_read_reg(&status);
        if (ret != NRFX_SUCCESS)
        {
            NRF_LOG_INFO("read reg fail ret=%d", ret);
            return ret;
        }
        if (flash_wait_time == 0)
        {
            ret = NRFX_ERROR_TIMEOUT;
            break;
        }
    }
    while ((FLASH_WRITE_BUSY_BIT & status) == 1);
    nrf_gpio_pin_set(SPI_SS_PIN);
    return ret;
}


uint32_t spi_flash_erase_full_chip(void)
{
    uint32_t ret = NRFX_SUCCESS;
/*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 begin*/
    flash_wait_time = 40;   // wait 25s
/*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 end*/

    ret = spi_flash_write_enable();
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("spi_write_enable fail:%d", ret);
        return ret;
    }
    nrf_gpio_pin_clear(SPI_SS_PIN);
    ret = spi_flash_write_reg(FLASH_CHIP_ERASE);
    nrf_gpio_pin_set(SPI_SS_PIN);

    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("erase chip fail:%d", ret);
        return ret;
    }
    ret = spi_flash_wait_busy();
    return ret;
}

uint32_t spi_flash_erase_4kSector(uint32_t addr)
{
    uint32_t ret = NRFX_SUCCESS;
/*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 begin*/
    flash_wait_time = 4;   // wait 2s
/*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 begin*/
    ret = spi_flash_write_enable();
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("spi_write_enable fail:%d", ret);
        return ret;
    }
    nrf_gpio_pin_clear(SPI_SS_PIN);
    ret = spi_flash_write_reg(FLASH_SECTOR_ERASE);
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("erase chip fail:%d", ret);
        return ret;
    }

    spi_flash_write_reg((addr & 0XFF0000) >> 16);
    spi_flash_write_reg((addr & 0XFF00) >> 8);
    spi_flash_write_reg(addr & 0XFF);
    nrf_gpio_pin_set(SPI_SS_PIN);

    ret = spi_flash_wait_busy();
    return ret;
}

/*FLASH页写入指令：使用页写入指令最多可以一次向FLASH传输256个字节的数据*/
uint32_t spi_flash_write_data(uint32_t addr, uint8_t *data, uint16_t size)
{
    uint32_t ret = NRFX_SUCCESS;
    if (size > 256)
    {
        NRF_LOG_INFO("write too large!");
        return NRFX_ERROR_INVALID_LENGTH;
    }
/*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 begin*/
    flash_wait_time = 4;   // wait 2s
/*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 end*/
    ret = spi_flash_write_enable();
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("enable write fail ret=%d", ret);
        return ret;
    }

    nrf_gpio_pin_clear(SPI_SS_PIN);
    ret = spi_flash_write_reg(FLASH_PAGE_PROGRAM);
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("page program fail ret=%d", ret);
        return ret;
    }

    spi_flash_write_reg((addr & 0XFF0000) >> 16);
    spi_flash_write_reg((addr & 0XFF00) >> 8);
    spi_flash_write_reg(addr & 0XFF);
    while (size--)
    {
        ret = spi_flash_write_reg(*data);
        if (ret != NRFX_SUCCESS)
        {
            NRF_LOG_INFO("ret=%d", ret);
            break;
        }
        data++;
    }
    nrf_gpio_pin_set(SPI_SS_PIN);
    ret = spi_flash_wait_busy();
    return ret;
}

uint32_t spi_flash_read_data(uint32_t addr, uint8_t *data, uint16_t size)
{
    uint32_t ret = NRFX_SUCCESS;
    if (size > 256)
    {
        NRF_LOG_INFO("read data too large!");
        return NRFX_ERROR_INVALID_LENGTH;
    }
    nrf_gpio_pin_clear(SPI_SS_PIN);
    ret = spi_flash_write_reg(FLASH_READ_DATA);
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("write read data reg fail ret=%d", ret);
        return ret;
    }

    ret = spi_flash_write_reg((addr & 0XFF0000) >> 16); //发送读地址的高位
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("write read data reg fail ret1=%d", ret);
    }
    ret = spi_flash_write_reg((addr & 0XFF00) >> 8);
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("write read data reg fail ret2=%d", ret);
    }
    ret = spi_flash_write_reg(addr & 0XFF);
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("write read data reg fail ret3=%d", ret);
    }
    while (size--)
    {
        ret = spi_flash_read_reg(data);
        if (ret != NRFX_SUCCESS)
        {
            ret = NRFX_ERROR_INTERNAL;
            break;
        }
        data++;
    }
    nrf_gpio_pin_set(SPI_SS_PIN);
    return ret;
}

/*[HM003-158] enhance report function part 2 by ganxiao at 20200229 start*/
uint32_t spi_flash_reset(void)
{
    uint32_t ret = NRFX_SUCCESS;

    nrf_gpio_pin_clear(SPI_SS_PIN);  //cs pin is 0: select the ic
    ret = spi_flash_write_reg(FLASH_RESET_ENABLE);
    if (ret != NRFX_SUCCESS)
    {
        return NRFX_ERROR_INTERNAL;
    }
    nrf_gpio_pin_set(SPI_SS_PIN);  //cs pin is 1: not select the ic

    nrf_delay_ms(5);

    nrf_gpio_pin_clear(SPI_SS_PIN);  //cs pin is 0: select the ic
    ret = spi_flash_write_reg(FLASH_RESET);
    if (ret != NRFX_SUCCESS)
    {
        return NRFX_ERROR_INTERNAL;
    }
    nrf_gpio_pin_set(SPI_SS_PIN);  //cs pin is 1: not select the ic

    nrf_delay_ms(NRFX_SUCCESS);

    return 0;
}
/*[HM003-158] enhance report function part 2 by ganxiao at 20200229 end*/

uint32_t spi_flash_read_id(void)
{
    uint32_t ret = NRFX_SUCCESS;
    uint32_t temp = 0;
    uint8_t temp0 = 0, temp1 = 0, temp2 = 0;

    nrf_gpio_pin_clear(SPI_SS_PIN);  //cs pin is 0: select the ic
    ret = spi_flash_write_reg(FLASH_DEVICE_ID);
    if (ret == NRFX_SUCCESS)
    {
        spi_flash_read_reg(&temp0);
        spi_flash_read_reg(&temp1);
        spi_flash_read_reg(&temp2);
    }
    nrf_gpio_pin_set(SPI_SS_PIN);  //cs pin is 1: not select the ic

    temp = (temp0 << 16) | (temp1 << 8) | temp2;
    return temp;
}

uint32_t spi_flash_init(void)
{
    uint32_t ret = NRFX_SUCCESS;
    uint32_t deviceId = 0;
    uint8_t try_num = 3;

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    //spi_config.ss_pin   = SPI_SS_PIN;  //把SS引脚禁用
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
    spi_config.frequency    = NRF_DRV_SPI_FREQ_8M;

    nrf_gpio_cfg_output(SPI_WIRTE_PROT_PIN);
    nrf_gpio_pin_clear(SPI_WIRTE_PROT_PIN);

    nrf_gpio_cfg_output(SPI_HOLD_PIN);
    nrf_gpio_pin_set(SPI_HOLD_PIN);

    nrf_gpio_cfg_output(SPI_SS_PIN);
    /*[HM003-51] add littlefs filesystem for spi flash by lizhiye at 20191014 begin*/
    nrf_gpio_pin_set(SPI_SS_PIN);
    nrf_delay_ms(10);
    /*[HM003-51] add littlefs filesystem for spi flash by lizhiye at 20191014 end*/

    ret = nrf_drv_spi_init(&spi, &spi_config, spi_flash_event_handler, NULL);
    if (ret != NRFX_SUCCESS)
    {
        NRF_LOG_INFO("spi init fail ret=%d", ret);
        return ret;
    }

    /*[HM003-158] enhance report function part 2 by ganxiao at 20200229 start*/
    spi_flash_reset();
    /*[HM003-158] enhance report function part 2 by ganxiao at 20200229 end*/

    /*[HM003-51] add littlefs filesystem for spi flash by lizhiye at 20191014 begin*/
    while (try_num-- > 0)
    {
        nrf_delay_ms(150);
        deviceId = spi_flash_read_id();
        if (deviceId != FLASH_ID)
        {
            NRF_LOG_INFO("spi flash fail get device id=0x%x", deviceId);
            /*[HM003-158] enhance report function part 2 by ganxiao at 20200229 start*/
            spi_flash_reset();
            /*[HM003-158] enhance report function part 2 by ganxiao at 20200229 end*/
            continue;
        }
        else
        {
            NRF_LOG_INFO("spi flash success get device id=0x%x", deviceId);
            return NRFX_SUCCESS;
        }
    }
    return NRFX_ERROR_INTERNAL;
    /*[HM003-51] add littlefs filesystem for spi flash by lizhiye at 20191014 end*/
}

/*[HM003-51] add littlefs filesystem for spi flash by lizhiye at 20191014 begin*/
void spi_flash_enter_lpm_mode(void)
{
    nrf_gpio_pin_clear(SPI_SS_PIN);
    spi_flash_write_reg(FLASH_ENTER_DEEP_POWER_DOWN_MODE);
    nrf_gpio_pin_set(SPI_SS_PIN);
}

void spi_flash_exit_lpm_mode(void)
{
    nrf_gpio_pin_clear(SPI_SS_PIN);
    spi_flash_write_reg(FLASH_EXIT_DEEP_POWER_DOWN_MODE);
    nrf_gpio_pin_set(SPI_SS_PIN);
}
/*[HM003-51] add littlefs filesystem for spi flash by lizhiye at 20191014 begin*/

