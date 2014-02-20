/*******************************************************************************
 *  example_main.c
 *
 *  Example program to demonstrate the RM024 C Library.
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
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "rm024.h"

HANDLE openComPort(char* portName)
{
    HANDLE port = CreateFileA(portName,
                      GENERIC_READ|GENERIC_WRITE,
                      0,                          /* no share  */
                      NULL,                       /* no security */
                      OPEN_EXISTING,
                      0,                          /* no threads */
                      NULL);                      /* no templates */

    if(port==INVALID_HANDLE_VALUE)
    {
        printf("unable to open comport\n");
        return(NULL);
    }

    DCB port_settings;
    memset(&port_settings, 0, sizeof(port_settings));  /* clear the new struct  */
    port_settings.DCBlength = sizeof(port_settings);

    if(!BuildCommDCBA("baud=115200 data=8 parity=N stop=1", &port_settings))
    {
      printf("unable to set comport dcb settings\n");
      CloseHandle(port);
      return(NULL);
    }

    if(!SetCommState(port, &port_settings))
    {
      printf("unable to set comport cfg settings\n");
      CloseHandle(port);
      return(NULL);
    }

    COMMTIMEOUTS Cptimeouts;

    Cptimeouts.ReadIntervalTimeout         = MAXDWORD;
    Cptimeouts.ReadTotalTimeoutMultiplier  = 0;
    Cptimeouts.ReadTotalTimeoutConstant    = 0;
    Cptimeouts.WriteTotalTimeoutMultiplier = 0;
    Cptimeouts.WriteTotalTimeoutConstant   = 0;

    if(!SetCommTimeouts(port, &Cptimeouts))
    {
      printf("unable to set comport time-out settings\n");
      CloseHandle(port);
      return(NULL);
    }

    return port;
}


void defaultServerSettings(HANDLE server)
{
    ERR errorCode;
    BYTE param[1];

    errorCode = WriteRM024(server, ENTER_COMMAND_MODE);
    if (errorCode != NO_ERRORS)
        printf("Error ENTER_COMMAND_MODE Code: %d\n",errorCode);

    errorCode = WriteRM024(server, RESTORE_RESET);
    if (errorCode != NO_ERRORS)
        printf("Error RESTORE_RESET Code: %d\n",errorCode);

    Sleep(3000);

    errorCode = WriteRM024(server, ENTER_COMMAND_MODE);
    if (errorCode != NO_ERRORS)
        printf("Error ENTER_COMMAND_MODE Code: %d\n",errorCode);

    errorCode = WriteRM024_EEPROM(server, EEPROM_SET_SERVER);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_SET_SERVER Code: %d\n",errorCode);

    errorCode = WriteRM024_EEPROM(server, EEPROM_RF_280_43_FCC);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_RF_280_43_FCC Code: %d\n",errorCode);

    param[0] = 0x33;
    errorCode = WriteRM024_EEPROM(server, EEPROM_CHANNEL_NUMBER, param);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_CHANNEL_NUMBER Code: %d\n",errorCode);

    errorCode = WriteRM024_EEPROM(server, EEPROM_SYSTEM_ID, param);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_SYSTEM_ID Code: %d\n",errorCode);

    errorCode = WriteRM024_EEPROM(server, EEPROM_LOW_POWER);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_LOW_POWER Code: %d\n",errorCode);

    param[0] = ENABLE;
    errorCode = WriteRM024_EEPROM(server, EEPROM_BROADCAST_MODE, param);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_BROADCAST_MODE Code: %d\n",errorCode);

    errorCode = WriteRM024(server, SOFT_RESET);
    if (errorCode != NO_ERRORS)
        printf("Error SOFT_RESET Code: %d\n",errorCode);
}


void defaultClientSettings(HANDLE client)
{
    ERR errorCode;
    BYTE param[1];

    errorCode = WriteRM024(client, ENTER_COMMAND_MODE);
    if (errorCode != NO_ERRORS)
        printf("Error ENTER_COMMAND_MODE Code: %d\n",errorCode);

    errorCode = WriteRM024(client, RESTORE_RESET);
    if (errorCode != NO_ERRORS)
        printf("Error RESTORE_RESET Code: %d\n",errorCode);

    Sleep(3000);

    errorCode = WriteRM024(client, ENTER_COMMAND_MODE);
    if (errorCode != NO_ERRORS)
        printf("Error ENTER_COMMAND_MODE Code: %d\n",errorCode);

    errorCode = WriteRM024_EEPROM(client, EEPROM_SET_CLIENT);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_SET_CLIENT Code: %d\n",errorCode);

    errorCode = WriteRM024_EEPROM(client, EEPROM_RF_280_43_FCC);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_RF_280_43_FCC Code: %d\n",errorCode);

    param[0] = 0x33;
    errorCode = WriteRM024_EEPROM(client, EEPROM_CHANNEL_NUMBER, param);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_CHANNEL_NUMBER Code: %d\n",errorCode);

    errorCode = WriteRM024_EEPROM(client, EEPROM_SYSTEM_ID, param);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_SYSTEM_ID Code: %d\n",errorCode);

    errorCode = WriteRM024_EEPROM(client, EEPROM_LOW_POWER);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_LOW_POWER Code: %d\n",errorCode);

    param[0] = ENABLE;
    errorCode = WriteRM024_EEPROM(client, EEPROM_BROADCAST_MODE, param);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_BROADCAST_MODE Code: %d\n",errorCode);

    errorCode = WriteRM024(client, SOFT_RESET);
    if (errorCode != NO_ERRORS)
        printf("Error SOFT_RESET Code: %d\n",errorCode);
}


void flashFirmware(const HANDLE device, char *filePre)
{
    BYTE *data = malloc(261 * sizeof(BYTE));
    BYTE *check = malloc(261 * sizeof(BYTE));

    FILE *filePtr;
    int fileIndex;
    char filePost[][9] = {"[00].bin\0","[24].bin\0","[25].bin\0","[26].bin\0","[27].bin\0","[28].bin\0","[29].bin\0","[30].bin\0",};
    char *fileName;

    // Attach the given filename prefix
    int count = 0;
    while(filePre[count++]);
    count--;
    fileName = malloc((count+sizeof(filePost[0]))*sizeof(char));
    memcpy(fileName,filePre,count);

    // Repeat 1-5 for each binary
    for (fileIndex = 0; fileIndex < 8; fileIndex++)
    {
        // Finish filename and open it for binary read
        memcpy(fileName+count,filePost[fileIndex],sizeof(filePost[fileIndex]));
        filePtr = fopen(fileName, "rb");
        if (!filePtr)
            goto FAIL;

        // 1 Erase Flash
        if (WriteRM024(device,ENTER_COMMAND_MODE))
            goto FAIL;
        if (WriteRM024(device,ERASE_FLASH))
            goto FAIL;

        // Set initial values
        data[0] = 0x00;
        data[1] = 0x00;
        data[2] = 0x01;
        data[3] = 0x00;

        // Repeat Until EOF
        while((fread((data+4), sizeof(BYTE), 256, filePtr)) > 0)
        {
            // 2 Write chunk from file
            if (WriteRM024(device,WRITE_FLASH,data))
                goto FAIL;

            Sleep(50);

            // 3 Read chunk to verify write
            if (ReadRM024(check,device,READ_FLASH,data))
                goto FAIL;

            // Check read command <result> byte
            if (check[2])
                goto FAIL;

            // Compare what was read to what was written
            if (memcmp(check + 1, data, 260) != 0)
                goto FAIL;

            printf("=");
            data[0]++;
        }
        fclose(filePtr);

        // 4 Decrypt FW image
        if (WriteRM024(device,DECRYPT_NEW_IMAGE))
            goto FAIL;

        // 5 Reset
        if (WriteRM024(device,SOFT_RESET))
            goto FAIL;

        printf("\n");
        Sleep(1300);
    }

    printf("Firmware Flash Successful\n");
    goto PASS;

    FAIL:
        printf("Error Flashing Firmware\n");
        fclose(filePtr);
    PASS:
        free((void*)data);
        free((void*)check);
}



int main()
{
    ERR errorCode;
    char *testString = "1234ABCD&$*(";
    char test[][4] = {"off","on"};
    int bytesWritten,bytesRead,i;

    BYTE *parameters = malloc(35*sizeof(BYTE));
    BYTE *returnValue = malloc(35*sizeof(BYTE));

    HANDLE server = openComPort("COM3");
    HANDLE client = openComPort("COM6");

    if (server == NULL || client == NULL)
    {
        return(1);
    }

    flashFirmware(client,"RM v1.3-0 ");

    defaultServerSettings(server);
    defaultClientSettings(client);

    Sleep(2000);

    // BEGIN TESTING
    if (WriteFile(server, testString, 13, (LPDWORD)((void *)&bytesWritten), NULL))
    {
        memset(returnValue,0,13);
        Sleep(500);
        if (ReadFile(client, returnValue, 13, (LPDWORD)((void *)&bytesRead), NULL))
        {
            printf("- %s - was read from the client\n",returnValue);
        }
    }

    Sleep(50);
    errorCode = WriteRM024(client,ENTER_COMMAND_MODE);
    if (errorCode != NO_ERRORS)
        printf("Error ENTER_COMMAND_MODE Code: %d\n",errorCode);

    parameters[0] = ON;
    errorCode = WriteRM024(client,AUTO_CHANNEL,parameters);
    if (errorCode != NO_ERRORS)
        printf("Error AUTO_CHANNEL Code: %d\n",errorCode);
    else
        printf("Write AUTO_CHANNEL as ENABLE\n");

    errorCode = WriteRM024(client,INTEGRATED_ANTENNA);
    if (errorCode != NO_ERRORS)
        printf("Error INTEGRATED_ANTENNA Code: %d\n",errorCode);
    else
        printf("Integrated antenna selected\n");

    errorCode = ReadRM024(returnValue,client,AUTO_CHANNEL);
    if (errorCode != NO_ERRORS)
        printf("Error AUTO_CHANNEL Code: %d\n",errorCode);
    else
        printf("Read AUTO_CHANNEL is %s\n",test[*returnValue]);

    errorCode = ReadRM024(returnValue,client,READ_TEMPERATURE);
    if (errorCode != NO_ERRORS)
        printf("Error READ_TEMPERATURE Code: %d\n",errorCode);
    else
        printf("Temp is %02X\n",*returnValue);

    parameters[0] = 0x41;
    errorCode = ReadRM024(returnValue,client,READ_IRAM,parameters);
    if (errorCode != NO_ERRORS)
        printf("Error READ_IRAM Code: %d\n",errorCode);
    else
        printf("Read IRAM is %02X\n",*returnValue);

    for (i=IRAM_RANGE_REFRESH;i<=IRAM_SYSTEM_ID;i++)
    {
        errorCode = ReadRM024(returnValue,client,i);
        if (errorCode != NO_ERRORS)
            printf("Error IRAM CMD %d Code: %d\n",i,errorCode);
        else
            printf("Read IRAM CMD %d is %02X\n",i,*returnValue);
    }

    errorCode = ReadRM024(returnValue,client,READ_DEST_ADDR);
    if (errorCode != NO_ERRORS)
        printf("Error READ_DEST_ADDR Code: %d\n",errorCode);
    else
        printf("Read READ_DEST_ADDR is %02X %02X %02X\n",returnValue[0],returnValue[1],returnValue[2]);

    errorCode = ReadRM024(returnValue,client,API_TRANSMIT);
    if (errorCode != NO_ERRORS)
        printf("Error API_TRANSMIT Code: %d\n",errorCode);
    else
        printf("Read API_TRANSMIT is %s\n",test[*returnValue]);

    parameters[0] = ENABLE;
    errorCode = WriteRM024(client,API_TRANSMIT,parameters);
    if (errorCode != NO_ERRORS)
        printf("Error API_TRANSMIT Code: %d\n",errorCode);

    errorCode = ReadRM024(returnValue,client,API_TRANSMIT);
    if (errorCode != NO_ERRORS)
        printf("Error API_TRANSMIT Code: %d\n",errorCode);
    else
        printf("Read API_TRANSMIT is %s\n",test[*returnValue]);

    errorCode = WriteRM024_EEPROM(client,EEPROM_RF_280_43_FCC);
    if (errorCode != NO_ERRORS)
        printf("Error setting Client RF mode Code: %d\n",errorCode);
    else
        printf("Client RF Mode successful\n");

    errorCode = ReadRM024_EEPROM(returnValue,client,EEPROM_MAC_ADDRESS);
    if (errorCode != NO_ERRORS)
        printf("Error EEPROM_MAC_ADDRESS Code: %d\n",errorCode);
    else
        printf("Read EEPROM_MAC_ADDRESS is %02X-%02X-%02X-%02X-%02X-%02X\n",returnValue[0],returnValue[1],returnValue[2],returnValue[3],returnValue[4],returnValue[5]);

    parameters = returnValue;
    errorCode = WriteRM024_EEPROM(client,EEPROM_DEST_MAC_ADDRESS,parameters);
    if (errorCode != NO_ERRORS)
        printf("Error setting EEPROM_DEST_MAC_ADDRESS: %d\n",errorCode);
    else
        printf("EEPROM_DEST_MAC_ADDRESS successful\n");

    errorCode = WriteRM024(client,EXIT_COMMAND_MODE);
    if (errorCode != NO_ERRORS)
        printf("Error EXIT_COMMAND_MODE Code: %d\n",errorCode);

    errorCode = FormatSleepTime(12,S,parameters);
    if (errorCode != NO_ERRORS)
        printf("Error! Code: %d\n",errorCode);

    errorCode = WriteRM024(client,SLEEP_W_TIMER,parameters);
    if (errorCode != NO_ERRORS)
        printf("Error SLEEP_W_TIMER Code: %d\n",errorCode);
    else
        printf("Sleeping for 12 seconds\n");


    CloseHandle(server);
    CloseHandle(client);

    return 0;
}
