 #!/bin/bash
clear
gcc main_plane.c udp_communication.c uart_communication.c log.c circular_buffer.c data_decoding.c  -pthread -DDEBUG=0 -o main_plane

# Milan
#./main_plane 10.33.132.47 8888 7777

# Jonas
# ./main_plane 10.33.136.11 8888 7777
#jonas
./main_plane 10.33.136.11 8888 7777

#carousel2
#./main_plane 10.33.136.31 8888 7777



