/******************************************************************************
 * File Name   : hy_uart.c
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
#include "hy_gsm.h"

static uint8_t Byte;
volatile uint8_t g_uartOpen = DBG_UART;

void uart_event_handle(app_uart_evt_t *p_event)
{
    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            if (NRF_SUCCESS == app_uart_get(&Byte))
            {
                if (g_uartOpen == GSM_UART) {
                    Gsm_OnUartEvent(Byte);
                } else if (g_uartOpen == DBG_UART) {
                    Factory_OnUartEvent(Byte);
                }
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            //APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            //APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        case APP_UART_TX_EMPTY:
            //NRF_LOG_INFO("uart_event_handle, data sending completed!\r\n");
            break;

        default:
            break;
    }
}

void UART_SendByte(uint8_t Byte)
{
    UNUSED_VARIABLE(app_uart_put(Byte));
}

void UART_SendMultiByte(const uint8_t *pData, uint16_t len)
{
    uint16_t Index;

    for (Index = 0; Index < len; Index++)
    {
        UART_SendByte(pData[Index]);
    }
}

void UA_SendGsmData(const uint8_t *pInBuf, uint16_t SLen)
{
    UART_SendMultiByte(pInBuf, SLen);
}

void Uart_Switch(UART_SEL sel)
{
    /*[HYN001-6]  modify nordic com and simcom7600 com by weibole at 20201124 start*/
    if (sel == DBG_UART || sel == BLE_UART)
    {
        nrf_gpio_pin_write(UART_SEL_A, 0);
    }
    else if (sel == GSM_UART)
    {
        nrf_gpio_pin_write(UART_SEL_A, 1);
    }
    g_uartOpen = sel;
    /*[HYN001-6]  modify nordic com and simcom7600 com by weibole at 20201124 end*/
}
/*[HMN001-166]OTA adds flow control and modifies network request interface by zhoushaoqing at 20210202 begin*/
void GSM_Flow_Ctrl(void)
{
    uint8_t Ret = 0;

	  Ret = Net_SendCmd((uint8_t *)"AT+IFC=2,2\r", 2, NULL, 0);
    if (Ret == OK)
    {
        app_uart_close();
        BSP_UartInit(BAUD_115200, APP_UART_FLOW_CONTROL_ENABLED);
    }
}
void GSM_BuadRateAdj(void)
{
    uint8_t Ret = 0;

	  Ret = Net_SendCmd((uint8_t *)"AT+IPR=115200\r", 2, NULL, 0);
    if (Ret == RET_OK)
    {
        app_uart_close();
        BSP_UartInit(BAUD_115200, APP_UART_FLOW_CONTROL_DISABLED);
    }
}
/*[HMN001-166]OTA adds flow control and modifies network request interface by zhoushaoqing at 20210202 end*/
