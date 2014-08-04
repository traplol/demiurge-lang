#ifndef _TOKEN_H
#define _TOKEN_H

#include <string>
class Token {
public:
    Token(int type, const std::string &value, int line = -1, int column = -1, bool isUnaryOperator = false)
        : _type(type)
        , _lineNumber(line)
        , _columnNumber(column)
        , _value(value)
        , _isUnaryOperator(isUnaryOperator) {}
    
    int Type() const { return _type; }
    int Line() const { return _lineNumber; }
    int Column() const { return _columnNumber; }
    std::string Value() const { return _value; }

    bool AsBool() const { return _value != "false"; } // anything not 'false' equates to true.
    double AsDouble() const { return strtod(_value.c_str(), nullptr); }
    long long int AsLong64() const { return strtoll(_value.c_str(), nullptr, 10); }
    unsigned long long int AsULong64() const { return strtoull(_value.c_str(), nullptr, 10); }
    bool IsUnaryOperator() const { return _isUnaryOperator; }

private:
    int _type;
    int _lineNumber;
    int _columnNumber;
    bool _isUnaryOperator;
    std::string _value;
};

#endif