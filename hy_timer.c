/******************************************************************************
 * File Name   : hy_timer.c
 *
 * Author      : ganxiao
 *
 * Version     : 1.0
 *
 * Date        : 2020-10-22
 *
 * DESCRIPTION : -
 *
 * --------------------
 * Copyright 2009-2020 Hymost Co.,Ltd.
 *
 ******************************************************************************/
#include"hy_public.h"

void TI_SecCnt(void)
{
    static uint16_t s_MillCnt = 0;

    s_MillCnt++;
	Event_Ten_MiliSencond_Time_Occer();
    if (s_MillCnt >= 100)
    {
        s_MillCnt = 0;
        /*add GTVGN,GTVGF function by tanghe at 20210302 begin*/
        montion_detect_interval = true;
        /*add GTVGN,GTVGF function by tanghe at 20210302 end*/

        rtc_timeTamp++;
        /* long stop time count zouzhongkai at 20210111 bigin */
		Event_Secomd_Time_Occer();
		/* long stop time count zouzhongkai at 20210111 end */
        CNT_SUB(AppStu.PrtInfoPeriod);
        g_wPowerCnt++;

        if (m_led_initialized)
        {
            bsp_board_led_invert(BSP_BOARD_LED_0);
        }
        
        /*[HYN001-34] add watchdog function by chongziqing at 20201222 begin*/
        wdt_time_pro();
        /*[HYN001-34] add watchdog function by chongziqing at 20201222 end*/
        
        /*if (GsmStu.ModuleOpnCnt > 1)
        {
            GsmStu.ModuleOpnCnt--;
        }
        if (GsmStu.ResCnt > 1)
        {
            GsmStu.ResCnt--;
        }
        if ((GsmStu.ResCnt == 1) &&
                (((GsmStu.creg != REG_LOC_NET) && (GsmStu.creg != REG_ROAM_NET))
                 || ((GsmStu.cgreg != REG_LOC_NET) && (GsmStu.cgreg != REG_ROAM_NET))))
        {
            GsmStu.ResCnt = 0;
            GsmStu.ModResMsg = true;
        }

        GsmStu.LngTime++;
        if (GsmStu.qInternalTime < 100)
        {
            GsmStu.qInternalTime++;
        }*/
        /*[HYN001-2]  add elec-fence function by tmy at 20201116 start*/
        CNT_SUB(CirFenceVar.CheckInterval);
        CNT_SUB(PolFenceVar.CheckInterval);
        /*[HYN001-2]  add elec-fence function by tmy at 20201116 end*/
        /*[HYN001-10] add spi falsh code by yangguangfu at 20201126 begin*/
        CNT_SUB(flash_wait_time);
        /*[HYN001-10] add spi falsh code by yangguangfu at 20201126 end*/
        CNT_SUB(led_on_hold_time);
        //APP_LedAllWork();
        /*[HYN001-27] add report logic by wangdai at 20201216 begin*/
        Logic_RESP_timer_Cnt();
        /*[HYN001-27] add report logic by wangdai at 20201216  end*/
        CNT_SUB(sensor_check_interval);

        /*Ignition time count, add by tanghe at 20210207 begin*/
        resp_ids_timer_count();
        /*Ignition time count, add by tanghe at 20210207 begin*/
    }
}

void TI_TenMilliCnt(void)
{
    CNT_SUB(g_sys_time);
    //CNT_SUB(GsmStu.AtNoDatWtTim);
    //CNT_SUB(GsmStu.RecTim);
    /*[HYN001-10] add spi falsh code by yangguangfu at 20201126 begin*/
    CNT_SUB(flash_write_read_time);
    /*[HYN001-10] add spi falsh code by yangguangfu at 20201126 end*/
	/*[HYN001-102] factory mode led status conflict  by dujianfei at 20210310 begin*/
	/*[HYN001-61] Add led state for gps and gsm scene by dujianfei at 20210122 begin*/
	//CNT_SUB(AppStu.GpsLedOn);
	//CNT_SUB(AppStu.GpsLedOff);
	//CNT_SUB(AppStu.GsmLedOff);
	//APP_LedFlash();
	/*[HYN001-61] Add led state for gps and gsm scene by dujianfei at 20210122 end*/
	/*[HYN001-102] factory mode led status conflict  by dujianfei at 20210310 end*/
    /*[HYN001-9] Add led api by dujianfei at 20201125 start*/
    if (!red_mutex)
        CNT_SUB(led_on);
    else if (red_mutex)
        CNT_SUB(red_green_led_off);
    if (!green_mutex)
        CNT_SUB(led_on);
    else if (green_mutex)
        CNT_SUB(green_led_off);
    //green_led_flicker();
    //red_led_flicker();
    /*[HYN001-9] Add led api by dujianfei at 20201125 end*/
    /*[HYN001-30] add optimize gps and time func by xulinfeng at 20201219 begin*/
    CNT_SUB(GpsCon.pollIntvl);
    /*[HYN001-30] add optimize gps and time func by xulinfeng at 20201219 end*/
    /*[HYN001-13] add gps func by xulinfeng at 20201127 end*/

    /*[HYN001-52] Add HBM function by chongziqing at 20210108 begin*/
    sensor_interval = 1;
    /*[HYN001-52] Add HBM function by chongziqing at 20210108 end*/
}
 /*[HYN001-88]optimize electric fence code wangdai 20210304 end*/
 #if 0
/*[HYN001-2]  add elec-fence function by tmy at 20201116 start*/
void TI_HundredMilliCnt(void)
{
    static u16 s_MillCntb = 0;

    if (s_MillCntb >= 10)
    {
        s_MillCntb = 0;
        /*[HYN001-14] Repair the problem of polygonal electronic fence detection error  by zhoushaoqing at 20201128 begin*/
        CNT_SUB(CirFenceVar.DelayCheckTime);
        CNT_SUB(PolFenceVar.DelayCheckTime);
        /*[HYN001-14] Repair the problem of polygonal electronic fence detection error  by zhoushaoqing at 20201128 end*/
    }
}
/*[HYN001-2]  add elec-fence function by tmy at 20201116 end*/
#endif
 /*[HYN001-88]optimize electric fence code wangdai 20210304 end*/
void AppTI_SecCnt(void)
{
    if (m_led_initialized)
    {
        bsp_board_led_invert(BSP_BOARD_LED_1);
    }
}

void timer_count_event_handler(nrf_timer_event_t event_type, void *p_context)
{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
            TI_TenMilliCnt();
             /*[HYN001-88]optimize electric fence code wangdai 20210304 begin*/
            /*[HYN001-2]  add elec-fence function by tmy at 20201116 start*/
            //TI_HundredMilliCnt();
            /*[HYN001-2]  add elec-fence function by tmy at 20201116 end*/
             /*[HYN001-88]optimize electric fence code wangdai 20210304 end*/
            TI_SecCnt();
            break;

        default:
            break;
    }
}

void hy_timer_handle(void *p_context)
{
    UNUSED_PARAMETER(p_context);

    AppTI_SecCnt();
}
/*[HYN001-19] Add TEMP&OIL sensor data report by yangguangfu at 20201203 begin*/
void hy_sensor_timer_handle(void *p_context)
{
    UNUSED_PARAMETER(p_context);
    update_sensor_data();
}
/*[HYN001-19] Add TEMP&OIL sensor data report by yangguangfu at 20201203 end*/

