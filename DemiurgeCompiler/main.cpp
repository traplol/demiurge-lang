#ifdef _WIN32
    #include "LibComments.h"
#else 
#error "Platform not supported."
int platform_not_supported[0];
#endif

#include "Compiler/DemiurgeCompiler.h"
#include <csignal>

DemiurgeCompiler *compilerptr;

void abortCallback(int x) {
    compilerptr->dump();
}

int main(int argc, char **argv) {

    signal(SIGABRT, abortCallback);

    DemiurgeCompiler compiler;
    compilerptr = &compiler;
    std::vector<std::string> args;
#ifdef _DEBUG
    args.push_back(argv[0]); // program name.
    args.push_back("-c");
    args.push_back("examples/unary-operators.demi");
#else
    args.assign(argv, argv + argc);
#endif
    if (compiler.UseArgs(args))
        compiler.Run();
#if defined(_WIN32) && defined(_DEBUG)
    system("PAUSE");
#endif
    return 0;
}