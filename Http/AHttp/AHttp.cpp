#include "AHttp.hpp"


AHttp::AHttp(): version(""), bodySize(0) , body("") {}

std::string AHttp::uriAllowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ._~:/?#[]@!$&'()*+,;=%-"; // lo?l -> lol


std::string AHttp::getVersion() const {
    return this->version;
}

size_t AHttp::getBodySize() const {
    return this->bodySize;
}

std::string AHttp::getBody() const {
    return this->body;
}


int HexToChar(char c) {
    if (isdigit(c))
        return c - '0';
    return tolower(c) - 'a' + 10;
}

void AHttp::encoding(std::string& str)
{
    for (size_t i = 0; i < str.length(); )
    {
        if (str[i] == '%' && i + 2 < str.length())
        {
            char c1 = tolower(str[i+1]);
            char c2 = tolower(str[i+2]);
            if (isxdigit(c1) && isxdigit(c2))
            {
                int value = HexToChar(c1) * 16 + HexToChar(c2);   
                str.replace(i, 3, 1, static_cast<char>(value));
                continue;
            }
        }
        i++;
    }
}

std::string AHttp::sanitizePath(std::string path) {
	int start;
	for (size_t i = 0; i < path.length(); )
	{
		if (path[i] == '.' && i > 0 && path[i - 1] == '/')
		{
			if (i + 1 < path.length() && path[i + 1] == '.')
			{
				start = i - 1;
				while (start > 0 && path[start - 1] != '/')
					start--;
				path.erase(start, i + 2 - start);
				i = start;
			}
			else if (i + 1 >= path.length() || path[i + 1] == '/')
				path.erase(i, 1);
			else
				i++;
		}
		else if (path[i] == '/' && i + 1 < path.length() && path[i + 1] == '/')
			path.erase(i + 1, 1);
		else
			i++;
	}
	for (size_t i = 0; i < path.length(); )
	{
		if (path[i] == '/' && i + 1 < path.length() && path[i + 1] == '/')
			path.erase(i + 1, 1);
		else
			i++;
	}
    std::string newPath;
    for (size_t i = 0; i < path.length();){
        if (uriAllowedChars.find(path[i]) == std::string::npos)
            i++;
        else
            newPath += path[i++];
    }
    encoding(newPath);
	if (newPath.empty())
		return "/";
	else
		return newPath;
}


std::string AHttp::getHeader(const std::string &headerName) const {
    std::map<std::string, std::string>::const_iterator it = this->headers.find(headerName);
    if (it != this->headers.end()) {
        return it->second;
    }
    return ""; // throw MissingHeader(headerName);
}

void AHttp::setHeader(const std::string &headerName, const std::string &headerValue) {
    this->headers[headerName] = headerValue;
}