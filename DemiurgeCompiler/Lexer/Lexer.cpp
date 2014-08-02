#include "Lexer.h"
#include "Token.h"
#include "TokenTypes.h"

Lexer::Lexer() {
}


std::vector<Token*> Lexer::Tokenize(const std::string &filename, const std::vector<char> &sourceCode) {
    _i = 0;
    _column = 1;
    _line = 1;
    _lastChar = ' ';

    _file = filename;
    _sourceCode = sourceCode;
    std::vector<Token*> retVal;
    while (!isEOF()) {
        retVal.push_back(getNextToken());
    }
    return retVal;
}

Token *Lexer::getNextToken() {

    if (isspace(_lastChar)) // Trim the white space
        return trimWhiteSpace();
    if (_lastChar == '"') // Build a string literal
        return buildStringLiteral();
    if (isalpha(_lastChar)) // Build an identifier or reserved word
        return buildWord();
    if (isdigit(_lastChar)) // Build a number
        return buildNumber();
    if (_lastChar == '/') // Comment 
        if (peekChar() == '*') // Paragraph comment - '/*'
            return trimCommentBlock();
        else if (peekChar() == '/') // Line comment - '//'
            return trimCommentLine();
    int thisChar = _lastChar;
    _builderWord = thisChar;
    _lastChar = getNextChar();

    if (_lastChar == EOF)
        return new Token(EOF, "EOF", _line, _column);
    // Checking for two char operators.
    if (Token *tok = tryFourCharOper(thisChar))
        return tok;
    if (Token *tok = tryThreeCharOper(thisChar))
        return tok;
    if (Token *tok = tryTwoCharOper(thisChar))
        return tok;
    
    return new Token(thisChar, _builderWord, _line, _column); // test should have succeeded if we hit this.
}

Token *Lexer::tryTwoCharOper(int thisChar) {
    TokenType tokType;
    std::string operTest;
    operTest = thisChar;
    operTest += _lastChar;

    if (operTest == "++") tokType = tok_plusplus;
    else if (operTest == "--") tokType = tok_minusminus;
    else if (operTest == "+=") tokType = tok_plusequals;
    else if (operTest == "-=") tokType = tok_minusequals;
    else if (operTest == "*=") tokType = tok_multequals;
    else if (operTest == "/=") tokType = tok_divequals;
    else if (operTest == "||") tokType = tok_booleanor;
    else if (operTest == "&&") tokType = tok_booleanand;
    else if (operTest == "<=") tokType = tok_lessequal;
    else if (operTest == ">=") tokType = tok_greatequal;
    else if (operTest == "==") tokType = tok_equalequal;
    else if (operTest == "!=") tokType = tok_notequal;
    else if (operTest == "<<") tokType = tok_leftshift;
    else if (operTest == ">>") tokType = tok_rightshift;
    else if (operTest == "..") tokType = tok_dotdot;
    else return nullptr;
    _lastChar = getNextChar(); // eat the second
    return new Token(tokType, operTest, _line, _column);
}
Token *Lexer::tryThreeCharOper(int thisChar) {
    TokenType tokType;
    std::string operTest;
    operTest = thisChar;
    operTest += _lastChar;
    operTest += peekChar();

    if (operTest == "...") tokType = tok_dotdotdot;
    else if (operTest == "<<=") tokType = tok_leftshiftequal;
    else if (operTest == ">>=") tokType = tok_rightshiftequal;
    else return nullptr;

    getNextChar(); // eat the second
    _lastChar = getNextChar(); // eat the third
    return new Token(tokType, operTest, _line, _column);
}
Token *Lexer::tryFourCharOper(int thisChar) {
    //TokenType tokType;
    //std::string operTest;
    //operTest = thisChar;
    //operTest += _lastChar;
    //operTest += peekChar();
    //operTest += peekChar(1);
    return nullptr;
}

Token *Lexer::buildStringLiteral() {
    
    _builderWord = _lastChar;
    int nextChar;
    while (!isEOF()) {
        
        _lastChar = getNextChar();
        _builderWord += _lastChar;

        if (_lastChar == '\\') { // now checck 
            nextChar = getNextChar();
            _builderWord += nextChar;
            if (nextChar == '"') { continue; }         // '\"' -> escaping quote
            else if (nextChar == '\\') { continue; }   // '\\' -> escaping backslash
            else if (nextChar == 'n') { continue; }    // '\n' -> new line character
            else if (nextChar == 'r') { continue; }    // '\r' -> carriage return character
            else if (nextChar == 't') { continue; }    // '\t' -> horizontal tab character
            else if (nextChar == 'v') { continue; }    // '\v' -> vertical tab character
            else {
                // Error, unknown escape sequence.
                fprintf(stderr, "Warning: unknown escape sequence at (%d:%d).\n", _line, _column);
            }
        }
        if (_lastChar == '"')
            break;
    }
    
    replaceInBuilderWord("\\\\", "\\"); // '\\' -> '\'
    replaceInBuilderWord("\\\"", "\""); // '\"' -> '"'
    replaceInBuilderWord("\\n", "\n");  // '\t' -> '<new line>'
    replaceInBuilderWord("\\r", "\r");  // '\t' -> '<carriage return>'
    replaceInBuilderWord("\\t", "\t");  // '\t' -> '<horizontal tab>'
    replaceInBuilderWord("\\v", "\v");  // '\t' -> '<vertical tab>'
    
    _builderWord.replace(0, 1, ""); // remove opening quote
    _builderWord.replace(_builderWord.length() - 1, 1, ""); // remove closing quote

    _lastChar = getNextChar(); // don't try to parse the closing quote as the start of another string.
    return new Token(tok_string, _builderWord, _line, _column);
}

void Lexer::replaceInBuilderWord(std::string find, std::string replace) {
    size_t index = 0;
    while (true) {
        index = _builderWord.find(find.c_str());
        if (index == std::string::npos) break;
        _builderWord.replace(index, find.length(), replace.c_str());
        index += replace.length();
    }
}

Token *Lexer::buildNumber() {
    _builderWord = _lastChar; // first char of number
    _lastChar = getNextChar();
    int decimalCounter = 0; // don't want to have more than 1 decimal in our number
    while (isdigit(_lastChar) || _lastChar == '.') {
        if (_lastChar == '.' && decimalCounter > 0) { // ignore extra decimals
            _lastChar = getNextChar();
            continue; // '1.23..4..5.6' -> '1.23456'
        }
        if (_lastChar == '.') { // start counting our decimals
            decimalCounter++;
        }
        _builderWord += _lastChar;// build the number
        _lastChar = getNextChar();
    }

    return new Token(tok_number, _builderWord, _line, _column);
}
Token *Lexer::buildWord() {
    _builderWord = _lastChar; // first char of word
    _lastChar = getNextChar();
    while (isalnum(_lastChar)) { // build word
        _builderWord += _lastChar;
        _lastChar = getNextChar();
    }

    tag_TokenType tokType;
    if (_builderWord == "var") tokType = tok_var;
    else if (_builderWord == "func") tokType = tok_func;
    else if (_builderWord == "return") tokType = tok_return;
    else if (_builderWord == "extern") tokType = tok_extern;
    else if (_builderWord == "if") tokType = tok_if;
    else if (_builderWord == "else") tokType = tok_else;
    else if (_builderWord == "true") tokType = tok_bool;
    else if (_builderWord == "false") tokType = tok_bool;
    else if (_builderWord == "while") tokType = tok_while;
    else if (_builderWord == "for") tokType = tok_for;
    else if (_builderWord == "void") tokType = tok_typevoid;
    else if (_builderWord == "int") tokType = tok_typeint;
    else if (_builderWord == "string") tokType = tok_typestring;
    else if (_builderWord == "double") tokType = tok_typedouble;
    else if (_builderWord == "bool") tokType = tok_typebool;
    else tokType = tok_identifier;

    return new Token(tokType, _builderWord, _line, _column);
}
Token *Lexer::trimWhiteSpace() {
    while (isspace(_lastChar)) {
        if (_lastChar == '\n') {
            _line++;
            _column = -1;
            if (_fromStdIn) {
                _lastChar = ' ';
                return new Token(tok_special_eol, "SPECIAL_EOL_TOKEN", _line, _column);
            }
        }
        _lastChar = getNextChar();
    }

    return getNextToken();
}
Token *Lexer::trimCommentBlock() {
    while (true) {
        _lastChar = getNextChar();
        if (_lastChar == '\n') _line++;
        if (_lastChar == EOF || (_lastChar == '*' && peekChar() == '/')) {
            _lastChar = getNextChar(); // eat '*'
            _lastChar = getNextChar(); // eat '/'
            break;
        }
    }

    return getNextToken();
}
Token *Lexer::trimCommentLine() {
    do { // eats the rest of the line
        _lastChar = getNextChar();
    } while (_lastChar != EOF && _lastChar != '\n' && _lastChar != '\r');

    return getNextToken();
}


int Lexer::ipp() {
    _i++;
    _column++;
    return _i - 1;
}
int Lexer::ppi() {
    _column++;
    return ++_i;
}
bool Lexer::isEOF(int i) {
    if (i == -1) {
        i = _i;
    }
    return i >= _sourceCode.size();
}
int Lexer::peekChar(int ahead) {
    if (_fromStdIn) return getchar();
    if (isEOF()) return -1;
    return _sourceCode[_i + ahead];
}
int Lexer::getNextChar() {
    return peekChar(ipp() - _i);
}