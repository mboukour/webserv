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
#include <iostream>

Token::Token(std::string &word, int type): word(word), type(type) {}

tokenVec Parser::tokens;

void Parser::initializeTokens(std::ifstream &configFile)
{
    std::string configString;
    std::stringstream ss;

    ss << configFile.rdbuf();
    configString = ss.str();
    if (configString.empty())
        throw std::logic_error("Empty file as a config file");
    for (size_t i = 0; i < configString.length(); i++) {
        if (isOwnChar(configString[i]))
        {
            std::string push(1, configString[i]); 
            Token newToken(push, configString[i]);
            tokens.push_back(newToken);
        }
        else if (configString[i] == '#')
            for (; configString[i] && configString[i] != '\n'; i++);
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
            tokens.push_back(newToken);
        }
    }
    checkSyntaxError(tokens);
}

void Parser::checkSyntaxError(const tokenVec &tokens)
{
    int braceCount = 0;
    bool foundBraces = false;
    int lastType = 0;
    for (tokenVec::const_iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        if (it->type == OPEN_BRACE)
        {
            foundBraces = true;
            braceCount++;
        }
        else if (it->type == CLOSE_BRACE)
        {
            if (lastType == OPEN_BRACE)
                throw std::logic_error("Empty blocks are not allowed.");
            braceCount--;
        }
            
        else if (it->type == SEMICOLON)
        {
            if (it != tokens.begin() && (it - 1)->type != WORD)
                throw std::logic_error("Empty directive not accepted");
        }
        lastType = it->type;
    }
    if (!foundBraces)
        throw std::logic_error("Unrecongnised config file structure");
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

Block Parser::parseConfigFile()
{
    tokenVec::iterator it = const_cast<tokenVec&>(tokens).begin();
    Block globalBlock;
    globalBlock.blockName.push_back("Global Block");
    globalBlock.subBlocks.push_back(parseBlock(it, tokens, true));
    while (it != tokens.end())
    {
        while(it->type != CLOSE_BRACE)
            it--;
        it++;
        globalBlock.subBlocks.push_back(parseBlock(it, tokens, true));
    }
    setLocationsDirectives(globalBlock.subBlocks);
    return globalBlock;
}

// Need to figure out a way to inherit error pages.
void Parser::setLocationsDirectives(std::vector<Block> &servers) {
    for (std::vector<Block>::iterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt) {
        const std::vector<stringVec> &serverDirectives = serverIt->directives;
        for (std::vector<Block>::iterator locationIt = serverIt->subBlocks.begin(); locationIt != serverIt->subBlocks.end(); ++locationIt) {
            std::vector<stringVec> &locationDirectives = locationIt->directives;
            for (std::vector<stringVec>::const_iterator serverDirIt = serverDirectives.begin(); serverDirIt != serverDirectives.end(); ++serverDirIt) {
                const std::string &serverDirective = (*serverDirIt)[0];
                if (serverDirective == "error_page")
                    continue;
                bool foundDirective = false;
                for (std::vector<stringVec>::const_iterator locationDirIt = locationDirectives.begin(); locationDirIt != locationDirectives.end(); ++locationDirIt) {
                    const std::string &locationDirective = (*locationDirIt)[0];
                    if (locationDirective == serverDirective) {
                        foundDirective = true;
                        break;
                    }
                }
                if (!foundDirective) {
                    locationDirectives.push_back(*serverDirIt);
                }
            }
        }
    }
}

Block Parser::parseBlock(tokenVec::iterator &it, const tokenVec &tokens, bool getName)
{
    Block result;

    if (getName)
    {
        for(; it->type != OPEN_BRACE; it++)
            result.blockName.push_back(it->word);
        it++; // Skip open brace
    }

    for (; it != tokens.end() && it->type != CLOSE_BRACE; )
    {
        stringVec vec;
        for(; it->type == WORD; it++) vec.push_back(it->word);
        if (it->type == SEMICOLON) {it++; result.directives.push_back(vec) ; continue;} // skip semicolon for next directive
        if (it->type == OPEN_BRACE) {
            it++; // skip open brace
            Block subBlock = parseBlock(it, tokens, false);
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
    for(std::vector<Token>::const_iterator it = tokens.begin(); it != tokens.end(); it++)
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