/*******************************************************************************
 *  embedded.h
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
#ifndef EMBEDDED_H_INCLUDED
#define EMBEDDED_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include "efm32_usart.h"

typedef USART_TypeDef *HANDLE;


unsigned char* Read(const HANDLE device, int length);
int Write(const HANDLE device, const unsigned char *buffer, int length);
void delay(uint32_t dlyTicks);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* EMBEDDED_H_INCLUDED */
