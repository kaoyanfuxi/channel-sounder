#!/bin/bash

# run with sudo

# make executable
#chmod +x setup_rmem_wrem.sh 

echo 'Setting net.core.rmem_max and net.core.wmem_max...'

sysctl -w net.core.rmem_max=62500000
sysctl -w net.core.wmem_max=62500000

echo 'Done!'


