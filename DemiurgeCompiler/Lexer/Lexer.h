#ifndef _LEXER_H
#define _LEXER_H

#include <vector>
#include <string>


class Token;

class Lexer {
public:
    Lexer();

    std::vector<Token*> Tokenize(const std::string &filePath, const std::vector<char> &sourceCode);

private:
    std::string _file;
    std::vector<char> _sourceCode;
    
    int _i;
    bool _fromStdIn;
    int _line;
    int _column;
    int  _lastChar;
    
    std::string _builderWord;

    int ipp();
    int ppi();
    bool isEOF(int i = -1);
    int peekChar(int ahead = 0);
    int getNextChar();
    void replaceInBuilderWord(std::string find, std::string replace);

    Token *getNextToken();

    Token *tryTwoCharOper(int thisChar);
    Token *tryThreeCharOper(int thisChar);
    Token *tryFourCharOper(int thisChar);

    Token *buildStringLiteral();
    Token *buildNumber();
    Token *buildWord();

    Token *trimWhiteSpace();
    Token *trimCommentBlock();
    Token *trimCommentLine();
};

#endif