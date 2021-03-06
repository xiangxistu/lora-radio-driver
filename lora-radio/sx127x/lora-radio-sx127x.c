/*!
 * \file      lora-radio.c for sx127x
 *
 * \brief     LoRa Radio interface
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Forest-Rain
 */
#include <stdlib.h>
#include "lora-radio.h"
#include "sx127x-board.h"

#include <rtthread.h>
#include "drv_gpio.h"

#ifdef USING_LORA_RADIO_DRIVER_RTOS_SUPPORT

#define EV_LORA_RADIO_IRQ_MASK         0x0007 // DIO0 | DIO1 | DIO2 | DIO3 | DIO4 | DIO5 depend on board

static struct rt_event lora_radio_event;
static struct rt_thread lora_radio_thread;
static rt_uint8_t rt_lora_radio_thread_stack[4096];
#endif // end of USING_LORA_RADIO_DRIVER_RTOS_SUPPORT

static void LoRaRadioInit( RadioEvents_t *events );

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    LoRaRadioInit,
    SX127xGetStatus,
    SX127xSetModem,
    SX127xSetChannel,
    SX127xIsChannelFree,
    SX127xRandom,
    SX127xSetRxConfig,
    SX127xSetTxConfig,
    SX127xCheckRfFrequency,
    SX127xGetTimeOnAir,
    SX127xSend,
    SX127xSetSleep,
    SX127xSetStby,
    SX127xSetRx,
    SX127xStartCad,
    SX127xSetTxContinuousWave,
    SX127xReadRssi,
    SX127xWrite,
    SX127xRead,
    //SX127xWriteBuffer,
    //SX127xReadBuffer,
    SX127xSetMaxPayloadLength,
    SX127xSetPublicNetwork,
    SX127xGetWakeupTime,
    NULL, // void ( *IrqProcess )( void )
    SX127xCheck,
    //SX126x Only
    NULL, // void ( *RxBoosted )( uint32_t timeout ) 
    NULL, // void ( *SetRxDutyCycle )( uint32_t rxTime, uint32_t sleepTime ) 
};

#ifdef USING_LORA_RADIO_DRIVER_RTOS_SUPPORT

static uint8_t get_irq_index(uint32_t ev)
{
    uint32_t i = 0;
    for(i = 0; i < 32; i++)
    {
        if(ev & 0x01)
        {
            break;
        }
        ev >>= 1;
    }
    return i;
}

/**
  * @brief  lora_radio_thread_entry
  * @param  None
  * @retval None
  */
void lora_radio_thread_entry(void* parameter)
{
    rt_uint32_t ev;
    
    while(1)
    {
        if (rt_event_recv(&lora_radio_event, EV_LORA_RADIO_IRQ_MASK,
                                RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                                RT_WAITING_FOREVER, &ev) == RT_EOK)
        {
            RadioIrqProcess(get_irq_index(ev));
        }
    }
}
void LoRaRadioInit( RadioEvents_t *events )
{
    SX127xIoInit();
    SX127xInit(events);

    rt_event_init(&lora_radio_event, "ev_lora_phy", RT_IPC_FLAG_FIFO);

    rt_thread_init(&lora_radio_thread,               	  
                   "lora-phy",                     	  
                   lora_radio_thread_entry,          	  
                   RT_NULL,                    	  
                   &rt_lora_radio_thread_stack[0],       
                   sizeof(rt_lora_radio_thread_stack),  
                   1,   // highest priority                       	  
                   20);                           
                               
    rt_thread_startup(&lora_radio_thread);                      
}

void SX127xOnDio0IrqEvent( void *args )
{
    rt_event_send(&lora_radio_event, EV_LORA_RADIO_IRQ0_FIRED);
}
void SX127xOnDio1IrqEvent( void *args )
{
    rt_event_send(&lora_radio_event, EV_LORA_RADIO_IRQ1_FIRED);      
}
void SX127xOnDio2IrqEvent( void *args )
{
    rt_event_send(&lora_radio_event, EV_LORA_RADIO_IRQ2_FIRED);     
}
void SX127xOnDio3IrqEvent( void *args )
{
    rt_event_send(&lora_radio_event, EV_LORA_RADIO_IRQ3_FIRED);     
}
void SX127xOnDio4IrqEvent( void *args )
{
    rt_event_send(&lora_radio_event, EV_LORA_RADIO_IRQ4_FIRED);
}
void SX127xOnDio5IrqEvent( void *args )
{
    rt_event_send(&lora_radio_event, EV_LORA_RADIO_IRQ5_FIRED);
}


#else
void RadioInit( RadioEvents_t *events )
{
    SX127xInit(events);
}
#endif


