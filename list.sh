#!/bin/bash

while true; do
    clear
    echo "===== System Resource Usage ====="

    # CPU Usage (Total)
    echo -n "CPU Usage: "
    mpstat 1 1 | awk '/all/ {print 100 - $13"%"}'

    # RAM Usage (Total)
    echo -n "RAM Usage: "
    free -h | awk '/Mem:/ {print $3 "/" $2 " used (" $3/$2*100 "%)"}'

    # Disk Usage
    echo "Disk Usage:"
    df -h | grep '^/'

    # GPU Usage (for NVIDIA GPUs)
    echo -n "GPU Usage: "
    nvidia-smi --query-gpu=utilization.gpu,memory.used,memory.total --format=csv,noheader,nounits 2>/dev/null || echo "No NVIDIA GPU detected"

    echo "===== Webserv Process Usage ====="

    # Check if Webserv is running
    pid=$(pgrep -x Webserv)
    if [[ -z "$pid" ]]; then
        echo "Webserv is not running."
    else
        # CPU & Memory usage of Webserv
        ps -p $pid -o %cpu,%mem,cmd --no-headers | awk '{print "CPU: " $1 "%, RAM: " $2 "%, CMD: " $3}'
    fi

    sleep 2  # Refresh every 2 seconds
done
