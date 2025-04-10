#include <string.h>
#include <iostream>

std::string uriAllowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ._~:/?#[]@!$&'()*+,;=%-";


void encoding(std::string& newPath) {
    for (size_t i = 0; i < newPath.length() - 2; ) {
        if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == '0')
            newPath.replace(i, 3, " ");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == '1')
            newPath.replace(i, 3, "!");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == '2') {
            newPath[i] = '"';
            newPath.erase(i + 1, 2);
        }
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == '3')
            newPath.replace(i, 3, "#");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == '4')
            newPath.replace(i, 3, "$");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == '5')
            newPath.replace(i, 3, "%");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == '6')
            newPath.replace(i, 3, "&");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == '7')
            newPath.replace(i, 3, "'");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == '8')
            newPath.replace(i, 3, "(");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == '9')
            newPath.replace(i, 3, ")");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == 'A')
            newPath.replace(i, 3, "*");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == 'B')
            newPath.replace(i, 3, "+");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == 'C')
            newPath.replace(i, 3, ",");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == 'D')
            newPath.replace(i, 3, "-");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == 'E')
            newPath.replace(i, 3, ".");
        else if (newPath[i] == '%' && newPath[i + 1] == '2' && newPath[i + 2] == 'F')
            newPath.replace(i, 3, "/");
        else if (newPath[i] == '%' && newPath[i + 1] == '3' && newPath[i + 2] == 'A')
            newPath.replace(i, 3, ":");
        else if (newPath[i] == '%' && newPath[i + 1] == '3' && newPath[i + 2] == 'B')
            newPath.replace(i, 3, ";");
        else if (newPath[i] == '%' && newPath[i + 1] == '3' && newPath[i + 2] == 'C')
            newPath.replace(i, 3, "<");
        else if (newPath[i] == '%' && newPath[i + 1] == '3' && newPath[i + 2] == 'D')
            newPath.replace(i, 3, "=");
        else if (newPath[i] == '%' && newPath[i + 1] == '3' && newPath[i + 2] == 'E')
            newPath.replace(i, 3, ">");
        else if (newPath[i] == '%' && newPath[i + 1] == '3' && newPath[i + 2] == 'F')
            newPath.replace(i, 3, "?");
        else if (newPath[i] == '%' && newPath[i + 1] == '4' && newPath[i + 2] == '0')
            newPath.replace(i, 3, "@");
        else if (newPath[i] == '%' && newPath[i + 1] == '5' && newPath[i + 2] == 'B')
            newPath.replace(i, 3, "[");
        else if (newPath[i] == '%' && newPath[i + 1] == '5' && newPath[i + 2] == 'C') {
            newPath[i] = '\\';
            newPath.erase(i + 1, 2);
        }
        else if (newPath[i] == '%' && newPath[i + 1] == '5' && newPath[i + 2] == 'D')
            newPath.replace(i, 3, "]");
        else if (newPath[i] == '%' && newPath[i + 1] == '5' && newPath[i + 2] == 'E')
            newPath.replace(i, 3, "^");
        else if (newPath[i] == '%' && newPath[i + 1] == '5' && newPath[i + 2] == 'F')
            newPath.replace(i, 3, "");
        else if (newPath[i] == '%' && newPath[i + 1] == '6' && newPath[i + 2] == '0')
            newPath.replace(i, 3, "`");
        else if (newPath[i] == '%' && newPath[i + 1] == '7' && newPath[i + 2] == 'B')
            newPath.replace(i, 3, "{");
        else if (newPath[i] == '%' && newPath[i + 1] == '7' && newPath[i + 2] == 'C')
            newPath.replace(i, 3, "|");
        else if (newPath[i] == '%' && newPath[i + 1] == '7' && newPath[i + 2] == 'D')
            newPath.replace(i, 3, "}");
        else if (newPath[i] == '%' && newPath[i + 1] == '7' && newPath[i + 2] == 'E')
            newPath.replace(i, 3, "~");
        else
            i++;
    }
}
std::string CleanPath(std::string path) {
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

int main() {
    std::string res = "http://localhost:3010/[Animerco.com]%20DS%20S3%20-%2001.mp4";
    std::cout << "Path:" << res << std::endl;
    CleanPath(res);
    std::cout << "New Path:" << CleanPath(res) << std::endl;
}