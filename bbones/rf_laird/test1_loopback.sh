#!/usr/bin/env bash

# Test UART pins by jumpering them together.
# for reference: 
# P9.21 is UART2 TXD
# P9.22 is UART2 RXD
# P9.1,2 are GND
# P9.3,4 are 3V
# P9.5,6,7,8 are two different 5V

TTY=/dev/ttyO2
echo "If you see this, the loopback test Succeeded!">TTY
cat TTY


