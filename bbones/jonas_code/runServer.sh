 #!/bin/bash
clear
gcc main_server.c log.c udp_communication.c uart_communication.c data_decoding.c analyze.c -pthread -o main_server -DDEBUG=0 -DANALYZE=0


./main_server 10.33.136.55 7777 8888 
