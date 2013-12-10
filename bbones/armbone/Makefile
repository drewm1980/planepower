TARGETS=test_spi BB-SPI1-01-00A0.dtbo BB-Bitbanging-00A0.dtbo 
all: ${TARGETS} 

CC=gcc
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -std=gnu99

HEADERS=bitbang_spi.h  encoder_calibration.h  pins.h  SimpleGPIO.h

SimpleGPIO.o:SimpleGPIO.c ${HEADERS}

test_spi.o: test_spi.c ${HEADERS}

bitbang_spi.o: bitbang_spi.c $(HEADERS)

test_spi:test_spi.o bitbang_spi.o  SimpleGPIO.o

#spi_status.o:spi_status.c pins.h encoder_calibration.h

#spi_status:spi_status.o SimpleGPIO.o

#bitbanging.o:bitbanging.c pins.h encoder_calibration.h

#bitbanging:bitbanging.o SimpleGPIO.o

BB-SPI1-01-00A0.dtbo:BB-SPI1-01-00A0.dts
	dtc -o $@ -b o -@ $^

BB-Bitbanging-00A0.dtbo:BB-Bitbanging-00A0.dts
	dtc -o $@ -b o -@ $^
	cp $@ ~

clean:
	rm -f ${TARGETS} *.o *.dtbo



