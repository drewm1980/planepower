#ifndef UART_COMMUNCATION_H_ 
#define UART_COMMUNCATION_H_

#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>

#define INPUT_BUFFER_SIZE 255 

enum uart_errCode {UART_ERR_READ_CHECKSUM = -4,UART_ERR_READ_LENGTH = -3,UART_ERR_READ_MESSAGE = -2,UART_ERR_READ_NO_DATA_IN_BUF = -1, UART_ERR_NONE=0,UART_ERR_SERIAL_PORT_FLUSH_INPUT,UART_ERR_SERIAL_PORT_FLUSH_OUTPUT,UART_ERR_SERIAL_PORT_OPEN,UART_ERR_SERIAL_PORT_CLOSE,UART_ERR_SERIAL_PORT_CREATE,UART_ERR_SERIAL_PORT_WRITE,UART_ERR_UNDEFINED};
typedef enum uart_errCode UART_errCode;
 
typedef struct{
	int fd;                        /* serial device fd          */
	struct termios orig_termios;   /* saved tty state structure */
	struct termios cur_termios;    /* tty state structure       */
}serial_port; 
 
serial_port *serial_stream;

struct Packets {
	struct Serial {
		uint32_t received;
		uint32_t lost;
	} serial;
	struct UDP {
		uint32_t received;
		uint32_t lost;
	} udp;
} packets;

union Serial_input {
	char buffer[INPUT_BUFFER_SIZE]; 
} serial_input;

//timers

struct timeval timers[10];

/********************************
 * PROTOTYPES PUBLIC
 * ******************************/
 
extern UART_errCode serial_port_create();
extern UART_errCode serial_port_setup(void); //returns the number of read bytes
extern int serial_input_check(void);
extern UART_errCode serial_port_write(uint8_t output[],long unsigned int message_length) ;
extern UART_errCode serial_port_close(void);
extern serial_port* serial_port_new(void);


#endif /*UART_COMMUNCATION_H_*/
