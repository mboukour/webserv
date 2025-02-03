#!/usr/bin/env python3

import os
import sys
import psutil
import socket
import time

# Get system metrics
cpu_load = psutil.cpu_percent(interval=1)  # CPU load as a percentage
virtual_memory = psutil.virtual_memory()  # Memory usage stats
hostname = socket.gethostname()  # Hostname of the server
uptime_seconds = time.time() - psutil.boot_time()  # Uptime in seconds
uptime = time.strftime("%H:%M:%S", time.gmtime(uptime_seconds))  # Convert seconds to H:M:S format

# Print the HTTP header
print("Content-Type: text/html\r")  # Content type for HTML response
# Blank line to indicate the end of headers and beginning of body
print()

# Start of the HTML page
print("<html>")
print("<head>")
print("<title>System Metrics</title>")
print("<style>")
print("body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 20px; padding: 20px; }")
print("h1 { color: #333366; }")
print("table { border-collapse: collapse; width: 100%; margin-top: 20px; }")
print("th, td { padding: 8px 12px; text-align: left; border: 1px solid #ddd; }")
print("th { background-color: #f2f2f2; color: #333366; }")
print("tr:nth-child(even) { background-color: #f9f9f9; }")
print("tr:hover { background-color: #e0e0e0; }")
print("</style>")
print("</head>")
print("<body>")

# Page Title
print("<h1>System Metrics</h1>")

# Table to display system information
print("<table>")
print("<tr><th>Metric</th><th>Value</th></tr>")

# Display system metrics
print(f"<tr><td>Hostname</td><td>{hostname}</td></tr>")
print(f"<tr><td>CPU Load</td><td>{cpu_load}%</td></tr>")
print(f"<tr><td>Memory Usage</td><td>{virtual_memory.percent}% of {virtual_memory.total / (1024 ** 3):.2f} GB</td></tr>")
print(f"<tr><td>Uptime</td><td>{uptime}</td></tr>")

# Optional: Add more system stats if needed, like disk usage
disk_usage = psutil.disk_usage('/')
print(f"<tr><td>Disk Usage</td><td>{disk_usage.percent}% of {disk_usage.total / (1024 ** 3):.2f} GB</td></tr>")

print("</table>")

# Footer
print("<hr>")
print("<p>CGI script execution complete.</p>")

# End of the HTML page
print("</body>")
print("</html>")
