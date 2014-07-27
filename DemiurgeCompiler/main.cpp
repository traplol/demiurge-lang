#ifdef _WIN32
    #include "LibComments.h"
#else 
#error "Platform not supported."
int platform_not_supported[0];
#endif

#include "DemiurgeCompiler.h"

int main(int argc, char **argv) {
    
    DemiurgeCompiler compiler;
    std::vector<std::string> args;
#ifdef _DEBUG
    args.push_back(argv[0]); // program name.
    args.push_back("-c");
    args.push_back("examples/ifelse.demi");
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