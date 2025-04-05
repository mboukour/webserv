#include "../../../Debug/Debug.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "../../../Utils/Logger/Logger.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse.hpp"

#include "../../../ClientState/ClientState.hpp"
#include "../../../Debug/Debug.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "../../../Server/ServerManager/ServerManager.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse.hpp"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <ctime>
#include <iostream>
#include <sstream>
#include <sys/time.h>

#include <ctime>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

std::string getFileLastModifiedTime(const std::string &filePath) {
  struct stat fileStat;
  if (stat(filePath.c_str(), &fileStat) != 0)
    return "";
  struct tm *tm_info = gmtime(&fileStat.st_mtime); // Convert to GMT time
  char buffer[100];
  strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
  return std::string(buffer);
}

std::string regexReplace(const std::string &filename) {
  std::string result;
  for (size_t i = 0; i < filename.size(); i++) {
    if ((filename[i] >= 'a' && filename[i] <= 'z') || // lowercase letters
        (filename[i] >= 'A' && filename[i] <= 'Z') || // uppercase letters
        (filename[i] >= '0' && filename[i] <= '9') || // digits
        (filename[i] == '_') ||                       // underscore
        (filename[i] == '.') ||                       // period
        (filename[i] == '-')) {                       // hyphen
      result += filename[i];                          // Keep the character
    } else {
      result += '_'; // Replace with underscore
    }
  }
  return result;
}

std::string randomizeFileName() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned long seed = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  srand(seed);
  std::stringstream sS;
  sS << rand();
  std::string fileID = sS.str();
  return fileID;
}

// if the transfer encoding is set tp chunked, the content-lenght header
// presence is not necessary map with the content-type and it corresponding file
// extension parse is chunked is set

// why sometimes i get "Fatal Runtime error: BODY ISZE BIGGER THAN CL"

// TODO:
// generate response: the randomized file name must be mentioned in the response
// headers create && fill the file

std::string HttpResponse::getConTypeExten(const std::string &contentType) {
  if (contentType == "text/html")
    return ".html";
  else if (contentType == "text/plain")
    return ".txt";
  else if (contentType == "image/jpeg")
    return ".jpg";
  else if (contentType == "image/png")
    return ".png";
  else if (contentType == "image/gif")
    return ".gif";
  else if (contentType == "application/json")
    return ".json";
  else if (contentType == "application/xml")
    return ".xml";
  else if (contentType == "application/pdf")
    return ".pdf";
  else if (contentType == "application/zip")
    return ".zip";
  else if (contentType == "audio/mpeg")
    return ".mp3";
  else if (contentType == "audio/mp3")
    return ".mp3";
  else if (contentType == "video/mp4")
    return ".mp4";
  return ""; // Return an empty string if no match is found
}

std::string HttpResponse::extToNature(const std::string &extension) {
  if (extension == ".html" || extension == ".txt")
    return "text/";
  else if (extension == ".jpg" || extension == ".png" || extension == ".gif")
    return "image/";
  else if (extension == ".json" || extension == ".xml" || extension == ".pdf")
    return "document/";
  else if (extension == ".zip")
    return "application/";
  else if (extension == ".mp3")
    return "audio/";
  else if (extension == ".mp4")
    return "video/";
  return "";
}

bool isDir(const char *path) {
  struct stat info;
  if (stat(path, &info) != 0)
    return false;
  return (info.st_mode & S_IFDIR) != 0; // Check if it's a directory
                                        //      holds info    is it a dir
}

void HttpResponse::postResponse(const HttpRequest &request, int statusCode,
                                std::string body, std::string const fileName) {
	if (request.isCgiRequest())
		return;
	std::cout << "Sending response" << std::endl;
	std::string connectState;
	ClientState *state = ServerManager::getClientState(this->clientFd);
	if (state->getIsKeepAlive())
		connectState = "keep-alive";
	else
		connectState = "close";
	const Server *server = request.getServer();
	std::string serverName = server->getServerName();
	int serverPort = server->getPort();
	std::stringstream sS;
	sS << serverName << ":" << serverPort << "/" << fileName;
	std::string location = sS.str();
	this->version = request.getVersion();
	this->statusCode = statusCode;
	this->reasonPhrase = HttpErrorException::getReasonPhrase(statusCode);
	std::string find = "FILE_LOCATION";
	size_t pos = body.rfind(find);
	if (pos != std::string::npos) {
	body.replace(pos, find.length(), location);
	}
	find = "SC_RP";
	pos = body.rfind(find);
	if (pos != std::string::npos) {
	body.replace(pos, find.length(), this->reasonPhrase);
	}
	this->body = body;
	sS.str("");
	sS.clear();
	sS << this->body.size();
	this->headers["Content-Length"] = sS.str();
	this->headers["Content-Type"] = "text/html";
	sS.str("");
	sS.clear();
	this->headers["Last-Modified"] = getFileLastModifiedTime(this->fileName);
	this->headers["Location"] = location;
	this->headers["Connection"] = connectState;
	if (state->getIsKeepAlive()) {
		this->headers["Keep-Alive"] = "timeout=10, max 1000"; // might make this dynamic later
	}
	std::string toStr = this->toString();
	ServerManager::sendString(toStr, this->clientFd);
}

std::string HttpResponse::setFileName(const HttpRequest &request) {
  std::string __contentType = request.getHeader("Content-Type");
  std::string __exten = ".bin"; // we need to save every content-type with its
                                // corresponding extension, ofc in a map
  std::string __folder = "";
  std::string ret = getConTypeExten(__contentType);
  if (ret.empty() == false)
    __exten = ret;
  ret = extToNature(__exten);
  if (ret.empty() == false)
    __folder = ret;
  this->fileName = randomizeFileName() + __exten;
  __folder = "uploads/" + __folder;
  this->fileName = __folder + this->fileName;
  return __folder;
}

void HttpResponse::firstPostBin(const HttpRequest &request) {
	if (!request.isCgiRequest()) {
		if (isDir(setFileName(request).c_str()) == true) {
		  this->fd = open(this->fileName.c_str(), O_WRONLY | O_CREAT,
						  S_IRUSR | S_IWUSR); // check for failure
		  write(this->fd, request.getBody().c_str(), request.getBody().size());
		} else {
		  std::cout << RED << "Folder do not exist!" << RESET << std::endl;
		  // throw an exception with an appropriate error page!!!!!!!!!!!!!
		}
	} else {
		cgiState->activateWriteState(request.getBody());
	}
}

void	HttpResponse::setPacket(const HttpRequest &request){
	if (this->postState == INIT_POST){
		this->postState = NEW_REQ_ENTRY;
		if (!request.isCgiRequest()) {
			if (isDir(setFileName(request).c_str()) == true){
				this->fd = open(this->fileName.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // check for failure
			}
			else
				std::cout << RED << "Folder do not exist!" << RESET << std::endl; // might update it to throw an exception later
		}
		this->packet = request.getBody();
	}
	else
		this->packet = request.getReqEntry();
}

std::string visualizeEscapes(const std::string input) {
    std::string result;
    result.reserve(input.length() * 2); // Pre-allocate for efficiency

    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '\r') {
            result += "\\r";
        } else if (input[i] == '\n') {
            result += "\\n";
        } else {
            result += input[i];
        }
    }

    return result;
}

void	HttpResponse::chunkedTransfer(const HttpRequest &request){
	// Logger::getLogStream() << this->chunkState << ": " << visualizeEscapes(this->packet.substr(0, 10)) << " // " << visualizeEscapes(this->packet.substr(this->packet.length() - 10)) << std::endl;
	bool processing = true;
	size_t curr_pos = 0;
	this->offset = this->packet.length();
	while (processing)
	{
		// std::cout << "Called" << std::endl;
		switch (this->chunkState){
			case CH_START:
				this->chunkState = CH_SIZE;
				break ;
			case CH_SIZE:{
				size_t end;
				if (this->pendingCRLF) {
					this->pendingCRLF = false;
					curr_pos++;
				}
				for (end = curr_pos; end < this->offset; end++){
					if ((this->packet[end] == '\r' && this->packet[end + 1] == '\n') || this->packet[end] == '\n') {
						this->chunkState = CH_DATA;
						break;
					}
				}
				this->prev_chunk_size += this->packet.substr(curr_pos, end - curr_pos);
				if (this->prev_chunk_size.size() > 20)
					throw HttpErrorException(INTERNAL_SERVER_ERROR, request, "Size bigger than 20");
				curr_pos = end;
				if (this->chunkState == CH_DATA){
					std::stringstream ss(this->prev_chunk_size);
					this->prev_chunk_size = "";
					if ((this->packet[curr_pos] == '\r' && this->packet[curr_pos + 1] == '\n'))
						curr_pos += 2;
					else if (this->packet[curr_pos] == '\n')
						curr_pos++;
					ss >> std::setbase(16) >> this->remaining_chunk_size;
					if (this->remaining_chunk_size == 0)
						this->chunkState = CH_COMPLETE;
				}
				if (curr_pos >= this->offset)
					processing = false;
				break;
			}
			case CH_DATA:
			{
				size_t ch_size = this->offset - curr_pos;
				processing = false;
				if (this->remaining_chunk_size <= ch_size) { // chunk will end in this packet
					ch_size = this->remaining_chunk_size;
					processing = true;
					this->chunkState = CH_TRAILER;
				}
				if (request.isCgiRequest())
					cgiState->activateWriteState(this->packet.c_str() + curr_pos);
				else
					write(this->fd, this->packet.c_str() + curr_pos, ch_size);
				this->remaining_chunk_size -= ch_size;
				if (!this->remaining_chunk_size)
					this->chunkState = CH_TRAILER;
				curr_pos += ch_size;
				if (curr_pos >= this->offset)
					processing = false;
				break;
			}
			case CH_TRAILER:
				if ((this->packet[curr_pos] == '\r' && this->packet[curr_pos + 1] == '\n')) {
					if (curr_pos + 2 > this->offset)
						return;
					curr_pos += 2;
				}
				else if (this->packet[this->offset - 1] == '\r') {
					this->pendingCRLF = true;
					this->chunkState = CH_SIZE;
					return;
				}
				this->chunkState = CH_SIZE;
				break;
			case CH_COMPLETE:
					this->isLastEntry = true;
					if (!request.isCgiRequest())
						postResponse(request, 201, this->success_create, this->fileName);
					return;
				break;
			default:
				break;
		}
	}
}

std::pair<std::string, std::string> HttpResponse::newMapNode(const HttpRequest &request, std::string const &str) {
	if (str.empty())
		return std::pair<std::string, std::string>("", "");;
	size_t pos = str.find(":");
	if (pos == std::string::npos)
		throw HttpErrorException(INTERNAL_SERVER_ERROR, request, "Invalid form header![1]");
	std::string key = str.substr(0, pos);
	std::string value = str.substr(pos);
	return 	std::pair<std::string, std::string>(key, value);
}



std::map<std::string, std::string> HttpResponse::strToHeaderMap(const HttpRequest &request, std::string &str){
	std::map<std::string, std::string> headers;
	size_t pos = 0;
	size_t curr = 0;
	while (true){
		curr = 0;
		pos = str.find("\r\n");
		if (pos == std::string::npos)
			throw HttpErrorException(INTERNAL_SERVER_ERROR, request, "Invalid form header![2]");
		std::pair<std::string, std::string> newNode = newMapNode(request, str.substr(curr, pos));
		if (newNode.first == "" && newNode.second == "")
			return headers;
		headers.insert(headers.end(), newNode);
		curr = pos + 2;
		if (curr >= str.length())
			break;
		str = str.substr(curr);
	}
	return headers;
}

std::map<std::string, std::string> HttpResponse::extractFileInfo(const HttpRequest &request, std::string const &str) {
	std::map<std::string, std::string> info;
	size_t pos;
	std::stringstream sS(str);
	std::string string;
	std::string skip;
	std::string key;
	std::string value;
	if (str.empty())
		throw HttpErrorException(BAD_REQUEST, request, "Content-Disposition field not found.");
	sS >> skip;
	if (sS.fail() || sS.eof())
		throw HttpErrorException(INTERNAL_SERVER_ERROR, request, "Server encountered an unexpected internal error.");
	sS >> string;
	if (sS.fail() || sS.eof() || string != "form-data;")
		throw HttpErrorException(BAD_REQUEST, request, "Missing form-data field in a multipart form-data request!");
	sS >> string;
	pos = string.find("=");
	if (pos == std::string::npos)
		throw HttpErrorException(BAD_REQUEST, request, "Invalid name field in multipart form-data request!");
	key = string.substr(0, pos);
	if (key != "name")
		throw HttpErrorException(BAD_REQUEST, request, "Missing name field in a multipart form-data request!");
	value = string.substr(pos + 1);
	if (value.length() < 2 || value[0] != '"' || (value[value.length() - 1] != '"' && value[value.length() - 1] != ';'))
		throw HttpErrorException(BAD_REQUEST, request, "Malformed name field in multipart form-data request!");
	value = value.substr(1, value.length() - 3);
	info[key] = value;
	std::getline(sS, string);
	pos = string.find("=");
	if (pos == std::string::npos)
		throw HttpErrorException(BAD_REQUEST, request, "Invalid filename field in multipart form-data request!");
	key = string.substr(0, pos);
	while (!key.empty() && key[0] == ' ')
		key.erase(0, 1);
	if (key != "filename"){
		info["filename"] = "";
		return info;
	}
	value = string.substr(pos + 1);
	if (value.length() < 2 || value[0] != '"' || value[value.length() - 1] != '"')
		throw HttpErrorException(BAD_REQUEST, request, "Malformed filename field in multipart form-data request!");
	value = value.substr(1, value.length() - 2);
	info["filename"] = value;
	return info;
}

std::string HttpResponse::generateFileName(const HttpRequest &request, std::string &file){
	std::string folder = request.getRequestBlock()->getUploadPath();
	size_t pos = file.rfind(".");
	std::string fileExten = "";
	if (pos == std::string::npos && pos != 0){ // what if content-type header doesn't exist ?
		std::string contentType = headers["Content-Type"];
		std::stringstream s(contentType);
		s >> contentType; // to skip ": "
		s >> contentType;
		fileExten = getConTypeExten(contentType);
	}
	if (folder.empty()){
		std::cerr << YELLOW << "[ALERT!]: " << RESET << "The upload path was not set in the config file. Uploading the file to the root directory..." << std::endl;
		folder = request.getRequestBlock()->getRoot();
	}
	if (!isDir(folder.c_str())){
		std::cerr << YELLOW << "[ALERT!]: " << RESET;
		std::cerr << "Folder: " << folder << " ,was not found, uploading file to your workspace root..." << std::endl;
		folder = "";
	}
	return folder + file + fileExten;
}

void HttpResponse::multiForm(const HttpRequest &request){
	std::string const bound = "--" + request.getBoundary();
	size_t boundLen = bound.length();
	size_t curr_pos = 0;
	std::string tmp;
	if (this->postState == INIT_POST){
		this->postState = NEW_REQ_ENTRY;
		this->packet = request.getBody();
	}
	else
		this->packet = request.getReqEntry();
	while (true){
		switch (this->multiState){ // what if this is the last boundary ?
			case M_BOUND:
			{
				tmp = "";
				if (this->packet.length() == 0)
					return;
				if (boundLen > this->packet.length() - curr_pos) {
					this->currBound += this->packet.length() - curr_pos;
					break;
				}
				if (this->currBound != 0 && this->packet[curr_pos] == '\n'){
					curr_pos++;
					this->multiState = M_BODY;
					continue;
				}
				curr_pos += boundLen - this->currBound + 2; // skip the bound len or what is left from bound len as i can be split + \r\n
				if (curr_pos > this->packet.length()){
					this->currBound = 1;
					break;
				}
				this->currBound = 0;
				tmp = this->packet.substr(0, boundLen);
				if (tmp != bound){
					throw HttpErrorException(INTERNAL_SERVER_ERROR, request, "Boundary not found");
				}
				if (this->packet == bound + "--\r\n"){
					this->isLastEntry = true;
					return;
				}
				this->packet = this->packet.substr(curr_pos);
				this->multiState = M_HEADERS;
				this->subHeaders = "";
				break;
			}
			case M_HEADERS:
			{
				this->packet = this->subHeaders + this->packet;
				size_t head_pos = this->packet.find("\r\n\r\n");
				if (head_pos == std::string::npos){
					this->subHeaders += this->packet;
					this->packet.clear();
					return;
				}
				std::string headerStr = this->packet.substr(0, head_pos) + "\r\n";
				std::map<std::string, std::string> headers = strToHeaderMap(request, headerStr);
				std::map<std::string, std::string> contentDisposition = extractFileInfo(request, headers["Content-Disposition"]);
				curr_pos = head_pos + 4;
				this->packet = this->packet.substr(curr_pos);
				std::string file = contentDisposition["filename"];
				if (file == ""){
					this->skip = true;
					this->multiState = M_BODY;
					break;
				}
				this->fileName = generateFileName(request, file);
				this->fd = open(fileName.c_str(), O_CREAT | O_WRONLY, 0644);
				this->multiState = M_BODY;
				break;
			}
			case M_BODY:
			{
				this->multiBody += this->packet;
				this->packet.clear();
				size_t bound_pos = this->multiBody.find(bound);
				if (bound_pos == std::string::npos){
					if (this->multiBody.length() > boundLen){
						std::string toWrite = this->multiBody.substr(0, this->multiBody.length() - boundLen);
						if (this->skip == false)
							write(this->fd, toWrite.c_str(), toWrite.length());
						toWrite.clear();
						this->multiBody = this->multiBody.substr(this->multiBody.length() - boundLen);
					}
					return;
				}
				this->packet = this->multiBody.substr(bound_pos);
				this->multiBody = this->multiBody.substr(0, bound_pos - 2); // \r\n -> as long a you found the boundary, \r\n is 100% guaranteed to be there
				if (this->skip == false)
					write(this->fd, this->multiBody.c_str(), this->multiBody.length());
				this->multiBody.clear();
				curr_pos = 0; // the packet will now start from the next
				this->multiState = M_BOUND;
				close(this->fd);
				this->skip = false;
				break;
			}
			default:
				break;
		}
	}
}

void HttpResponse::multiForm_chunked(const HttpRequest &request){
	std::string const bound = "--" + request.getBoundary();
	size_t boundLen = bound.length();
	size_t curr_pos = 0;
	std::string tmp;
	while (true){
		switch (this->multiState){ // what if this is the last boundary ?
			case M_BOUND:
			{
				tmp = "";
				if (this->packet.length() == 0)
					return;
				if (boundLen > this->packet.length() - curr_pos) {
					this->currBound += this->packet.length() - curr_pos;
					break;
				}
				if (this->currBound != 0 && this->packet[curr_pos] == '\n'){
					curr_pos++;
					this->multiState = M_BODY;
					continue;
				}
				curr_pos += boundLen - this->currBound + 2; // skip the bound len or what is left from bound len as i can be split + \r\n
				if (curr_pos > this->packet.length()){
					this->currBound = 1;
					break;
				}
				this->currBound = 0;
				tmp = this->packet.substr(0, boundLen);
				if (tmp != bound){
					throw HttpErrorException(INTERNAL_SERVER_ERROR, request, "Boundary not found");
				}
				if (this->packet == bound + "--\r\n"){
					return;
				}
				this->packet = this->packet.substr(curr_pos);
				this->multiState = M_HEADERS;
				this->subHeaders = "";
				break;
			}
			case M_HEADERS:
			{
				this->packet = this->subHeaders + this->packet;
				size_t head_pos = this->packet.find("\r\n\r\n");
				if (head_pos == std::string::npos){
					this->subHeaders += this->packet;
					this->packet.clear();
					return;
				}
				std::string headerStr = this->packet.substr(0, head_pos) + "\r\n";
				std::map<std::string, std::string> headers = strToHeaderMap(request, headerStr);
				std::map<std::string, std::string> contentDisposition = extractFileInfo(request, headers["Content-Disposition"]);
				curr_pos = head_pos + 4;
				this->packet = this->packet.substr(curr_pos);
				std::string file = contentDisposition["filename"];
				if (file == ""){
					this->skip = true;
					this->multiState = M_BODY;
					break;
				}
				this->fileName = generateFileName(request, file);
				this->fd = open(fileName.c_str(), O_CREAT | O_WRONLY, 0644);
				this->multiState = M_BODY;
				break;
			}
			case M_BODY:
			{
				this->multiBody += this->packet;
				this->packet.clear();
				size_t bound_pos = this->multiBody.find(bound);
				if (bound_pos == std::string::npos){
					if (this->multiBody.length() > boundLen){
						std::string toWrite = this->multiBody.substr(0, this->multiBody.length() - boundLen);
						if (this->skip == false)
							write(this->fd, toWrite.c_str(), toWrite.length());
						toWrite.clear();
						this->multiBody = this->multiBody.substr(this->multiBody.length() - boundLen);
					}
					return;
				}
				this->packet = this->multiBody.substr(bound_pos);
				this->multiBody = this->multiBody.substr(0, bound_pos - 2); // \r\n -> as long a you found the boundary, \r\n is 100% guaranteed to be there
				if (this->skip == false)
					write(this->fd, this->multiBody.c_str(), this->multiBody.length());
				this->multiBody.clear();
				curr_pos = 0; // the packet will now start from the next
				this->multiState = M_BOUND;
				close(this->fd);
				this->skip = false;
				break;
			}
			default:
				break;
		}
	}
}

void HttpResponse::multiChunked(const HttpRequest &request){
	std::string const bound = "--" + request.getBoundary();
	// size_t boundLen = bound.length();
	bool processing = true;
	size_t curr_pos = 0;
	this->offset = this->packet.length();
	if (this->postState == INIT_POST){
		this->postState = NEW_REQ_ENTRY;
		this->packet = request.getBody();
	}
	else
		this->packet = request.getReqEntry();
	this->offset = this->packet.length();
	while (processing)
	{
		switch (this->chunkState){
			case CH_START:
				this->chunkState = CH_SIZE;
				break ;
			case CH_SIZE:{
				size_t end;
				if (this->pendingCRLF) {
					this->pendingCRLF = false;
					curr_pos++;
				}
				for (end = curr_pos; end < this->offset; end++){
					if ((this->packet[end] == '\r' && this->packet[end + 1] == '\n') || this->packet[end] == '\n') {
						this->chunkState = CH_DATA;
						break;
					}
				}
				this->prev_chunk_size += this->packet.substr(curr_pos, end - curr_pos);
				if (this->prev_chunk_size.size() > 20)
					throw HttpErrorException(INTERNAL_SERVER_ERROR, request, "Size bigger than 20");
				curr_pos = end;
				if (this->chunkState == CH_DATA){
					std::stringstream ss(this->prev_chunk_size);
					this->prev_chunk_size = "";
					if ((this->packet[curr_pos] == '\r' && this->packet[curr_pos + 1] == '\n'))
						curr_pos += 2;
					else if (this->packet[curr_pos] == '\n')
						curr_pos++;
					ss >> std::setbase(16) >> this->remaining_chunk_size;
					if (this->remaining_chunk_size == 0)
						this->chunkState = CH_COMPLETE;
				}
				if (curr_pos >= this->offset)
					processing = false;
				break;
			}
			case CH_DATA:
			{
				size_t ch_size = this->offset - curr_pos;
				processing = false;
				if (this->remaining_chunk_size <= ch_size) { // chunk will end in this packet
					ch_size = this->remaining_chunk_size;
					processing = true;
					this->chunkState = CH_TRAILER;
				}
				// Logger::getLogStream() << visualizeEscapes(this->packet.substr(curr_pos, ch_size));
				std::string save = this->packet;
				this->packet = this->packet.substr(curr_pos, ch_size);
				multiForm_chunked(request);
				this->packet = save;
				// write(this->fd, this->packet.c_str() + curr_pos, ch_size);
				this->remaining_chunk_size -= ch_size;
				if (!this->remaining_chunk_size)
					this->chunkState = CH_TRAILER;
				curr_pos += ch_size;
				if (curr_pos >= this->offset)
					processing = false;
				break;
			}
			case CH_TRAILER:
				if ((this->packet[curr_pos] == '\r' && this->packet[curr_pos + 1] == '\n')) {
					if (curr_pos + 2 > this->offset)
						return;
					curr_pos += 2;
				}
				else if (this->packet[this->offset - 1] == '\r') {
					this->pendingCRLF = true;
					this->chunkState = CH_SIZE;
					return;
				}
				this->chunkState = CH_SIZE;
				break;
			case CH_COMPLETE:
					this->isLastEntry = true;
					postResponse(request, 201, this->success_create, this->fileName);
					return;
				break;
			default:
				break;
		}
	}
}


void HttpResponse::handlePostRequest(const HttpRequest &request) {
	std::string path = request.getRequestBlock()->getRoot();
	std::string __contentType = request.getHeader("Content-Type");
	std::string __exten = ".bin"; // we need to save every content-type with its
									// corresponding extension, ofc in a map
	std::string __folder = "";
	if (request.getRequestBlock()->getIsLimited()) {
		std::cout << MAGENTA <<  "max body size: " << request.getRequestBlock()->getMaxBodySize() << RESET << std::endl;
	}
	if (request.isChunkedRequest() == false) {
		if (request.isMultiRequest() && !request.isCgiRequest()){
			this->isLastEntry = request.getBodySize() == request.getContentLength();
			multiForm(request);
			if (this->isLastEntry){
				postResponse(request, 201, this->success_create, this->fileName);
				close(this->fd);
			}
		}
		else {
			this->isLastEntry = request.getBodySize() == request.getContentLength();
			if (this->postState == INIT_POST || (this->postState == LAST_ENTRY && this->prevPostState == INIT_POST)) {
				firstPostBin(request);
				if (this->isLastEntry && !request.isCgiRequest())
					postResponse(request, 201, this->success_create, this->fileName);
				this->postState = NEW_REQ_ENTRY;
			}
			else {
				const std::string *buff = request.getReqEntryPtr();
				if (request.isCgiRequest())
					this->cgiState->activateWriteState(*buff);
				else
					write(this->fd, buff->c_str(), buff->size());
				if (this->isLastEntry && !request.isCgiRequest())
					postResponse(request, 201, this->success_create, this->fileName);
			}
		}
	}
	else{
		if (request.isMultiRequest())
			multiChunked(request);
		else {
			setPacket(request);
			chunkedTransfer(request);
		}
	}
}
