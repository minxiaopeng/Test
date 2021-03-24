/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_km_goldie_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    KM Goldie application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */

#include "hy_public.h"
#include "hy_net.h"

int main(void)
{
    BSP_Init();
    APP_Init();

    /*[HYN001-55] make the littlefs init more later spiflash factory test by yanggguangfu at 20210126 begin*/
    printf("---> HYN001 power on\r\n");

    if (Factory_WaitingToEnter() == 0) {
        /*[HYN001-102] factory mode led status conflict  by dujianfei at 20210310 begin*/
        //APP_LEDInit();
        /*[HYN001-102] factory mode led status conflict  by dujianfei at 20210310 end*/
        /*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 begin*/
        spi_flash_littlefs_mount();
        /*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 end*/
        /*[HYN001-47] Add optimize report and save mode by xulinfeng at 20210111 begin*/
        initAllFileFifo();
        /*[HYN001-47] Add optimize report and save mode by xulinfeng at 20210111 end*/
        /*[HYN001-55] make the littlefs init more later spiflash factory test by yanggguangfu at 20210126 end*/
        Uart_Switch(GSM_UART);
        if (fota_is_need_upgrade()) {
            Net_Init(BOOT_MODE_OTA);
            fota_upgrade_process();
        }
        else
        {
            Net_Init(BOOT_MODE_NORMAL);
            /*[HYN001-69] Add apn set,gtdog,gtlgn,last time store by xulinfeng at 20210129 begin*/
            Logic_set_apn();
            /*[HYN001-69] Add apn set,gtdog,gtlgn,last time store by xulinfeng at 20210129 end*/
#ifdef HYN001
            GSM_BuadRateAdj();
            GSM_Flow_Ctrl();
#endif
            APP_LEDInit();
            /*[HYN001-34] add watchdog function by chongziqing at 20201222 begin*/
            //BSP_Wdtinit();
            /*[HYN001-34] add watchdog function by chongziqing at 20201222 end*/
            /*[HYN001-47] Add optimize report and save mode by xulinfeng at 20210111 begin*/
            Logic_SetNetCallBack();
            /*[HYN001-47] Add optimize report and save mode by xulinfeng at 20210111 end*/
            Start_Protocol();
            Logic_AT_GTPIN_Handle_check();//Each time the device powers on, it will detect whether the SIM card is locked with a PIN.If it is locked, the device will unlock the PIN automatically.		
            Logic_RESP_GTPNA_init();//  POWER ON EVENT REPORT

            while (1)
            {
		/*[HYN001-86] Added setting parameter and read parameter handling by zhoushaoqing at 20210225 begin*/
                if (setconfigFlag)
                {
                 parseConfigData();
                 setconfigFlag = 0;
                }
		/*[HYN001-86] Added setting parameter and read parameter handling by zhoushaoqing at 20210225 end*/
                LPM_Pro();
                Logic_pro();
                SENSOR_Pro();
                BLE_Pro();
                GPS_Pro();
                WIFI_Pro();
                ADC_Pro();
                /*[HYN001-2]  add elec-fence function by tmy at 20201116 start*/
                FencePro();
                /*[HYN001-2]  add elec-fence function by tmy at 20201116 end*/
                /*[HYN001-34] add watchdog function by chongziqing at 20201222 begin*/
                //wdt_feed_pro();
                /*[HYN001-34] add watchdog function by chongziqing at 20201222 end*/
                Net_TaskLooper();
                Reset_Module();   //if moden is error,reset moden and mcu.restart link
            }
        }
    }
    else
    {
        Factory_Runtime();
    }
}
