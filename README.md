# A Compiler for SL — Simple Language
Implementation of the Simple Language compiler.

This is a project developed for the subject MO403 - Implementation of Programming Languages taught by professor Tomasz Kowaltowski during the second semester of 2020.

The subject website: https://sites.google.com/unicamp.br/ic-mo403-mc900/main

## Project Description
The purpose of this task is the implementation of a compiler for SL programs, following suggestions 
presented in class. At least six files should be submitted:
* `scanner.l`
* `parser.y`
* `tree.c`
* `tree.h`
* `codegen.c`
* `codegen.h`

The first four files can be almost identical to the ones from the previous tasks, with minor adaptations 
whenever necessary. The other files should implement MEPA code generating functions. Up to four additional
`.c` and/or `.h` files may be submitted; they will be included in the compilation process (`*.c`).

Your implementation will be tested by the program `slc.c` which is provided (with `slc.h`). Notice that 
`slc.c` will import the submitted file `codegen.h`; the minimum requirement for this file is to include 
the definition:
```
void processProgram(void *p);
```

This function should be implemented in one of your `.c` files. It should generate a complete MEPA code from
the program tree pointed to by the argument `p`; its type can be declared `void` because the test program 
does not need to know the details of the tree structures. MEPA code should be written to the standard output
file.

The submission will be processed by the following sequence of Linux commands:

```
bison -d -o parser.c parser.y
flex -i -o scanner.c  scanner.l
gcc -std=c99 -pedantic -o slc *.c
    for each test NN:
        slc  < progNN.sl > progNN.mep
        mepa --limit 12000 --progfile progNN.mep < dataNN.in > progNN.res
```

(Mepa will be invoked only if the compiler `slc` did not detect any errors.)

Your program should detect lexical, syntactical and semantic errors, using "panic" mode, i.e. a simple error
message should be printed and execution aborted with exit code 0. File `slc.c` provides two error messaging
functions to be used in an obvious way. Notice that these messages will be written into the same file which
contains already a (partial) MEPA translation.

There are three sets of test files. Tests 00 to 27 cover the minimal part of SL to be implemented. Optional
tests 31 to 37 cover type declarations and arrays. Finally, optional tests 41 to 43 cover functions as 
parameters. The correct option should be chosen for submission.

(Tests 23 to 28 should produce error messages at the end of their .mep files.)

### Remarks
* File `all.zip` (see auxiliary files) provides the test program mentioned above as well as the test files
  and their results.
* Mepa files produced by the compiler do not have to be identical but their execution (when correct) should
  produce identical results.
* Maximum number of submissions is 20 — test your solutions exhaustively before submission!
* Due to the number of tests and the processing required for each of them, the results of each submission
  may take up to 30 seconds or even more, depending on the current load of the SuSy system.
* Final submission date: January 15, 2021.
* This task contributes to the final project grade as follows:
    * 30% for the implementation of the basic part of SL
    * 45% for the implementation of the basic part of SL and functions as parameters
    * 45% for the implementation of the basic part of SL and type declarations and arrays
    * 60% for the complete implementation of SL.
* A complete copy of the [Mepa interpreter](mepa.zip) is provided for download on course's page. See 
  [Mepa description](mepa.pdf) for usage instructions.

### Obs
* The `all.zip` file mentioned had the files contained in the directory `tests` and the files 
  `src/test_tree.h` and `test_tree.c`.
* This compiler is based on the scanner, parser and syntax tree implemented previously. Their projects
  can be found as follows:
    * [sl-scanner](https://github.com/sabrina-beck/sl-scanner)
    * [sl-parser](https://github.com/sabrina-beck/sl-parser)
    * [sl-tree](https://github.com/sabrina-beck/sl-tree)

## Project Execution

In order to run the program use the following command:
```
make run
```

In order to run the tests use:
```
make test
```