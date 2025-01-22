#include "Parser.hpp"
#include <cctype>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <iostream>

Token::Token(std::string &word, int type): word(word), type(type) {}

Parser::Parser(std::ifstream &configFile)
{
    std::string configString;
    std::stringstream ss;

    ss << configFile.rdbuf();
    configString = ss.str();

    for (size_t i = 0; i < configString.length(); i++) {
        if (isOwnChar(configString[i]))
        {
            std::string push(1, configString[i]); 
            Token newToken(push, configString[i]);
            this->tokens.push_back(newToken);
            i++;
        }
        else if (!isSkipChar(configString[i]))
        {
            std::stringstream wordStream;
            while (i < configString.length() 
                && !isSkipChar(configString[i]) 
                && !isOwnChar(configString[i]))
            {
                wordStream << configString[i];
                i++;
            }
            i--;
            std::string str = wordStream.str();
            Token newToken(str, WORD);
            this->tokens.push_back(newToken);
        }
    }
    checkSyntaxError();
}


void Parser::checkSyntaxError(void)
{
    int braceCount = 0;
    for (tokenVec::iterator it = this->tokens.begin(); it != this->tokens.end(); it++)
    {
        if (it->type == OPEN_BRACE)
            braceCount++;
        else if (it->type == CLOSE_BRACE)
            braceCount--;
        else if (it->type == SEMICOLON)
        {
            if (it != this->tokens.begin() && (it - 1)->type != WORD)
                throw std::logic_error("Empty directive not accepted");
        }
    }
    if (braceCount)
        throw std::logic_error("Unclosed braces");
    for (tokenVec::iterator it = this->tokens.begin(); it != this->tokens.end(); it++)
        if (it->word == "")
            throw std::logic_error("EMPTY STRING");
}

bool Parser::isSkipChar(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

bool Parser::isOwnChar(char c)
{
    return (c == '{' || c == '}' || c == ';');
}


Block Parser::parseConfigFile(void)
{
    tokenVec::iterator it = this->tokens.begin();
    return (parseBlock(it));
}

stringVec Parser::parseDirective(tokenVec::iterator &it)
{
    stringVec vec;
    for (tokenVec::iterator check = it; check != this->tokens.end(); check++)
    {
        if (check->type == SEMICOLON)
            break;
        else if (check->type == OPEN_BRACE || check->type == CLOSE_BRACE)
            return vec;
    }
    while(it != this->tokens.end() && it->type != SEMICOLON)
    {
        vec.push_back(it->word);
        it++;
    }
    return vec;
}
Block Parser::parseBlock(tokenVec::iterator &it)
{
    Block result;

    if (it->type == CLOSE_BRACE)
        return result;
    while (it != this->tokens.end() && it->type != OPEN_BRACE)
    {
        result.blockName.push_back(it->word);
        it++;
    }


    if (it == this->tokens.end() || (it + 1) == this->tokens.end())
        return result;

    if (it->type == OPEN_BRACE)
        it++;
    while(it != this->tokens.end() && it->type == CLOSE_BRACE)
        it++;

    stringVec res;
    while (!(res = parseDirective(it)).empty()) {
        if (it == this->tokens.end())
            return result;
        result.directives.push_back(res);
        it++;
    }
     for (tokenVec::iterator check = it; check != this->tokens.end(); check++)
    {
        if (check->type == OPEN_BRACE)
            break;
        else if (check->type == CLOSE_BRACE)
            return result;
    }
    Block sub;
    while (!(sub = parseBlock(it)).blockName.empty()) {
        result.subBlocks.push_back(sub);
        if (it != this->tokens.end() && it->type == CLOSE_BRACE)
        {
            it++;
            if (it == this->tokens.end())
                return result;
        }
    }

    if (it != this->tokens.end() && it->type == CLOSE_BRACE)
    {
        it++;
        if (it == this->tokens.end())
            return result;
        return (result);
    }

    // while (!(res = parseDirective(it)).empty()) {
    //     result.directives.push_back(res);
    // }


    // if (it != this->tokens.end() && it->type == CLOSE_BRACE)
    // {
    //     it++;
    // }

    return result;
}

// DEBUG
void Parser::printTokens(void)
{
    std::string type;
    for(std::vector<Token>::iterator it = this->tokens.begin(); it !=  this->tokens.end(); it++)
    {
        if (it->type == WORD)
            type = "WORD";
        else if (it->type == OPEN_BRACE)
            type = "OPEN_BRACE";
        else if (it->type == CLOSE_BRACE)
            type = "CLOSE_BRACE";
        else if (it->type == SEMICOLON)
            type = "SEMICOLON";
        std::cout << "Word: " << it->word << " Type: " << type << '\n';
    }
}