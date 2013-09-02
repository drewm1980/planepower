TARGETS=spi_status BB-SPI1-01-00A0.dtbo
all: ${TARGETS} 

SimpleGPIO.o:SimpleGPIO.c 
	gcc  $^ -c

spi_status.o:spi_status.c 
	gcc  $^ -c

spi_status:spi_status.o SimpleGPIO.o
	gcc  $^ -o $@

BB-SPI1-01-00A0.dtbo:BB-SPI1-01-00A0.dts
	dtc -o $@ -b o -@ $^
	cp $@ ~

clean:
	rm -f ${TARGETS} *.o *.dtbo



