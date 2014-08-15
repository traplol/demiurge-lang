### Refactor
  - Namespacing
  - Parser

### Arrays
  - Array instantiation 
    + Dynamic
    + Compile time bound checking for unsafe static arrays
        - Warn, don't error

### Garbage collection
  - Compile time GC
  - Runtime GC
  - Maybe user invoked GC?

### Structures
  - Objects
  - PODS (Plain Old Data Structure)
  - Safe built-in types
    + Safe Arrays/Strings ( bounds checking )
    + Vectors

### Operators
  - User defined operators and operator overloading
  - '..' - range operator

### Control Flow
  - ~~For loop~~
    + ~~for (var i = 0; i < 10; ++i) {}   - Standard for loop.~~
    + for (i = 0 in 0..10;){}  - essentially the same as the line above
  - Switch case
  - continue keyword
  - break keyword
  
