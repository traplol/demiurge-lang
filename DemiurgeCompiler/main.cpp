#ifdef _WIN32
    #include "LibComments.h"
#else 
#error "Platform not supported."
int platform_not_supported[0];
#endif

#include "Compiler/DemiurgeCompiler.h"
#include <csignal>
#include <ctime>

DemiurgeCompiler *compilerptr;

void abortCallback(int x) {
    compilerptr->dump();
}

int main(int argc, char **argv) {
    srand(time(0));
    signal(SIGABRT, abortCallback);

    DemiurgeCompiler compiler;
    compilerptr = &compiler;
    std::vector<std::string> args;
#ifdef _DEBUG
    args.push_back(argv[0]); // program name.
    args.push_back("-c");
    //args.push_back("examples/test.demi");
    //args.push_back("examples/add.demi");
    //args.push_back("examples/arrays.demi");
    //args.push_back("examples/fibR.demi");
    //args.push_back("examples/fib.demi");
    //args.push_back("examples/fizzbuzz.demi");
    //args.push_back("examples/gcd.demi");
    //args.push_back("examples/ifelse.demi");
    //args.push_back("examples/nesting.demi");
    //args.push_back("examples/new-bool.demi");
    //args.push_back("examples/scope.demi");
    //args.push_back("examples/strings.demi");
    //args.push_back("examples/unary-operators.demi");
    //args.push_back("examples/variables.demi");
    //args.push_back("examples/void-return.demi");
    //args.push_back("examples/whileloops.demi");
    args.push_back("examples/for-loops.demi");
    //args.push_back("examples/tests/arithmetic.demi");
    //args.push_back("examples/tests/bitwise.demi");
    //args.push_back("examples/tests/complex-boolean.demi");
    //args.push_back("examples/tests/simple-boolean.demi");
    //args.push_back("examples/all-number-types.demi");

#else
    args.assign(argv, argv + argc);
#endif
    if (compiler.UseArgs(args)) {
        compiler.Run();
    }
#if defined(_WIN32) && defined(_DEBUG)
    system("PAUSE");
#endif
    return 0;
}