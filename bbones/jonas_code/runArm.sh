 #!/bin/bash
clear

gcc main_arm.c SimpleGPIO.c udp_communication.c spi_communication.c log.c data_decoding.c circular_buffer.c -DDEBUG=1 -o main_arm

# naser
./main_arm 10.33.136.49 8888 


