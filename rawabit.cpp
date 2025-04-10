#include <string.h>
#include <iostream>

std::string uriAllowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ._~:/?#[]@!$&'()*+,;=%-";

int HexToChar(char c) {
    if (isdigit(c))
        return c - '0';
    return tolower(c) - 'a' + 10;
}

void encoding(std::string& str)
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
    std::string res = "%20%2K1%22%23%24%25%26%27%28%29%2A%2B%2C%2D%2E%2F%3A%3B%3C%3D%3E%3F%40%5B%5C%5D%5E%5F%60%7B%7C%7D%7E";
    std::cout << "Path:" << res << std::endl;
    CleanPath(res);
    std::cout << "New Path:" << CleanPath(res) << std::endl;
}