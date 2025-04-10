#!/usr/bin/env python3

import cgi
import time

# Print the HTTP header, setting a fixed Content-Length (incorrectly)
print("Content-Type: text/html")
print("Transfer-Encoding: identity")
# print("Content-Length: 50")  # Incorrect length!
print()  # Blank line to separate headers from body

# Print more content than the length specified in the header
content = "<html><body><h1>This content exceeds the declared length!</h1></body></html>"
print(content)