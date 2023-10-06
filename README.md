# 8CC C-Compiler
Custom fork of the 8CC (Toy Compiler) which Sleirs modified to work with PS4.. Maintained by me...

8cc is a compiler for the C programming language. Its primary aim is to support all C11 language features while maintaining a small and straightforward codebase.

The compiler can compile itself, allowing you to view its code both as an implementation of the C language and as an example of what this compiler can handle.

8cc's source code is meticulously written to be as concise and easy to read as possible. This makes it excellent study material for learning various techniques used in compilers. You'll find that the lexer, preprocessor, and parser are already valuable resources for understanding how C source code is processed at each stage.

It's important to note that 8cc is not an optimizing compiler. Generated code is typically 2x or more slower than that produced by GCC. I plan to implement a reasonable level of optimization in the future.

Currently, 8cc supports x86-64 Linux exclusively. I do not have immediate plans to make it portable until I address all known miscompilations and introduce an optimization pass. As of 2015, I'm using Ubuntu 14 as my development platform. However, it should work on other x86-64 Linux distributions.

Please keep in mind that this compiler may not meet all your expectations. If you attempt to compile a program other than the compiler itself, you may encounter compile errors or miscompilations. This project is primarily the work of a single individual, and I've devoted only a few months of my spare time to it thus far.
