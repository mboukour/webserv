#include "Login.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "../../Server/ServerManager/ServerManager.hpp"
#include <csignal>
#include <cstddef>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>

std::string Login::generateUniqueSessionID(std::map<std::string, std::string>& userCreds) {
    while(true) {
        std::srand(static_cast<unsigned int>(std::time(NULL)));
        int randomPart1 = std::rand();
        int randomPart2 = std::rand();
        struct timeval tv;
        gettimeofday(&tv, NULL);
        long long millis = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        std::stringstream ss;
        ss << std::hex << millis << std::setw(8) << std::setfill('0') << randomPart1 << randomPart2;
        if (userCreds.find(ss.str()) == userCreds.end())
            return ss.str();
    }
}

std::string Login::getLogin(const HttpRequest &request, std::map<std::string, std::string> &userCreds) {

    std::string sessionId;
    try {
        sessionId = request.getCookie("sessionId");
    } catch(std::out_of_range &) {} // keep it empty
    std::stringstream html;

    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "    <title>Session Test</title>\n"
         << "    <style>\n"
         << "        body { font-family: Arial, sans-serif; margin: 40px auto; max-width: 600px; text-align: center; }\n"
         << "        .container { padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }\n"
         << "        input { padding: 8px; margin: 10px 0; width: 200px; }\n"
         << "        button { background: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }\n"
         << "    </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "    <div class='container'>\n";

    if (!sessionId.empty() && userCreds.find(sessionId) != userCreds.end()) {
        html << "        <h2>Welcome back, " << userCreds[sessionId] << "! :)</h2>\n"
             << "        <p>We remembered your name!</p>\n"
             << "        <button id='forgetBtn' type='button'>Forget me</button>\n"
             << "        <script>\n"
             << "            document.getElementById('forgetBtn').addEventListener('click', function() {\n"
             << "                fetch('/session-test', {\n"
             << "                    method: 'DELETE',\n"
             << "                    credentials: 'same-origin'\n"
             << "                }).then(function() {\n"
             << "                    window.location.reload();\n"
             << "                });\n"
             << "            });\n"
             << "        </script>\n";
    } else {
        html << "        <h2>Hi there! What's your name?</h2>\n"
             << "        <p>We'll remember it for you :)</p>\n"
             << "        <form action='/session-test' method='POST'>\n"
             << "            <input type='text' name='username' placeholder='Enter your name' required>\n"
             << "            <br>\n"
             << "            <button type='submit'>Remember me</button>\n"
             << "        </form>\n";
    }

    html << "    </div>\n"
         << "</body>\n"
         << "</html>\n";

    return html.str();
}

std::string Login::deleteLogin(const HttpRequest &request, std::map<std::string, std::string> &userCreds) {
    std::string sessionId;
    try {
        sessionId = request.getCookie("sessionId");
    } catch(std::out_of_range &) {
        throw HttpErrorException(BAD_REQUEST, request, "No session ID to remove");
    }
    userCreds.erase(sessionId);
    std::stringstream html;
    html << "<!DOCTYPE html>\n"
        << "<html>\n"
        << "<head>\n"
        << "    <title>Session Test</title>\n"
        << "    <style>\n"
        << "        body { font-family: Arial, sans-serif; margin: 40px auto; max-width: 600px; text-align: center; }\n"
        << "        .container { padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }\n"
        << "        input { padding: 8px; margin: 10px 0; width: 200px; }\n"
        << "        button { background: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }\n"
        << "    </style>\n"
        << "</head>\n"
        << "<body>\n"
        << "    <div class='container'>\n"
        << "        <h2>Hi there! What's your name?</h2>\n"
        << "        <p>We'll remember it for you :)</p>\n"
        << "        <form action='/session-test' method='POST'>\n"
        << "            <input type='text' name='username' placeholder='Enter your name' required>\n"
        << "            <br>\n"
        << "            <button type='submit'>Remember me</button>\n"
        << "        </form>\n"
        << "    </div>\n"
        << "</body>\n"
        << "</html>\n";
    return html.str();
}

std::string Login::postLogin(const HttpRequest &request, HttpResponse &response ,std::map<std::string, std::string> &userCreds) {
    const std::string &body = request.getBody();


    if (body.find("username=") != 0)
        throw HttpErrorException(BAD_REQUEST, request, "Invalid login body");
    size_t pos = body.find("=");
    std::string userName = body.substr(pos + 1);
    if (userName.size() > NAME_MAX_LENGTH)
        throw HttpErrorException(BAD_REQUEST, request, "Name exceeded max length"); // we can change this by showing custom error for login??

    std::string sessionId = generateUniqueSessionID(userCreds);
    userCreds[sessionId] = userName;
    response.addCookie("sessionId", sessionId, "");
    std::stringstream html;

    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "    <title>Session Test</title>\n"
         << "    <style>\n"
         << "        body { font-family: Arial, sans-serif; margin: 40px auto; max-width: 600px; text-align: center; }\n"
         << "        .container { padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }\n"
         << "        input { padding: 8px; margin: 10px 0; width: 200px; }\n"
         << "        button { background: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }\n"
         << "    </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "    <div class='container'>\n"
         << "       <h2>Welcome, " << userName << "! :)</h2>\n"
         << "        <p>Refresh to see if we will remember your name!</p>\n"
         << "    </div>\n"
         << "</body>\n"
         << "</html>\n";
    return html.str();
}


void Login::respondToLogin(const HttpRequest &request, std::map<std::string, std::string> &userCreds, int clientFd) {
    const std::string &method = request.getMethod();
    HttpResponse response(request.getVersion(), 200, "OK", "");
    std::string html;
    if (method == "GET")
        html =  getLogin(request, userCreds);
    else if (method == "POST")
        html = postLogin(request, response, userCreds);
    else 
        html = deleteLogin(request, userCreds);
    response.setBody(html);
    std::string responseStr = response.toString();
    ServerManager::sendString(responseStr, clientFd);
}