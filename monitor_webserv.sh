#!/bin/bash

INTERVAL=2  # Update interval in seconds
OUTPUT_FILE="webserv_stats.log"  # Optional log file
PROCESS_NAME="Webserv"

# Colors for better readability
BLUE='\033[0;34m'
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'  # No Color

# Function to clear screen and print header
print_header() {
    clear
    echo -e "${BLUE}===============================================${NC}"
    echo -e "${BLUE}         WEBSERV MONITORING STATS${NC}"
    echo -e "${BLUE}===============================================${NC}"
    echo -e "${YELLOW}Process name:${NC} $PROCESS_NAME"
    echo -e "${YELLOW}Monitoring interval:${NC} ${INTERVAL}s"
    echo -e "${YELLOW}Current time:${NC} $(date +"%H:%M:%S")"
    echo -e "${BLUE}===============================================${NC}"
}

# Check if the process exists
check_process() {
    PID=$(pgrep -x "$PROCESS_NAME" 2>/dev/null)
    if [ -z "$PID" ]; then
        PID=$(pgrep -f "$PROCESS_NAME" 2>/dev/null | head -1)
        if [ -z "$PID" ]; then
            return 1
        fi
    fi
    echo "$PID"
}

# Main monitoring loop
monitor() {
    # Initialize log file if requested
    if [ "$LOG_TO_FILE" = true ]; then
        echo "Time,PID,CPU%,MEM%,VSZ(KB),RSS(KB),Connections,FDs,Threads" > "$OUTPUT_FILE"
    fi

    while true; do
        print_header
        
        PID=$(check_process)
        if [ -z "$PID" ]; then
            echo -e "${RED}Process $PROCESS_NAME is not running!${NC}"
            sleep $INTERVAL
            continue
        fi
        
        echo -e "${GREEN}PID:${NC} $PID"

        # Basic process info
        if ps -p "$PID" >/dev/null 2>&1; then
            PROCESS_INFO=$(ps -p "$PID" -o pcpu,pmem,vsz,rss --no-headers)
            PROC_CPU=$(echo "$PROCESS_INFO" | awk '{print $1}')
            PROC_MEM=$(echo "$PROCESS_INFO" | awk '{print $2}')
            PROC_VSZ=$(echo "$PROCESS_INFO" | awk '{print $3}')
            PROC_RSS=$(echo "$PROCESS_INFO" | awk '{print $4}')
            
            # Display information
            echo -e "${GREEN}CPU Usage:${NC} $PROC_CPU%"
            echo -e "${GREEN}Memory Usage:${NC} $PROC_MEM%"
            echo -e "${GREEN}Virtual Memory:${NC} $(echo "$PROC_VSZ/1024" | bc 2>/dev/null || echo "$PROC_VSZ") MB"
            echo -e "${GREEN}Resident Memory:${NC} $(echo "$PROC_RSS/1024" | bc 2>/dev/null || echo "$PROC_RSS") MB"
        else
            echo -e "${RED}Process information unavailable${NC}"
        fi
        
        # Network connections
        CONNECTIONS=$(netstat -anp 2>/dev/null | grep -c "$PID" || echo "N/A")
        
        # Open file descriptors
        if [ -d "/proc/$PID/fd" ]; then
            FD_COUNT=$(ls -1 /proc/"$PID"/fd 2>/dev/null | wc -l)
        else
            FD_COUNT="N/A"
        fi
        
        # Thread count
        THREAD_COUNT=$(ps -p "$PID" -o nlwp --no-headers 2>/dev/null || echo "N/A")
        
        echo -e "${GREEN}Active Connections:${NC} $CONNECTIONS"
        echo -e "${GREEN}Open File Descriptors:${NC} $FD_COUNT"
        echo -e "${GREEN}Thread Count:${NC} $THREAD_COUNT"
        echo -e "${BLUE}===============================================${NC}"
        
        # Connection details
        echo -e "${YELLOW}Connection States:${NC}"
        netstat -anp 2>/dev/null | grep "$PID" | grep -v "127.0.0.1" | awk '{print $5,$6}' | sort | uniq -c | sort -nr
        
        # Optional logging
        if [ "$LOG_TO_FILE" = true ]; then
            echo "$(date +"%H:%M:%S"),$PID,$PROC_CPU,$PROC_MEM,$PROC_VSZ,$PROC_RSS,$CONNECTIONS,$FD_COUNT,$THREAD_COUNT" >> "$OUTPUT_FILE"
        fi
        
        # Epoll events (relevant for your implementation)
        echo -e "${BLUE}===============================================${NC}"
        echo -e "${YELLOW}Recent Epoll Events (from dmesg, if available):${NC}"
        dmesg | grep -i "epoll" | tail -5 2>/dev/null || echo "No epoll events found"
        
        sleep $INTERVAL
    done
}

# Parse arguments
LOG_TO_FILE=false
for arg in "$@"; do
    case $arg in
        --log)
            LOG_TO_FILE=true
            shift
            ;;
        --interval=*)
            INTERVAL="${arg#*=}"
            shift
            ;;
        --process=*)
            PROCESS_NAME="${arg#*=}"
            shift
            ;;
        *)
            echo "Unknown argument: $arg"
            exit 1
            ;;
    esac
done

# Run the monitor
monitor