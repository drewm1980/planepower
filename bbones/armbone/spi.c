/*
 * * Copyright (C) 2013 Alan Backlund
 * *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 as
 * * published by the Free Software Foundation.
 * */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

int fd;

int main(int argc, const char *argv[])
{
	    const char *spidev = "/dev/spidev1.0";
            if (argc == 2)
	    spidev = argv[1];
	    fd = open(spidev, O_RDWR);
	    if (fd < 0) {
	               perror("file open");
	               abort();
	                }

	    uint8_t cmd[1], rsp[1];
            cmd[0] = 0X02;
	   /* cmd[1] = 0x12;
            cmd[2] = 0x34;
	    cmd[3] = 0x56;
*/          
	    int status=0;

	    struct spi_ioc_transfer tr;
	    tr.tx_buf = (unsigned long)cmd;

           if (status==0){
	    tr.rx_buf = (unsigned long)rsp;
	   }
	   else{
		   tr.rx_buf = 0;
		   rsp[0]=0;
		   printf("Wrong Information\n");
	   }
	    tr.len = 4;
            tr.delay_usecs = 0;
	    tr.speed_hz = 0;
            tr.bits_per_word = 0;
	    tr.cs_change = 1;
            
	    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

		    if (ret < 0) {

		    perror("ioctl");

                 	 }
	

	    printf("cmd: 0x%02X\n", cmd[0]);

	    printf("rsp: 0x%02X\n", rsp[0]);

	    const char *spidev1 = "/dev/spidev1.1";
            if (argc == 2)
	    spidev1 = argv[1];
	    fd = open(spidev1, O_RDWR);
	    if (fd < 0) {
	               perror("file open");
	               abort();
	                }

	    uint8_t cmd1[4], rsp1[4];
            cmd1[0] = 0X06;
	    cmd1[1] = 0x16;
            cmd1[2] = 0x38;
	    cmd1[3] = 0x59;

	    struct spi_ioc_transfer tr1;
	    tr1.tx_buf = (unsigned long)cmd1;
            tr1.rx_buf = (unsigned long)rsp1;
	    tr1.len = 4;
            tr1.delay_usecs = 0;
	    tr1.speed_hz = 0;
            tr1.bits_per_word = 0;
	    tr1.cs_change = 1;

	    int ret1 = ioctl(fd, SPI_IOC_MESSAGE(1), &tr1);

		    if (ret1 < 0) {

		    perror("ioctl");

                 	 }
	

	    printf("cmd: 0x%02X 0x%02X 0x%02X 0x%02X\n", cmd1[0], cmd1[1], cmd1[2], cmd1[3]);

	    printf("rsp: 0x%02X 0x%02X 0x%02X 0x%02X\n", rsp1[0], rsp1[1], rsp1[2], rsp1[3]);

	    close(fd);
}

