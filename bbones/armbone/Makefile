TARGETS=spi_status BB-SPI1-01-00A0.dtbo BB-Bitbanging-00A0.dtbo bitbanging
all: ${TARGETS} 

SimpleGPIO.o:SimpleGPIO.c 
	gcc  $^ -c -Wall -Wextra

spi_status.o:spi_status.c pins.h encoder_calibration.h
	gcc  $^ -c

spi_status:spi_status.o SimpleGPIO.o
	gcc  $^ -o $@

bitbanging.o:bitbanging.c pins.h encoder_calibration.h
	gcc  $^ -c -std=gnu99

bitbanging:bitbanging.o SimpleGPIO.o
	gcc  $^ -o $@

BB-SPI1-01-00A0.dtbo:BB-SPI1-01-00A0.dts
	dtc -o $@ -b o -@ $^

BB-Bitbanging-00A0.dtbo:BB-Bitbanging-00A0.dts
	dtc -o $@ -b o -@ $^
	cp $@ ~

clean:
	rm -f ${TARGETS} *.o *.dtbo



