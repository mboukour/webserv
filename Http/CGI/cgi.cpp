#include "../Http/HttpRequest/HttpRequest.hpp"
#include "../Http/HttpResponse/HttpResponse.hpp"


class Cgi {
    private:
        
};
/*
    For the enviroment i need:
        REQUEST_METHOD = (POST, GET, DELETE)
        CONTENT_LENGTH = (size of the body)
        QUERY_STRING = (the query string)
        SCRIPT_NAME = (the path to the script)
        SERVER_PROTOCOL = (HTTP/1.1)
        SERVER_NAME = (the server name)
        SERVER_PORT = (the server port)
        REMOTE_ADDR = (the client ip)
        REMOTE_PORT = (the client port)
        GATEWAY_INTERFACE = (CGI/1.1)
*/

