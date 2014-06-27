#ifndef SPI_H_ 
#define SPI_H_

#include <stdint.h>
 
enum spi_errCode {SPI_ERR_NONE=0,SPI_ERR_UNDEFINED,SPI_ERR_OPEN_DEV};
typedef enum spi_errCode SPI_errCode;

//const char spidev[] = "/dev/spidev1.0";
//const char spidev1[] = "/dev/spidev1.1";

extern SPI_errCode spi_open();
extern SPI_errCode spi_read(uint8_t data_sensors[]);
extern SPI_errCode spi_close();
extern void SPI_err_handler(SPI_errCode err,void (*write_error_ptr)(char *,char *,int));  

#endif /*SPI_H_*/

