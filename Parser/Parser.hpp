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
        static tokenVec tokens;
        static void checkSyntaxError(const tokenVec &tokens);
        static bool isSkipChar(char c);
        static bool isOwnChar(char c);
        static void setLocationsDirectives(std::vector<Block> &servers);
        static Block parseBlock(tokenVec::iterator &it, const tokenVec &tokens, bool getName);
    public:
        static void initializeTokens(std::ifstream &configFile);
        static Block parseConfigFile(void);
        static void printTokens(void);
};


#endif