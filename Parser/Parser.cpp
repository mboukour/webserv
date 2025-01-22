#include "Parser.hpp"
#include <cctype>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
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
    Block globalBlock;
    globalBlock.blockName.push_back("Global Block");
    globalBlock.subBlocks.push_back(parseBlock(it, true));
    while (it != this->tokens.end())
    {
        while(it->type != CLOSE_BRACE)
            it--;
        it++;
        globalBlock.subBlocks.push_back(parseBlock(it, true));
    }
    return globalBlock;
}

Block Parser::parseBlock(tokenVec::iterator &it, bool getName)
{
    Block result;

    if (getName)
    {
        for(; it->type != OPEN_BRACE; it++)
            result.blockName.push_back(it->word);
        it++; // Skip open brace
    }

    for (; it != this->tokens.end() && it->type != CLOSE_BRACE; )
    {
        stringVec vec;
        for(; it->type == WORD; it++) vec.push_back(it->word);
        if (it->type == SEMICOLON) {it++; result.directives.push_back(vec) ; continue;} // skip semicolon for next directive
        if (it->type == OPEN_BRACE) {
            it++; // skip open brace
            Block subBlock = parseBlock(it, false);
            subBlock.blockName = vec;
            result.subBlocks.push_back(subBlock);
        }
    }
    it++; // skip this block's close brace
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