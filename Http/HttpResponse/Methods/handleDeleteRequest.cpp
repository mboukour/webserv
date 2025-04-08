#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "../../../Debug/Debug.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <dirent.h>
#include "../../../Server/ServerManager/ServerManager.hpp"

void	HttpResponse::deleteResponse(const HttpRequest &request){
	int statusCode = 200;
	if (request.isCgiRequest())
			return;
		std::cout << "Sending response" << std::endl;
		std::string connectState;
		ClientState *state = ServerManager::getClientState(this->clientFd);
		if (state->getIsKeepAlive())
			connectState = "keep-alive";
		else
			connectState = "close";
		this->version = request.getVersion();
		this->statusCode = statusCode;
		this->reasonPhrase = HttpErrorException::getReasonPhrase(statusCode);
		this->body = this->success_delete;
		std::stringstream sS;
		sS << this->body.size();
		this->headers["Content-Length"] = sS.str();
		this->headers["Content-Type"] = "text/html";
		this->headers["Last-Modified"] = getFileLastModifiedTime(this->fileName);
		this->headers["Connection"] = connectState;
		if (state->getIsKeepAlive())
			this->headers["Keep-Alive"] = "timeout=10, max 1000"; // might make this dynamic later
		std::string toStr = this->toString();
		ServerManager::sendString(toStr, this->clientFd);
}

void HttpResponse::handleDeleteRequest(const HttpRequest &request)
{
	std::string path = sanitizePath(request.getRequestBlock()->getRoot() + request.getPath());
	std::cout << RED << path << RESET << std::endl;
	if (isDir(path.c_str()))
		throw HttpErrorException(BAD_REQUEST, request, "Directory upload is not supported");
	if (!std::remove(path.c_str()))
		deleteResponse(request);
	else
		throw HttpErrorException(INTERNAL_SERVER_ERROR, request, "Failed to delete resource!");
}
