#include "DemiurgeCompiler.h"

#include <stdio.h>
#include <fstream>

#include "Token.h"
#include "ASTNodes.h"
#include "Lexer.h"
#include "Parser.h"
#include "CodeGenerator.h"
#include "TreeContainer.h"

DemiurgeCompiler::DemiurgeCompiler() {
    _lexer = new Lexer();
    _parser = new Parser();
    _codeGenerator = new CodeGenerator();
}

DemiurgeCompiler::~DemiurgeCompiler() {
    delete _lexer;
    delete _parser;
    delete _codeGenerator;
}

bool DemiurgeCompiler::UseArgs(const std::vector<std::string> &args) {
    _args = args;
    return _canRun = (verifyArgs() && setStateVars());
}

void DemiurgeCompiler::Run() {
    if (!_canRun)
        return;

    if (!_isInInteractiveMode) {
        auto iter = _sourceFiles.begin();
        auto end = _sourceFiles.end();
        for (; iter != end; ++iter) {
            std::vector<Token*> tokens = _lexer->Tokenize(iter->first, iter->second);
            TreeContainer *trees = _parser->ParseTrees(tokens);
            /*std::vector<llvm::Module*> modules = */ _codeGenerator->GenerateCode(trees);
            _codeGenerator->DumpLastModule();
            _codeGenerator->CacheLastModule();
        }
    }
    else {
        // TODO: Interactive mode setup
    }
    // TODO: Dump and link modules where necessary.
}

bool DemiurgeCompiler::verifyArgs() {
    // This function may be unnecessary
    // TODO: Verify passed arguments.
    return true;
}

bool DemiurgeCompiler::setStateVars() {
    for (int i = 1, e = _args.size(); i < e; ++i) { // start at 1 because [0] is the string of what invoked the program.
        std::string str(_args[i]);
        if (str == "--interactive" || str == "-I") {
            _isInInteractiveMode = true;
        }
        else if (str == "--compile" || str == "-c" ) {
            if (i + 1 >= e) {
                fprintf(stderr, "'%s' flag used with no input file.\n", str.c_str());
                return false;
            }
            getSource(_args[++i]);
        }
        else if (str == "--info") {
            printCompilerInformationMessage();
            return false;
        }
        else if (str == "--help" || str == "-h") {
            printHelpMessage();
            return false;
        }
        else {
            fprintf(stderr, "Unknown argument '%s' used, try -h or --help for usage.\n", str.c_str());
            return false;
        }
    }
    return true;
}

void DemiurgeCompiler::getSource(std::string filepath) {
    std::ifstream file(filepath, std::ifstream::binary);
    if (file) {
        file.seekg(0, file.end);
        unsigned len = 2 + file.tellg();
        char *buf = new char[len];
        file.seekg(0, file.beg);
        file.read(buf, len);
        file.close();
        buf[len - 2] = '\n'; // EOF new line hack 
        _sourceFiles[filepath] = std::vector<char>(buf, buf + len);
        delete[] buf; // done with the buffer
    }
    else {
        fprintf(stderr, "Cannot open file '%s' for parsing.\n", filepath.c_str());
    }
}

void DemiurgeCompiler::printHelpMessage() {
    fprintf(stderr, "%s [opts]\n", _args[0].c_str());
    fprintf(stderr, "  Options:\n");
    fprintf(stderr, "    -c --compile [file]: uses [file] as the input file to compile.\n");
    fprintf(stderr, "    -I --interactive   : runs the compiler in interactive mode.\n");
    fprintf(stderr, "    -h --help          : prints this message.\n");
    fprintf(stderr, "    --info             : prints compiler information.\n");
    fprintf(stderr, "\n");
}

void DemiurgeCompiler::printCompilerInformationMessage() {
    fprintf(stderr, "The Demiurge Programming Language Compiler\n");
    fprintf(stderr, "\n");
    fprintf(stderr, " Demiurge\n");
    fprintf(stderr, " - n\n");
    fprintf(stderr, " 1.  a. (in the philosophy of Plato) the creator of the universe\n");
    fprintf(stderr, "     b. (in Gnostic and some other philosophies) the creator of\n");
    fprintf(stderr, "        the universe, supernatural but subordinate to the Supreme Being\n");
    fprintf(stderr, "\n");
    fprintf(stderr, " Written and developed by :\n");
    fprintf(stderr, "     Max Mickey - max.mickey1991@gmail.com\n");
    fprintf(stderr, "\n");
    fprintf(stderr, " Version 0.0.1 - 2014\n");
}