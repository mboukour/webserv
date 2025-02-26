#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include <cerrno>
#include <fstream>
#include <ios>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/types.h>
 #include <sys/stat.h>
#include "../ResponseState/ResponseState.hpp"
const std::string YELLOW = "\033[33m";
const std::string RESET = "\033[0m";
#include <string>
#include <sys/socket.h>
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"

std::string const generateContent(const HttpRequest &request, HttpResponse &response, std::string const path){
    (void)response;
    (void)request;
    std::fstream fileToGet(path.c_str());
    if (fileToGet.fail() == true)
        throw HttpErrorException(NOT_FOUND, request, "cant find file");
    // std::string line;
    // while (std::getline(fileToGet, line))
    //     content += line + "\n";
    std::stringstream ss;
    ss << fileToGet.rdbuf();
    return ss.str();
}

// std::string generateHeader(){

// }


void HttpResponse::handleGetRequest(const HttpRequest& request) {
	std::string path = request.getRequestBlock()->getRoot() + request.getPath();
    std::cout << YELLOW << "path: [" << path << "]" << RESET << std::endl;
    // std::string const content = generateContent(request, *this, path);
    struct stat filestat;
    if (stat(path.c_str(), &filestat) == -1)
        throw HttpErrorException(NOT_FOUND, request, "cant find file");

    std::fstream fileToGet(path.c_str());
    if (fileToGet.fail() == true)
        throw HttpErrorException(NOT_FOUND, request, "cant find file");
    std::stringstream responseSs;

	responseSs << "HTTP/1.1 200 OK\r\n";
	responseSs << "Content-Type: video/mp4\r\n";
	responseSs << "Connection: close\r\n";
	responseSs << "Content-Length: " << filestat.st_size << "\r\n";
	responseSs << "\r\n";
    // std::string &response = responseSs.str();
    send(this->clientFd, responseSs.str().c_str(), responseSs.str().size(), 0);
    std::vector<char> buffer(4096);  // 4KB buffer
    while (true) {
        fileToGet.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = fileToGet.gcount();
    
        if (bytesRead == 0) break;  // EOF
    
        // Send the data
        ssize_t totalSent = 0;
        while (totalSent < bytesRead) {
            ssize_t bytesSent = send(clientFd, buffer.data() + totalSent, bytesRead - totalSent, 0);
            if (bytesSent == -1) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    std::streampos filePos = fileToGet.tellg();
                    filePos -= (bytesRead - totalSent);
                    struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLOUT;
                    ev.data.fd = this->clientFd;
                    ev.data.ptr = new ResponseState(path, filePos, this->clientFd, this->epollFd);
                    epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->clientFd, &ev);
                    return;
                } else {
                    perror("Error: ");
                    exit(1);
                }
            }
            totalSent += bytesSent;
        }
    }
    // const std::string &range = request.getHeader("Range");
    // if (range.empty()) {
    //     std::string content;
    //     std::string line;
    //     while (std::getline(fileToGet, line))
    //         content += line + "\n";
    //     send(this->clientFd, content.c_str(), content.size(), 0);
    // } else {
        
    // }
	// responseSs << content;
    // std::cout << 
    // std::cout << YELLOW << response << RESET << std::endl;]
    // std::cout << YELLOW << "Size: " << content.size() << RESET << std::endl;
    // ssize_t bytesSent = send(this->clientFd, content.c_str(), content.size(), 0);
    // if (bytesSent == -1) {
    //     if (errno == EWOULDBLOCK || errno == EAGAIN) {
    //         std::cout << "Blocked :(";
    //         exit(1);
    //     }
    // }

    // if (bytesSent != static_cast<ssize_t>(content.size())) {
    //     std::cout << YELLOW << "Not all bytes: " << bytesSent << RESET << std::endl;
    // }
	// if (bytesSent == -1) {

    // } 

    // getHeader will return the value of the header if it exists, or an empty string if it doesn't
    // nta dik sa3a checki hado w 3la hasab lvalue gha tkml, lt7t kayn examples d chno t9d tl9a
    std::string contentType = request.getHeader("Content-Type"); 
    std::string transferEncoding = request.getHeader("Transfer-Encoding");
    	// throw HttpErrorException(request.getVersion(), NOT_IMPLEMENTED, "Not Implemented", "no code yet ;)", request.getRequestBlock()->getErrorPageHtml(NOT_IMPLEMENTED));
}

// 1. Content-Type
// The Content-Type header indicates the media type (MIME type) of the data being sent in the request or response. It tells the client or server how to interpret the data.

// Common Content-Type values:

// Text Types:
// text/plain: Plain text without any formatting.
// text/html: HTML content (used for web pages).
// text/css: CSS stylesheets.
// text/csv: Comma-separated values (CSV) data.
// text/javascript: JavaScript code (though application/javascript is more common).

// Application Types:
// application/json: JSON data (commonly used in APIs).
// application/xml: XML data.
// application/javascript: JavaScript code.
// application/pdf: PDF files.
// application/octet-stream: Binary data (e.g., files for download).
// application/x-www-form-urlencoded: Form data encoded as key-value pairs (default for HTML forms).
// application/zip: ZIP archive files.

// Multipart Types
// multipart/form-data: Used for file uploads or forms with binary data. Each part of the form is separated by a boundary.
// multipart/byteranges: Used for partial content responses (e.g., resuming downloads).

// Image Types:
// image/jpeg: JPEG images.
// image/png: PNG images.
// image/gif: GIF images.
// image/svg+xml: SVG vector images.

// Audio/Video Types:
// audio/mpeg: MP3 audio files.
// video/mp4: MP4 video files.
// video/mpeg: MPEG video files.

// Font Types:
// font/woff: WOFF font files.
// font/woff2: WOFF2 font files.
// font/ttf: TrueType font files.

// Other Types:

// message/rfc822: Email messages.

// model/gltf-binary: 3D model files (GLTF).

// 2. Transfer-Encoding
// The Transfer-Encoding header specifies the encoding used to safely transfer the payload body to the user. It is primarily used in HTTP/1.1.

// Common Transfer-Encoding values:

// chunked:
// The data is sent in a series of chunks. Each chunk is preceded by its size in bytes, and the end of the data is marked by a chunk with a size of zero.
// Used when the total size of the data is unknown in advance (e.g., streaming).

// compress:
// The data is compressed using the UNIX compress program (not commonly used).

// deflate:
// The data is compressed using the zlib structure (RFC 1951).

// gzip:
// The data is compressed using the GNU zip format (RFC 1952). This is widely used for compressing HTTP responses.

// identity:
// No encoding is applied. This is the default if no Transfer-Encoding header is present.