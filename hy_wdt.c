/******************************************************************************
 * File Name   : hy_wdt.c
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
#include "hy_public.h"
volatile u8 wdt_wait_time = 0;
volatile u8 wdt_feed_flag = 0;

/*[HYN001-34] add watchdog function by chongziqing at 20201222 begin*/
void wdt_time_pro(void)
{
    wdt_wait_time ++ ;
    if(wdt_wait_time >= 15)
    {
        wdt_wait_time = 0;
        wdt_feed_flag = 1;
    }
}

void wdt_feed_pro(void)
{
    if(wdt_feed_flag)
    {
        wdt_feed_flag = 0;
        BSP_FeedWdt();
    }
}
/*[HYN001-34] add watchdog function by chongziqing at 20201222 end*/
