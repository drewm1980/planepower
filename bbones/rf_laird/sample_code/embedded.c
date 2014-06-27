/*******************************************************************************
 *  embedded.c
 *
 *  Hardware specific routines to supplement the RM024 C Library for the EFM32 Tiny
 *    Gecko Start Kit.
 *
 *  January 3, 2013
 *  Version 1.0.0
 *
 *  wireless.support@lairdtech.com
 *
 *  Copyright (c) 2012-2013, Laird Technologies, Inc - http://www.lairdtech.com
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Laird Technologies, Inc nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL LAIRD TECHNOLOGIES, INC BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
#include "embedded.h"

volatile uint32_t msTicks; /* counts 1ms timeTicks */


/*
 *  Reads from the given serial port
 *
 *  INPUTS: device - the handle for the connected port
 *          buffer - a pointer to the response holder
 *          length - number of bytes to read
 *
 *  OUTPUTS: int - number of bytes read or -1 on error
 */
unsigned char* Read(const HANDLE device, int length)
{
    int i;
    uint16_t count;
    unsigned char *buffOut = malloc(length * sizeof(BYTE));
    unsigned char *buffer = buffOut;

    for (i = 0; i < length; i++)
    {
        count = 0;
        while (!(device->STATUS & USART_STATUS_RXDATAV) && (count < 65535))
            count++;
        if (count < 65535)
            *(buffer++) = device->RXDATA;
        else
            buffOut = 0;
    }
    return buffOut;
}


/*
 *  Writes to the given serial port
 *
 *  INPUTS: device - the handle for the connected serial port
 *          buffer - a pointer to the data to write
 *          length - number of bytes to write
 *
 *  OUTPUTS: int - number of bytes written or -1 on error
 */
int Write(const HANDLE device, const unsigned char *buffer, int length)
{
    int i;
    uint16_t count;
    for (i = 0; i < length; i++)
    {
        count = 0;
        while (!(device->STATUS & USART_STATUS_TXBL) && (count < 65535))
            count++;
        if (count < 65535)
            device->TXDATA = *(buffer++);
        else
            return (-1);
    }
    return i;
}


/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       /* increment counter necessary in Delay()*/
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}
