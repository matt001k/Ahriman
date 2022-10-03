#include "clock.h"
#include "cpu.h"
#include "delay.h"
#include "dma.h"
#include "flash.h"
#include "gpio.h"
#include "spi.h"
#include "timer.h"
#include "uart.h"
#include "watchdog.h"
#include "platform.h"
#include "ext_flash.h"
#include "systick.h"
#include "serial.h"
#include "wdt.h"
#include "led.h"
#include "run.h"
#include "blink.h"

static uint8_t txReady = 1U;
static uint8_t rxReady = 1U;

static void spi_Transfer(uint8_t *txData, uint8_t *rxData, uint32_t size);

//static uint8_t txDataWrite[] = {0x6U};
//static uint8_t txDataEraseSector[] = {0x20U, 0x00, 0x00, 0x00};
//static uint8_t txDataReadStatus[] = {0x5U, 0xFF};
//static uint8_t txDataWriteData[] = {0x02, 00, 00, 00, 0xAB, 0x11, 0x14};
//static uint8_t txDataReadData[] = {0x03, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff};
//static uint8_t rxData[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//static uint8_t rxDataStatus[] = {0x00, 0x00};
uint8_t byte_array_dec[1024u] = {80, 253, 241, 47, 192, 151, 94, 159, 142, 49, 173, 91, 124, 49, 96, 37, 180, 128, 236, 78, 241, 92, 17, 108, 178, 1, 129, 84, 230, 98, 63, 162, 59, 193, 229, 109, 119, 236, 88, 174, 60, 115, 26, 119, 24, 148, 73, 85, 170, 4, 126, 26, 150, 61, 131, 17, 87, 82, 106, 65, 42, 154, 145, 8, 140, 15, 248, 255, 125, 135, 208, 33, 40, 84, 184, 68, 197, 131, 186, 192, 233, 71, 161, 36, 180, 195, 151, 204, 7, 213, 227, 19, 36, 153, 65, 63, 113, 179, 224, 61, 173, 77, 100, 236, 137, 91, 41, 115, 120, 151, 249, 121, 54, 223, 93, 128, 84, 243, 58, 177, 152, 249, 19, 173, 129, 150, 140, 201, 199, 239, 111, 36, 51, 166, 46, 233, 147, 162, 75, 161, 235, 144, 231, 45, 154, 140, 47, 231, 236, 185, 248, 128, 123, 59, 220, 151, 226, 91, 254, 131, 231, 35, 122, 150, 174, 134, 127, 206, 122, 119, 69, 29, 139, 78, 192, 143, 7, 119, 199, 80, 148, 211, 51, 138, 91, 37, 57, 13, 71, 196, 134, 111, 78, 156, 208, 141, 118, 224, 102, 228, 48, 239, 170, 119, 17, 108, 86, 175, 152, 229, 102, 69, 65, 153, 196, 19, 196, 53, 18, 30, 135, 91, 121, 148, 8, 67, 152, 248, 150, 225, 30, 217, 211, 131, 73, 202, 240, 129, 4, 240, 178, 49, 130, 238, 195, 154, 42, 210, 168, 26, 2, 132, 251, 146, 206, 202, 231, 178, 129, 214, 184, 137, 212, 117, 157, 6, 165, 151, 184, 4, 51, 22, 61, 131, 173, 146, 190, 56, 211, 169, 214, 153, 100, 176, 139, 85, 37, 142, 117, 43, 225, 2, 77, 252, 101, 25, 237, 135, 73, 38, 180, 93, 15, 218, 36, 67, 239, 12, 113, 29, 64, 230, 82, 252, 177, 189, 143, 182, 220, 203, 226, 228, 16, 38, 235, 146, 144, 193, 14, 51, 177, 163, 108, 28, 190, 182, 228, 171, 106, 247, 13, 118, 48, 66, 148, 38, 143, 165, 195, 130, 5, 188, 161, 189, 118, 132, 178, 66, 57, 218, 45, 149, 115, 234, 51, 16, 29, 128, 3, 145, 92, 129, 88, 204, 14, 62, 42, 2, 107, 80, 28, 202, 200, 3, 52, 241, 1, 9, 0, 163, 216, 207, 156, 194, 137, 237, 156, 25, 188, 136, 2, 86, 192, 108, 89, 49, 165, 8, 226, 81, 113, 36, 206, 252, 184, 64, 142, 89, 5, 138, 21, 118, 106, 226, 123, 108, 133, 46, 74, 136, 0, 254, 178, 224, 206, 82, 55, 41, 247, 200, 10, 133, 69, 0, 90, 32, 50, 254, 86, 54, 28, 239, 227, 41, 33, 194, 122, 214, 160, 82, 198, 9, 13, 244, 140, 30, 63, 194, 52, 133, 207, 210, 191, 91, 70, 4, 7, 14, 248, 146, 151, 217, 204, 140, 152, 51, 27, 188, 209, 238, 41, 146, 35, 230, 158, 8, 27, 83, 234, 55, 120, 76, 158, 246, 11, 6, 35, 121, 140, 18, 141, 220, 249, 47, 79, 83, 101, 28, 28, 170, 55, 193, 104, 237, 167, 205, 112, 196, 180, 77, 240, 137, 9, 216, 242, 213, 5, 13, 112, 224, 137, 75, 4, 164, 211, 67, 168, 151, 155, 44, 45, 86, 213, 186, 93, 138, 55, 25, 17, 205, 174, 169, 226, 102, 26, 254, 59, 224, 146, 104, 141, 192, 223, 70, 4, 94, 83, 101, 107, 167, 71, 14, 39, 207, 205, 79, 37, 238, 28, 29, 172, 111, 61, 92, 110, 204, 155, 51, 231, 162, 79, 115, 98, 49, 214, 78, 203, 174, 231, 199, 249, 13, 183, 207, 214, 52, 132, 170, 54, 100, 40, 72, 104, 102, 92, 3, 234, 119, 58, 183, 126, 67, 234, 248, 225, 237, 83, 211, 150, 211, 24, 184, 174, 142, 69, 223, 23, 215, 247, 116, 61, 230, 152, 200, 190, 42, 142, 79, 106, 164, 1, 237, 73, 22, 141, 128, 130, 131, 108, 247, 25, 246, 208, 195, 220, 37, 66, 206, 5, 184, 88, 198, 203, 33, 94, 128, 6, 129, 120, 234, 135, 58, 9, 67, 167, 64, 5, 11, 220, 217, 168, 15, 69, 216, 172, 111, 126, 151, 170, 186, 61, 2, 206, 89, 84, 103, 198, 246, 2, 141, 241, 59, 52, 113, 107, 8, 99, 235, 4, 105, 212, 127, 111, 180, 49, 47, 168, 66, 134, 196, 152, 186, 255, 217, 90, 178, 151, 68, 160, 200, 126, 170, 18, 73, 233, 224, 71, 97, 50, 244, 18, 243, 251, 226, 213, 131, 32, 98, 74, 228, 80, 157, 223, 221, 144, 207, 208, 103, 66, 117, 19, 201, 177, 13, 45, 129, 182, 35, 165, 73, 65, 11, 171, 142, 141, 208, 77, 11, 36, 45, 65, 242, 249, 188, 234, 72, 181, 154, 24, 186, 246, 241, 166, 7, 229, 75, 241, 94, 130, 103, 116, 126, 231, 30, 174, 109, 226, 111, 26, 214, 152, 236, 181, 222, 44, 28, 37, 134, 124, 148, 208, 16, 155, 31, 87, 102, 71, 184, 127, 24, 79, 87, 30, 106, 230, 131, 35, 124, 35, 82, 132, 74, 46, 49, 241, 77, 203, 16, 86, 130, 235, 151, 134, 68, 225, 111, 9, 2, 14, 33, 4, 220, 195, 85, 52, 222, 221, 208, 134, 211, 208, 43, 50, 127, 62, 0, 68, 190, 109, 18, 140, 194, 59, 182, 213, 75, 145, 134, 211, 209, 21, 53, 77, 155, 49, 37, 87, 22, 93, 107, 197, 228, 215, 167, 13, 19, 104, 80, 5, 57, 190, 5, 75, 169, 98, 213, 116, 155, 165, 233, 112, 134, 28, 86, 62, 157, 194, 188, 202, 124, 189, 222, 226, 56, 234, 224, 83, 89, 38, 15, 229, 201, 9, 175, 36, 238, 33, 73, 228, 2, 71, 85, 10, 88, 141, 173, 108, 77, 13, 105, 163, 219, 193, 116, 86, 163, 24, 231, 173, 113, 62, 75, 158, 185, 101, 78, 84, 75, 152, 158, 158, 1, 109, 239, 30, 163, 242, 150, 159, 254, 133, 211, 158, 149, 106, 226, 248, 163, 206, 238, 212, 150, 247, 119}; 
uint8_t txData[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
uint8_t rxData[1024U] = {0x00};

#include <stdio.h>

int main(void)
{
    CPU_Init();
    Clock_Init(12000000);
    GPIO_Init();
    Flash_Init();
    Timer_Init();
    Delay_Init();
    //UART_Init();
    SPI_Init();
    DMA_Init();
    Systick_Init();
    Serial_Init();
    LED_Init();
    WDT_Init();

    Blink_Init();

    GPIO_Set(FLASH_WP);


    Ext_FlashInit();

    //Flash_Erase(0x00010000, 0x800);
    //Flash_Read(0x00010000, rxData, 1024);
    //Flash_Write(0x00010000, byte_array_dec, 1023);
    //Flash_Read(0x00010000, rxData, 1023);

    uint8_t printBuf[64] = {0u};

    while(1)
    {
        Run();
        //GPIO_Set(LED1);
        //GPIO_Set(LED2);
        //GPIO_Set(LED3);
        //Delay_Ms(1000U);
        //Watchdog_Kick();
        //GPIO_Clear(LED1);
        //GPIO_Clear(LED2);
        //GPIO_Clear(LED3);
        //snprintf((char *) printBuf, 64, "Runtime: %lX\r\n", Systick_GetMs());
        //Serial_Transmit((uint8_t *) printBuf, 64);
        ////spi_Transfer(txDataWrite, rxData, 1U);
        ////spi_Transfer(txDataEraseSector, rxData, 4U);
        ////rxDataStatus[1] = 0x1;
        ////waitCnt = 0;
        ////while (rxDataStatus[1] & 0x1)
        ////{
        ////    waitCnt++;
        ////    spi_Transfer(txDataReadStatus, rxDataStatus, 2U);
        ////}
        ////spi_Transfer(txDataReadData, rxData, 7U);
////
        ////waitCnt = 0;
        ////spi_Transfer(txDataWrite, rxData, 1U);
        ////spi_Transfer(txDataWriteData, rxData, 7U);
        ////rxDataStatus[1] = 0x1;
        ////while (rxDataStatus[1] & 0x1)
        ////{
        ////    waitCnt++;
        ////    spi_Transfer(txDataReadStatus, rxDataStatus, 2U);
        ////}
        ////spi_Transfer(txDataReadData, rxData, 7U);
        ////Ext_FlashEraseSector(100U);
        ////Ext_FlashEraseSector(101U);
        ////Ext_FlashStartRead(100, 4095, 5);
        ////while(Ext_FlashGetState() != FLASH_TRANSFER_STATE_DONE) {};
        ////Ext_FlashGetRead(rxData, 5);
        ////Ext_FlashWrite(100, 4095, txData, 5);
        ////Ext_FlashStartRead(100, 4095, 5);
        ////while(Ext_FlashGetState() != FLASH_TRANSFER_STATE_DONE) {};
        ////Ext_FlashGetRead(rxData, 5);
        //Delay_Ms(1000U);
        WDT_Kick();
    }
}


static void spi_Transfer(uint8_t *txData, uint8_t *rxData, uint32_t size)
{
    SPI_AssertCs(FLASH_CS);
    SPI_TransmitReceive(SPI_FLASH, txData, rxData, size);
}