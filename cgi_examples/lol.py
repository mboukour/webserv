#!/usr/bin/env python3
import cgi
import cgitb
import os
import sys
import tempfile
import time
from io import BytesIO

# Enable detailed error reporting
cgitb.enable()

class HTTPTester:
    def __init__(self):
        self.env = os.environ
        self.content_type = self.env.get('CONTENT_TYPE', '')
        self.request_method = self.env.get('REQUEST_METHOD', 'GET')
        self.content_length = self.env.get('CONTENT_LENGTH', '')
        self.transfer_encoding = self.env.get('HTTP_TRANSFER_ENCODING', '')
        
    def handle_request(self):
        """Main request handler that routes to appropriate methods"""
        print("Content-Type: text/html\n")
        
        print("""<!DOCTYPE html>
        <html>
        <head>
            <title>HTTP Test Tool</title>
            <style>
                body { font-family: Arial, sans-serif; margin: 20px; }
                h1 { color: #333; }
                h2 { color: #555; }
                pre { background-color: #f5f5f5; padding: 10px; border: 1px solid #ddd; }
                .container { margin-bottom: 20px; }
                form { margin: 15px 0; }
                table { border-collapse: collapse; width: 100%; }
                th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
                th { background-color: #f2f2f2; }
            </style>
        </head>
        <body>
            <h1>HTTP Testing Tool</h1>""")
        
        # Display request information
        self.show_request_info()
        
        if self.request_method == 'GET':
            self.handle_get()
        elif self.request_method == 'POST':
            self.handle_post()
        
        # Show forms for testing
        self.show_test_forms()
        
        print("</body></html>")
        
    def show_request_info(self):
        """Display information about the current request"""
        print("<div class='container'>")
        print("<h2>Request Information</h2>")
        print("<table>")
        print("<tr><th>Variable</th><th>Value</th></tr>")
        print(f"<tr><td>Request Method</td><td>{self.request_method}</td></tr>")
        print(f"<tr><td>Content-Type</td><td>{self.content_type}</td></tr>")
        print(f"<tr><td>Content-Length</td><td>{self.content_length}</td></tr>")
        print(f"<tr><td>Transfer-Encoding</td><td>{self.transfer_encoding}</td></tr>")
        print(f"<tr><td>Script Name</td><td>{self.env.get('SCRIPT_NAME', '')}</td></tr>")
        print(f"<tr><td>Query String</td><td>{self.env.get('QUERY_STRING', '')}</td></tr>")
        print(f"<tr><td>Remote Address</td><td>{self.env.get('REMOTE_ADDR', '')}</td></tr>")
        print("</table>")
        print("</div>")
        
    def handle_get(self):
        """Handle GET requests"""
        query_string = self.env.get('QUERY_STRING', '')
        if query_string:
            print("<div class='container'>")
            print("<h2>GET Parameters</h2>")
            print("<table>")
            print("<tr><th>Parameter</th><th>Value</th></tr>")
            
            params = {}
            for param in query_string.split('&'):
                if '=' in param:
                    key, value = param.split('=', 1)
                    params[key] = value
                    print(f"<tr><td>{key}</td><td>{value}</td></tr>")
                
            print("</table>")
            print("</div>")
            
            # Handle download request
            if 'download' in params:
                self.generate_download(params.get('download', '10'))
        
    def handle_post(self):
        """Handle POST requests"""
        if self.content_type.startswith('multipart/form-data'):
            self.handle_multipart()
        elif self.content_type == 'application/x-www-form-urlencoded':
            self.handle_form_urlencoded()
        else:
            self.handle_raw_post()
            
    def handle_multipart(self):
        """Handle multipart/form-data uploads"""
        print("<div class='container'>")
        print("<h2>Multipart Form Data</h2>")
        
        try:
            form = cgi.FieldStorage()
            
            # Display form fields
            print("<h3>Form Fields</h3>")
            print("<table>")
            print("<tr><th>Field</th><th>Value</th></tr>")
            
            for key in form.keys():
                if not isinstance(form[key], list):
                    items = [form[key]]
                else:
                    items = form[key]
                    
                for item in items:
                    if item.filename:
                        print(f"<tr><td>{key}</td><td>File: {item.filename} ({len(item.value)} bytes)</td></tr>")
                    else:
                        print(f"<tr><td>{key}</td><td>{item.value}</td></tr>")
            
            print("</table>")
            
            # Handle uploaded files
            print("<h3>Uploaded Files</h3>")
            for key in form.keys():
                if not isinstance(form[key], list):
                    items = [form[key]]
                else:
                    items = form[key]
                    
                for item in items:
                    if item.filename:
                        print(f"<p>File uploaded: {item.filename}</p>")
                        print(f"<p>Size: {len(item.value)} bytes</p>")
                        print(f"<p>Content type: {item.type}</p>")
                        
                        # Show first 100 bytes in hex
                        print("<p>First 100 bytes (hex):</p>")
                        hex_dump = ' '.join(f"{b:02x}" for b in item.value[:100])
                        print(f"<pre>{hex_dump}</pre>")
                        
        except Exception as e:
            print(f"<p>Error processing multipart data: {e}</p>")
            
        print("</div>")
        
    def handle_form_urlencoded(self):
        """Handle application/x-www-form-urlencoded data"""
        print("<div class='container'>")
        print("<h2>Form URL Encoded Data</h2>")
        
        try:
            form = cgi.FieldStorage()
            
            print("<table>")
            print("<tr><th>Field</th><th>Value</th></tr>")
            
            for key in form.keys():
                print(f"<tr><td>{key}</td><td>{form.getvalue(key)}</td></tr>")
            
            print("</table>")
            
        except Exception as e:
            print(f"<p>Error processing form data: {e}</p>")
            
        print("</div>")
        
    def handle_raw_post(self):
        """Handle raw POST data"""
        print("<div class='container'>")
        print("<h2>Raw POST Data</h2>")
        
        try:
            if self.content_length:
                length = int(self.content_length)
                data = sys.stdin.buffer.read(length)
                
                print(f"<p>Received {len(data)} bytes of data</p>")
                
                # Display first 1000 bytes as hex and text
                print("<h3>Hex View (first 1000 bytes)</h3>")
                hex_dump = ' '.join(f"{b:02x}" for b in data[:1000])
                print(f"<pre>{hex_dump}</pre>")
                
                print("<h3>Text View (first 1000 bytes, if printable)</h3>")
                text_data = ''.join(chr(b) if 32 <= b < 127 else '.' for b in data[:1000])
                print(f"<pre>{text_data}</pre>")
                
            elif self.transfer_encoding == 'chunked':
                # Handle chunked encoding
                print("<p>Chunked encoding detected</p>")
                chunks = []
                chunk_sizes = []
                
                # Read chunked data
                while True:
                    line = sys.stdin.readline().strip()
                    chunk_size = int(line, 16)
                    chunk_sizes.append(chunk_size)
                    
                    if chunk_size == 0:
                        break
                        
                    chunk = sys.stdin.buffer.read(chunk_size)
                    chunks.append(chunk)
                    
                    # Read CRLF
                    sys.stdin.readline()
                
                total_size = sum(len(chunk) for chunk in chunks)
                print(f"<p>Received {len(chunks)} chunks, total size: {total_size} bytes</p>")
                
                print("<h3>Chunks</h3>")
                print("<table>")
                print("<tr><th>Chunk #</th><th>Size</th><th>Preview</th></tr>")
                
                for i, (size, chunk) in enumerate(zip(chunk_sizes, chunks)):
                    preview = ''.join(chr(b) if 32 <= b < 127 else '.' for b in chunk[:50])
                    if len(chunk) > 50:
                        preview += "..."
                    print(f"<tr><td>{i+1}</td><td>{size}</td><td>{preview}</td></tr>")
                
                print("</table>")
                
            else:
                print("<p>No content length or chunked encoding specified</p>")
                
        except Exception as e:
            print(f"<p>Error processing POST data: {e}</p>")
            
        print("</div>")
        
    def generate_download(self, size_param):
        """Generate a file for download testing"""
        try:
            size = int(size_param)
            if size > 100 * 1024 * 1024:  # Limit to 100MB
                size = 100 * 1024 * 1024
                
            # Redirect to download handler
            print(f"""
            <div class="container">
                <h2>Download Test</h2>
                <p>Generating a file of {size} bytes for download...</p>
                <p><a href="?download_file={size}" target="_blank">Download File</a></p>
                
                <h3>Download Options</h3>
                <form method="get">
                    <label for="download_size">File size (bytes):</label>
                    <input type="number" name="download_file" id="download_size" value="{size}">
                    <button type="submit">Generate Download</button>
                </form>
            </div>
            """)
            
        except ValueError:
            print("<p>Invalid size parameter</p>")
            
    def show_test_forms(self):
        """Show forms for testing various HTTP features"""
        print("""
        <div class="container">
            <h2>Test Forms</h2>
            
            <h3>1. Standard Form Upload</h3>
            <form method="post" enctype="multipart/form-data">
                <p>
                    <label for="name">Name:</label>
                    <input type="text" name="name" id="name">
                </p>
                <p>
                    <label for="file1">File:</label>
                    <input type="file" name="file1" id="file1">
                </p>
                <p>
                    <button type="submit">Upload</button>
                </p>
            </form>
            
            <h3>2. Multiple File Upload</h3>
            <form method="post" enctype="multipart/form-data">
                <p>
                    <label for="files">Select multiple files:</label>
                    <input type="file" name="files" id="files" multiple>
                </p>
                <p>
                    <button type="submit">Upload Multiple</button>
                </p>
            </form>
            
            <h3>3. Generate Test Download</h3>
            <form method="get">
                <p>
                    <label for="download">File size (bytes):</label>
                    <input type="number" name="download" id="download" value="10240">
                </p>
                <p>
                    <button type="submit">Generate</button>
                </p>
            </form>
            
            <h3>4. Raw POST Test</h3>
            <p>Use curl to test raw POST and chunked encoding:</p>
            <pre>
# Test with Content-Length
curl -X POST -H "Content-Type: application/octet-stream" --data-binary @file.bin http://yourserver/path/to/script.py

# Test with chunked encoding
curl -X POST -H "Content-Type: application/octet-stream" -H "Transfer-Encoding: chunked" --data-binary @file.bin http://yourserver/path/to/script.py

# Test with multipart/form-data
curl -X POST -F "name=test" -F "file=@file.bin" http://yourserver/path/to/script.py
</pre>
        </div>
        """)

def main():
    """Main function to run the script"""
    tester = HTTPTester()
    
    # Check if download file is requested
    query_string = os.environ.get('QUERY_STRING', '')
    params = {}
    for param in query_string.split('&'):
        if '=' in param:
            key, value = param.split('=', 1)
            params[key] = value
            
    if 'download_file' in params:
        try:
            size = int(params['download_file'])
            if size > 100 * 1024 * 1024:  # Limit to 100MB
                size = 100 * 1024 * 1024
                
            # Send file headers
            print("Content-Type: application/octet-stream")
            print(f"Content-Length: {size}")
            print(f"Content-Disposition: attachment; filename=test_file_{size}_bytes.bin")
            print("")
            
            # Generate and send file data
            sys.stdout.flush()  # Ensure headers are sent
            
            # Generate data in chunks for efficiency
            chunk_size = 8192
            bytes_sent = 0
            
            while bytes_sent < size:
                chunk = min(chunk_size, size - bytes_sent)
                # Create some patterned data for verification
                pattern = bytes([i % 256 for i in range(256)])
                repeats = (chunk + 255) // 256
                data = pattern * repeats
                sys.stdout.buffer.write(data[:chunk])
                bytes_sent += chunk
                
            sys.stdout.buffer.flush()
            return
            
        except Exception as e:
            print("Content-Type: text/plain\n")
            print(f"Error generating download: {e}")
            return
    
    # Handle normal request
    tester.handle_request()

if __name__ == "__main__":
    main()