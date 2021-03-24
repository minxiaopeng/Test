/******************************************************************************
 * File Name   : hy_app.c
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
#include"hy_net.h"

APP_StructDef AppStu;
volatile uint16_t g_sys_time = 0;
volatile uint32_t g_wPowerCnt = 0;
///*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 begin*/
//volatile u32 g_spi_life_test_cycles = 0;
///*[HYN001-22] add littlefs filesystem for spi flash by yangguangfu at 20201207 begin*/

u16 Labs(s16 Par)
{
    s16 Val = Par;

    if (Val < 0)
    {
        Val = ~Val + 1;
    }

    return ((u16)Val);
}

u8 APP_StrToBcd(u8 *pStr, u8 *pOutBuf, u8 Len)
{
    u8 i = 0, ii = 0;

    for (i = 0; i < Len; i += 2)
    {
        pOutBuf[ii++] = CALC_TIME(pStr[i], pStr[i + 1]);
    }
    return (ii);
}

void delayms(u16 deyms)
{
    g_sys_time = deyms / 10;
    while (g_sys_time > 0)
    {
        ;
    }
}

void conver(const u8 *Text, u8 *str, u16 pLen)
{
    u16 i;
    for (i = 0; i < pLen; i++)
    {
        str[i] = Text[i];
    }
}

u8 chr(u8 x)
{
    if ((x >= '0') && (x <= '9'))
        return (x - '0');
    if ((x >= 'A') && (x <= 'F'))
        return (x - '7');
    if ((x >= 'a') && (x <= 'f'))
        return (x - 0x57);
    return (0);
}

u8 asc(u8 x)
{
    if (x <= 9)
        return (x + '0');
    if (x <= 15)
        return (x + '7');
    return ('0');
}

void Hexstr2Asc(u8 *Hexstr, u16 len, u8 *Ascstr)
{
    u16 j = 0;
    u16 k = 0;

    for (j = 0; j < len; j++)
    {
        Ascstr[k++] = asc((Hexstr[j] >> 4));
        Ascstr[k++] = asc((Hexstr[j] & 0x0f));
    }
}

u8 HexToAsc(u32 HexVar, u8 Len, u8 *pDest)
{
    u8 Idx = 0, AscLen = 0;
    u8 TmpValue = 0;

    for (Idx = Len; Idx > 0; Idx--)
    {
        TmpValue = HexVar >> 8 * (Idx - 1);
        pDest[AscLen++] = asc(TmpValue >> 4);
        pDest[AscLen++] = asc(TmpValue & 0x0f);
    }
    return (AscLen);
}

u16 instr(u8 *text, u16 iLen, const u8 *str, u8 sLen)
{
    u16 j;
    u8 i, f = 0;
    for (j = 0; j < iLen; j++)
    {
        if ((j + sLen) > iLen)
        {
            return (0);
        }
        f = 0;
        for (i = 0; i < sLen; i++)
        {
            if (text[j + i] != str[i])
            {
                f = 1;
            }
        }
        if (!f)
        {
            return (j + 1);
        }
    }
    return (0);
}

void mid(u8 *Text, u8 *Buff, u8 z)
{
    u8 i;
    u8 x;
    Buff[0] = 0;
    for (i = 0; i < z; i++)
    {
        x = Text[i];
        if (x > 0x20 && x < 0x7f)
        {
            Buff[0]++;
            Buff[Buff[0]] = x;
        }
    }
}

u32 AscToLong(u8 *str, u8 size, u8 e)
{
    u32 k, j = 1;
    u8 i, dh = 10;
    if ((size == 0) || (size > 9))
        return (0);
    if (e == 'H')
        dh = 0x10;
    if (size > 1)
    {
        for (i = 1; i < size; i++)
        {
            j = j * dh;
        }
        k = 0;
        for (i = 0; i < size; i++)
        {
            k = k + chr(*(str + i)) * j;
            j = j / dh;
        }
        return (k);
    }
    else
        return (chr(*str));
}

u8 LongToAsc(u32 lng, u8 *str, u8 e)
{
    //0x12345678
    //0xabcdefcd
    u8 k, r, i, j = 28;
    u32 xorLng = 0xffffffff;
    k = 0;
    for (i = 0; i < 8; i++)
    {
        r = (u8)(lng >> j);
        if ((r == 0) && (k == 0) && (e == 0)) //e=1 return 8 bytes, e=0 return variable bytes
        {
        }
        else
        {
            *(str + k) = asc(r);
            k++;
        }
        xorLng >>= 4;
        lng &= xorLng;
        j -= 4;
    }
    if (k == 0)
    {
        *str = '0';
        k = 1;
    }
    return (k);
}

u8 long2ascD(u32 x, u8 *Text)
{
    u8 i, a, j;
    u32 k;
    k = 1000000000;
    j = 0;
    for (i = 1; i <= 10; i++)
    {
        a = x / k;
        if ((j > 0) || (a != 0))
        {
            Text[j++] = a + '0';
        }
        x = x - a * k;
        k /= 10;
    }
    if (j == 0)
    {
        Text[j++] = '0';
    }
    Text[j] = 0;
    return (j);
}

u8 Hex(u8 x)
{
    //20-->32
    return (((x / 10) << 4) + (x % 10));
}

u8 APP_HexToDec(u8 HexData)
{
    return ((HexData >> 4) * 10 + (HexData & 0x0F));

}

u8 DecToBcd(u32 InData, u8 *pOutBuf)
{
    u8 TmpData = 0;
    u8 i = 0;
    u8 k = 0;
    u8 TmpBuf[5];

    for (i = 0; InData > 0; i++)
    {
        TmpData = InData % 100;
        TmpBuf[i] = Hex(TmpData);
        InData = InData / 100;
    }
    for (k = 0; i > 0; i--)
    {
        pOutBuf[k] = TmpBuf[i - 1];
        k++;
    }
    return (k);
}

u8 IsNumLen(u8 *Text)
{
    u8 i = 0;
    //1234 x
    while (i < 250)
    {
        if (Text[i] < '0' || Text[i] > '9')
            return (i);
        i++;
    }
    return (i);
}

u8 IsChrLen(u8 *Text)
{
    u8 i = 0;
    //1234 x HUIYE
    while (i < 250)
    {
        if (Text[i] >= 'A' && Text[i] <= 'Z')
            i++;
        else if (Text[i] >= 'a' && Text[i] <= 'z')
            i++;
        else if (Text[i] >= '0' && Text[i] <= '9')
            i++;
        else if (Text[i] == '*' || Text[i] <= ')')
            return (i);
        else if (Text[i] >= ' ' && Text[i] < '0')
            i++;
        else
            return (i);
    }
    return (i);
}

/*[HYN001-26] add improve gps func by xulinfeng at 20201216 begin*/
u32  hy_atoi(const u8 *str)
{
    u32 s = 0;

    while (*str == ' ')
    {
        str++;
    }

    while (*str >= '0' && *str <= '9')
    {
        s = s * 10 + *str - '0';
        str++;
    }
    return s;
}

double hy_atof(const char *str)
{
    double s = 0.0;
    double d = 10.0;

    bool falg = false;

    while (*str == ' ')
    {
        str++;
    }

    if (*str == '-')
    {
        falg = true;
        str++;
    }

    if (!(*str >= '0' && *str <= '9'))
        return s;

    while (*str >= '0' && *str <= '9' && *str != '.')
    {
        s = s * 10.0 + *str - '0';
        str++;
    }

    if (*str == '.')
        str++;

    while (*str >= '0' && *str <= '9')
    {
        s = s + (*str - '0') / d;
        d *= 10.0;
        str++;
    }
    return s * (falg ? -1.0 : 1.0);
}
/*[HYN001-26] add improve gps func by xulinfeng at 20201216 end*/

char *trim(char *str) {
    char *p = str + strlen(str) - 1;
    while (' ' == *p || '\t' == *p || '\n' == *p || '\r' == *p) {
        *p = '\0';
        p--;
    }
    return str;
}

/*[HYN001-32]modify parse data from server wangdai 20210208 begin*/
/*hex string --> dec integer*/
u32 htoi(char *str)
{
	char *p=str;
	u32 x = 0;

	if (NULL == p)
	{
		NRF_LOG_ERROR("htoi: parameter error");
		return 0;
	}

	if(*p == '0' && (*(p+1) == 'x' || *(p+1) == 'X'))
	{
		p = p+2;
	}

	for( ;*p != '\0';p++)
	{
		x = x*16+chr(*p);
	}
	return x;
}
/*[HYN001-32]modify parse data from server wangdai 20210208 end*/

u8 APP_CalVeri(u8 *pBuf, u16 len)
{
    u8  bcc = 0;

    while (len--)
    {
        bcc ^= *pBuf++;
    }

    return bcc;
}

void UserParaInit(void)
{
/*[HYN001-67]add some of at handle by wangdai 20210127 begin*/
	memset(&ACC_Sta, 0, sizeof(ACC_Sta));
	memset(&Flag_MASK, 0, sizeof(Flag_MASK));
/*[HYN001-67]add some of at handle by wangdai 20210127 end*/
    /*[HYN001-69] Add apn set,gtdog,gtlgn,last time store by xulinfeng at 20210129 begin*/
    Logic_user_param_init();
    /*[HYN001-69] Add apn set,gtdog,gtlgn,last time store by xulinfeng at 20210129 end*/
}

/*[HYN001-61] Add led state for gps and gsm scene by dujianfei at 20210122 begin*/
void APP_GpsLed(void)
{
	static volatile bool flag = 0;
	if(GpsStu.currentStatus == GPS_CLOSED && !flag)
	{
		flag = 1;
		GPS_LED_OFF();
	}
	else if((GpsStu.currentStatus == GPS_OPENED) && (GpsStu.locValidCh != 'A'))
	{
		if((AppStu.GpsLedOff == 0) && (flag == 0))
		{
			flag = 1;
			//NRF_LOG_INFO("000000000 flag == %d",flag);
			GPS_LED_ON();
			AppStu.GpsLedOn = FAST_ON;			
		}
		if((AppStu.GpsLedOn == 0) && (flag == 1))
		{
			flag = 0;
			//NRF_LOG_INFO("111111111 flag == %d",flag);
			GPS_LED_OFF();
			AppStu.GpsLedOff = FAST_OFF;
		}
		/*
		if(AppStu.GpsLedOff == 0)
		{
			NRF_LOG_INFO("22222222222222");
			flag = 1;
			GPS_LED_ON();
			AppStu.GpsLedOn = FAST_ON;
		}
		*/
	}
	else if(GpsStu.locValidCh == 'A' && !flag)
	{
		//NRF_LOG_INFO("333333333333");
		flag = 1;
		GPS_LED_ON();
	}
}
/*[HYN001-96] when remove sim card gsm led still keep on  by dujianfei at 20210305 begin*/
void APP_GsmLed(void)
{
	static volatile bool slow_flag = 0;
	static volatile bool fast_flag = 0;
	//static bool flag = 0;
	if(Net_GetNetInterface()->GetSimState() == SIM_STATE_READY)
	{
		if(Net_GetNetInterface()->GetNetRegistrationState() == NETWORK_SEARCHING)
		{
			if((AppStu.GpsLedOff == 0) && (fast_flag == 0))
			{
				fast_flag = 1;
				//NRF_LOG_INFO("444444444 fast_flag == %d",fast_flag);
				GSM_LED_ON();
				AppStu.GpsLedOn = FAST_ON;
			}
			if((AppStu.GpsLedOn == 0) && (fast_flag == 1))
			{
				fast_flag = 0;
				//NRF_LOG_INFO("555555555 fast_flag == %d",fast_flag);
				GSM_LED_OFF();
				AppStu.GpsLedOff = FAST_OFF;
			}
		}
		else if((Net_GetNetInterface()->GetNetRegistrationState() == NETWORK_REGISTERED) || (Net_GetNetInterface()->GetNetRegistrationState() == NETWORK_REGISTERED_ROAMING))
		{
			if((AppStu.GsmLedOff == 0) && (slow_flag == 0))
			{
				slow_flag = 1;
				//NRF_LOG_INFO("666666666 slow_flag == %d",slow_flag);
				GSM_LED_ON();
				AppStu.GpsLedOn = FAST_ON;
			}
			if((AppStu.GpsLedOn == 0) && (slow_flag == 1))
			{
				slow_flag = 0;
				//NRF_LOG_INFO("777777777 slow_flag == %d",slow_flag);
				GSM_LED_OFF();
				AppStu.GsmLedOff = SLOW_OFF;
			}
		}
		else
		{
			GSM_LED_OFF();
		}
	}
	else if(((Net_GetNetInterface()->GetSimState() == SIM_STATE_PIN_REQUIRED) || (Net_GetNetInterface()->GetSimState() == SIM_STATE_PUK_REQUIRED) || (Net_GetNetInterface()->GetSimState() == SIM_STATE_PHONE_SIM_PIN_REQUIRED)
		||(Net_GetNetInterface()->GetSimState() == SIM_STATE_PIN2_REQUIRED) || (Net_GetNetInterface()->GetSimState() == SIM_STATE_PUK2_REQUIRED) || (Net_GetNetInterface()->GetSimState() == SIM_STATE_PHONE_NETWORK_PIN_REQUIRED)))
	{
		//flag = 0;
		//NRF_LOG_INFO("LED SCENE:SimState is %d",Net_GetNetInterface()->GetSimState());
		GSM_LED_ON();
	}
	else if((Net_GetNetInterface()->GetSimState() == SIM_STATE_UNKNOWN) || (Net_GetNetInterface()->GetSimState() == SIM_STATE_ABSENT))
	{
		//flag = 1;
		//NRF_LOG_INFO("99999999");
		GSM_LED_OFF();
	}
}
/*[HYN001-96] when remove sim card gsm led still keep on  by dujianfei at 20210305 end*/
#if 0
volatile uint16_t led_on_hold_time = 30*60;
static bool flag = 0;
static u8 minute_count = 0;
void APP_LedAllOn(void)
{
	if(!AT_GTCFG_VAR.Led_On)
	{
		if(!flag)
		{
			flag = 1;
			led_ctrl(RED_LED, LED_ON);
			led_ctrl(GREEN_LED, LED_ON);
		}
		minute_count++;
		if(minute_count >=60)
		{
			minute_count = 0;
			if(led_on_hold_time == 0)
			{
				led_ctrl(RED_LED, LED_OFF);
				led_ctrl(GREEN_LED, LED_OFF);
			}
		}
	}
}
#else
volatile uint16_t led_on_hold_time = 30*60;//30 minutes
static bool thirty_minute_flag = 0;
static u8 second_count = 0;
//extern bool volatile LedOn_OnlyOnce_GetFlag;
bool LedOn_OnlyOnce_GetFlag = 0;
void APP_LedAllWork(void)
{
	if(LedOn_OnlyOnce_GetFlag)
	{
		thirty_minute_flag = 1;
		second_count++;
		if(second_count >=60)
		{
			second_count = 0;
			if(led_on_hold_time == 0)
			{
				thirty_minute_flag = 0;
				NRF_LOG_INFO("00000000000");
				GSM_LED_OFF();//led_ctrl(RED_LED, LED_OFF);
				GPS_LED_OFF();//led_ctrl(GREEN_LED, LED_OFF);
			}
		}
	}
}

#endif
void APP_LedFlash(void)
{
	if((LedOn_OnlyOnce_GetFlag) && (!thirty_minute_flag))
	{
		//thirty_minute_flag = 1;
		second_count++;
		APP_GpsLed();
		APP_GsmLed();
		if(second_count >= 60)
		{
			second_count = 0;
			if(led_on_hold_time == 0)
			{
				thirty_minute_flag = 1;
				//NRF_LOG_INFO("00000000000");
				GSM_LED_OFF();
				GPS_LED_OFF();
			}
		}
	}
	else if(!LedOn_OnlyOnce_GetFlag)
	{
		//NRF_LOG_INFO("1111111111111");
		APP_GpsLed();
		APP_GsmLed();
	}
}
/*[HYN001-61] Add led state for gps and gsm scene by dujianfei at 20210122 end*/
/*[HYN001-102] factory mode led status conflict  by dujianfei at 20210310 begin*/
void hy_led_handler(void *p_context)
{
	UNUSED_PARAMETER(p_context);
	CNT_SUB(AppStu.GpsLedOn);
	CNT_SUB(AppStu.GpsLedOff);
	CNT_SUB(AppStu.GsmLedOff);
	APP_LedFlash();
}

void APP_LEDInit(void)
{
	APP_TIMER_DEF(hy_ledtimer);
	app_timer_create(&hy_ledtimer, APP_TIMER_MODE_REPEATED, hy_led_handler);
	app_timer_start(hy_ledtimer, APP_TIMER_TICKS(100), NULL);
}
/*[HYN001-102] factory mode led status conflict  by dujianfei at 20210310 end*/

void APP_Init(void)
{
    /*[HYN001-7] add parameter read/write interface by yuechaofu at 20201125 begin*/
    fds_factoryParaInit();
    /*[HYN001-7] add parameter read/write interface by yuechaofu at 20201125 end*/
    if(!AT_GTCFG_VAR.Led_On)
       LedOn_OnlyOnce_GetFlag = 1;
    UserParaInit();
    RTT_InfoPrint(POWR_ON_PRT);
    /*[HYN001-14] Repair the problem of polygonal electronic fence detection error  by zhoushaoqing at 20201128 begin*/
    //TestAPPInitFencePara();
    fence_para_init();
    /*[HYN001-14] Repair the problem of polygonal electronic fence detection error  by zhoushaoqing at 20201128 end*/
}
