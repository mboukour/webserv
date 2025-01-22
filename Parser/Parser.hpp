#ifndef PARSER_HPP
#define PARSER_HPP


#include <fstream>
#include <string>
#include <vector>


enum TokenType {
    WORD,
    OPEN_BRACE = '{',
    CLOSE_BRACE = '}',
    SEMICOLON = ';'
};

typedef std::vector<std::string> stringVec;

struct Block {
    stringVec blockName;
    std::vector<stringVec > directives;
    std::vector<Block> subBlocks;
};

struct Token {
    Token(std::string &word, int type);
    std::string word;
    int type;
};

typedef std::vector<Token> tokenVec;

class Parser {
    private:
        tokenVec tokens;
        std::vector<Block> blocks;

        
        void checkSyntaxError(void);
        stringVec parseDirective(tokenVec::iterator &it);
    
        static bool isSkipChar(char c);
        static bool isOwnChar(char c);
        Block parseBlock(tokenVec::iterator &it, bool getName);
    public:
        Parser(std::ifstream &configFile);
        Block parseConfigFile(void);
        void printTokens(void);
        // static Block parseBlock(std::ifstream &configFile);

};


#endif