#ifndef _DEMIURGE_COMPILER_H
#define _DEMIURGE_COMPILER_H

/*
 *      The Demiurge Programming Language Compiler
 *        
 *      Demiurge
 *      - n
 *      1.  a. (in the philosophy of Plato) the creator of the universe
 *          b. (in Gnostic and some other philosophies) the creator of 
 *              the universe, supernatural but subordinate to the Supreme Being
 *              
 *
 *      Written and developed by: 
 *          Max Mickey - max.mickey1991@gmail.com
 *          2014
 */

#include <vector>
#include <string>
#include <map>

class Lexer;
class Parser;
class CodeGenerator;

class DemiurgeCompiler {
public:
    DemiurgeCompiler();
    ~DemiurgeCompiler();
    bool UseArgs(const std::vector<std::string> &args);
    void Run();

    void dump();
public:
    
    void printHelpMessage();
    void printCompilerInformationMessage();
    void getSource(std::string filepath);

    bool verifyArgs();
    bool setStateVars();

    std::vector<std::string> _args;

    Lexer *_lexer;
    Parser *_parser;
    CodeGenerator *_codeGenerator;

    /* compiler state variables */
    bool _canRun;
    bool _isInInteractiveMode;
    bool _outputLlvmAsm;
    bool _stderrDump;
    std::map<std::string, std::vector<char> > _sourceFiles;
};

#endif