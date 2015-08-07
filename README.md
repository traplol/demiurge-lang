demiurge-lang
=============


The Demiurge Programming Language Compiler
  
``` 
demiurge  (ˈdemēˌərj) 

— n
1.	a. (in the philosophy of Plato) the creator of the universe
 	b. (in Gnostic and some other philosophies) the creator of the universe, 
 	    supernatural but subordinate to the Supreme Being
```

This project is a parser/frontend for a simple programming language with [LLVM](http://llvm.org/) as the backend.

I started this project during the summer of 2014 and had a lot of fun designing and implementing this simple
programming language. This is mostly an educational project that I have decided to port to *nix rather than 
work only within my defunct visual studio workspace. There are still probably lots of bugs within this project
and it shouldn't be taken too seriously, but I do plan to continue playing with it and implementing features 
in [TODO.md](TODO.md). Originally, this project used LLVM 3.4 and I have started work on bringing it up to 
LLVM 3.6.2 and might keep it updated with future releases of LLVM.

Dependencies
============
- LLVM Development Toolchain
  + [LLVM 3.6.2](http://llvm.org/releases/download.html#3.6.2)

Install
=======
Download the [LLVM 3.6.2 Source code](http://llvm.org/releases/3.6.2/llvm-3.6.2.src.tar.xz) and run: 

```
$ tar -xJf llvm-3.6.2.src.tar.xz
$ cd llvm-3.6.2.src
$ ./configure
$ # this will take a while and root may be required if installing to /usr
$ make && make install prefix=INSTALL_DIR 
$ # if installed somewhere other than `/usr' then add INSTALL_DIR/ to your path so `llvm-config'
$ # may resolve paths.
$ cd DEMIURGE_LANG_PATH
$ make
$ cd bin
$ ./demi -c ../examples/tests/complex-boolean.demi
```
