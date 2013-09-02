TARGETS=spi_status
all: ${TARGETS} 

SimpleGPIO.o:SimpleGPIO.c 
	gcc  $^ -c

spi_status.o:spi_status.c 
	gcc  $^ -c

spi_status:spi_status.o SimpleGPIO.o
	gcc  $^ -o $@

clean:
	rm -f ${TARGETS} *.o 

