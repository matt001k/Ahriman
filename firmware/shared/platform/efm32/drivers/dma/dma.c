/**************************************************************************//**
 * (c) 2022 Ahriman
 * This code is licensed under MIT license (see LICENSE.txt for details)
 *****************************************************************************/

/**
 * @addtogroup dma
 * @{
 */

/**
 * @file        dma.c
 *
 * @brief       Provides the API for DMA Driver
 *
 * @author      Matthew Krause
 *
 * @date        2022-09-03
 */

/*****************************************************************************/
#include "dma.h"
#include "efm32tg11b520f128gm64.h"
#include <stddef.h>

#define DMA_PRIORITY (1U)

#define DMA_FLAG_CLEAR (0U)
#define DMA_FLAG_SET (1U)

#define DMA_DIRECTION_M2P (0U)
#define DMA_DIRECTION_P2M (1U)

#define DMA_CHANNEL_0 (0 << _LDMA_CHEN_CHEN_SHIFT)
#define DMA_CHANNEL_1 (1 << _LDMA_CHEN_CHEN_SHIFT)
#define DMA_CHANNEL_2 (2 << _LDMA_CHEN_CHEN_SHIFT)
#define DMA_CHANNEL_3 (3 << _LDMA_CHEN_CHEN_SHIFT)
#define DMA_CHANNEL_4 (4 << _LDMA_CHEN_CHEN_SHIFT)
#define DMA_CHANNEL_5 (5 << _LDMA_CHEN_CHEN_SHIFT)
#define DMA_CHANNEL_6 (6 << _LDMA_CHEN_CHEN_SHIFT)
#define DMA_CHANNEL_7 (7 << _LDMA_CHEN_CHEN_SHIFT)

#define DMA_GET_CHANNEL_SHIFT(i) (1UL << dCfg[i].channel)

typedef struct
{
    uint8_t channel;
    uint8_t swRequest : 1U;
    uint8_t dir : 1U;
    uint32_t source;
    uint32_t signal;
    uint32_t size;
    uint32_t dstinc;
    uint32_t srcinc;
    uint32_t block;
    uint32_t address;
    DMA_Callback_t cb;
    void *arg;
} dma_Config_t;

static dma_Config_t dCfg[NUM_DMA] =
{
    [UART_DEBUG_DMA] =
    {
        DMA_CHANNEL_6,
        DMA_FLAG_SET,
        DMA_DIRECTION_M2P,
        LDMA_CH_REQSEL_SOURCESEL_USART1,
        LDMA_CH_REQSEL_SIGSEL_USART1TXBL,
        LDMA_CH_CTRL_SIZE_BYTE,
        LDMA_CH_CTRL_DSTINC_NONE,
        LDMA_CH_CTRL_SRCINC_ONE,
        LDMA_CH_CTRL_BLOCKSIZE_UNIT1,
        (uint32_t) &USART1->TXDATA,
        NULL,
        NULL,
    },
    [SPI_FLASH_TX_DMA] =
    {
        DMA_CHANNEL_4,
        DMA_FLAG_CLEAR,
        DMA_DIRECTION_M2P,
        LDMA_CH_REQSEL_SOURCESEL_USART2,
        LDMA_CH_REQSEL_SIGSEL_USART2TXBL,
        LDMA_CH_CTRL_SIZE_BYTE,
        LDMA_CH_CTRL_DSTINC_NONE,
        LDMA_CH_CTRL_SRCINC_ONE,
        LDMA_CH_CTRL_BLOCKSIZE_UNIT1,
        (uint32_t) &USART2->TXDATA,
        NULL,
        NULL,
    },
    [SPI_FLASH_RX_DMA] =
    {
        DMA_CHANNEL_5,
        DMA_FLAG_CLEAR,
        DMA_DIRECTION_P2M,
        LDMA_CH_REQSEL_SOURCESEL_USART2,
        LDMA_CH_REQSEL_SIGSEL_USART2RXDATAV,
        LDMA_CH_CTRL_SIZE_BYTE,
        LDMA_CH_CTRL_DSTINC_ONE,
        LDMA_CH_CTRL_SRCINC_NONE,
        LDMA_CH_CTRL_BLOCKSIZE_UNIT1,
        (uint32_t) &USART2->RXDATA,
        NULL,
        NULL,
    },
};

void DMA_Init(void)
{
    /* Enable bus clock for peripheral */
    CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_LDMA;

    /* Ensure Round Robin mode is not used */
    LDMA->CTRL = LDMA_CTRL_NUMFIXED_DEFAULT;

    /* Enable interrupts */
    LDMA->IEN = LDMA_IEN_ERROR |
                (_LDMA_IEN_DONE_MASK << _LDMA_IEN_DONE_SHIFT);
    NVIC_ClearPendingIRQ(LDMA_IRQn);
    NVIC_SetPriority(LDMA_IRQn, DMA_PRIORITY);
    NVIC_EnableIRQ(LDMA_IRQn);

    for (uint8_t dIdx = 0U; dIdx < NUM_DMA; dIdx++)
    {
        /* Disable software requests if required */
        LDMA->REQDIS |= dCfg[dIdx].swRequest ? 0U : dCfg[dIdx].channel;
        
        /* Enable peripheral request/signal if required */
        LDMA->CH[dCfg[dIdx].channel].REQSEL = dCfg[dIdx].source |
                                                 dCfg[dIdx].signal;
    }
}

uint32_t DMA_StartTransfer(DMA_Channel_t channel, void *data, uint32_t size)
{
    if (data)
    {
        /* Do not write to the channel if it is busy */
        while (LDMA->CHBUSY & (DMA_GET_CHANNEL_SHIFT(channel))) {};

        /* Configure the control register */
        LDMA->CH[dCfg[channel].channel].CTRL = dCfg[channel].size |
                                               LDMA_CH_CTRL_REQMODE_BLOCK |
                                               LDMA_CH_CTRL_DONEIFSEN |
                                               dCfg[channel].block |
                                               dCfg[channel].dstinc |
                                               dCfg[channel].srcinc |
                                               ((size - 1U) << 
                                               _LDMA_CH_CTRL_XFERCNT_SHIFT) |
                                               LDMA_CH_CTRL_STRUCTTYPE_TRANSFER;

        /* Configure source and destination memory registers */
        if (dCfg[channel].dir == DMA_DIRECTION_M2P)
        {
            LDMA->CH[dCfg[channel].channel].SRC = (uint32_t) data;
            LDMA->CH[dCfg[channel].channel].DST = dCfg[channel].address;
        }
        else
        {
            LDMA->CH[dCfg[channel].channel].SRC = dCfg[channel].address;
            LDMA->CH[dCfg[channel].channel].DST = (uint32_t) data;
        }

        /* Enable the channel */
        LDMA->CHEN |= DMA_GET_CHANNEL_SHIFT(channel);

        /* If request select is set to nothing use a software request */
        if (LDMA->CH[dCfg[channel].channel].REQSEL & 
            LDMA_CH_REQSEL_SOURCESEL_NONE)
        {
            LDMA->SWREQ |= DMA_GET_CHANNEL_SHIFT(channel);
        }
    }
    else
    {
        size = 0U;
    }

    return size;
}

void DMA_RegisterCb(DMA_Channel_t channel, DMA_Callback_t cb, void *arg)
{
    if (cb)
    {
        dCfg[channel].cb = cb;
    }
    if (arg)
    {
        dCfg[channel].arg = arg;
    }
}

void LDMA_IRQHandler(void)
{
    if (LDMA->IF & LDMA_IF_ERROR)
    {
        LDMA->IFC |= LDMA_IFC_ERROR;
    }
    else
    {
        for (uint8_t dIdx = 0U; dIdx < NUM_DMA; dIdx++)
        {
            if (LDMA->IF & (DMA_GET_CHANNEL_SHIFT(dIdx)))
            {
                if (dCfg[dIdx].cb)
                {
                    dCfg[dIdx].cb(dCfg[dIdx].arg);
                }
                LDMA->IFC |= (DMA_GET_CHANNEL_SHIFT(dIdx));
            }
        }
    }
}

/**@} dma */