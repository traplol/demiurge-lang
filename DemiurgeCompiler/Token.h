#ifndef _TOKEN_H
#define _TOKEN_H

#include <string>
class Token {
public:
    Token(int type, const std::string &value, int line = -1, int column = -1)
        : _type(type)
        , _lineNumber(line)
        , _columnNumber(column)
        , _value(value) {}
    
    int Type() { return _type; }
    int Line() { return _lineNumber; }
    int Column() { return _columnNumber; }
    std::string Value() { return _value; }

    bool AsBool() { return _value != "false"; } // anything not 'false' equates to true.
    double AsDouble() { return strtod(_value.c_str(), nullptr); }
    long long int AsLong64() { return strtoll(_value.c_str(), nullptr, 10); }
    unsigned long long int AsULong64() { return strtoull(_value.c_str(), nullptr, 10); }

private:
    int _type;
    int _lineNumber;
    int _columnNumber;
    std::string _value;
};

#endif