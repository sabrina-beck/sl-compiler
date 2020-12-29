# A Tree Builder for SL — Simple Language
Implementation of a Syntax Tree (from syntax analysis phase) for the Simple Language compiler.

This is a project developed for the subject MO403 - Implementation of Programming Languages taught by professor Tomasz Kowaltowski during the second semester of 2020.

The subject website: https://sites.google.com/unicamp.br/ic-mo403-mc900/main

## Project Description
The purpose of this task is the implementation of tree representation for SL programs, following suggestions presented in class. At least four files should be submitted:
* `scanner.l`
* `parser.y`
* `tree.c`
* `tree.h`

The first file can be almost identical to the one from the previous tasks, with minor adaptations whenever necessary. The second file will be similar to the previous one, but will include semantic actions in order to build the tree. The other two files should implement the tree building functions.

Your implementation will be tested by the program `test_tree.c` which is provided (with `test_tree.h`). Notice that `test_tree.c` will import the submitted file `tree.h`. The minimum requirement for this file is to include the definitions:
```
   void *getTree();  // returns any pointer
   void counts(void *p, int *functions, int *funcalls, int *whiles, int *ifs, int *bin);
```

Both functions must be implemented in one of the submitted `.c` files. The first function should return a pointer to the final program tree. Notice that if the tree was build correctly, it will be the only pointer on the stack. It is not necessary for the testing program to know its type.

The second function should return the number of occurrences of program constructs in your tree corresponding to:
* function declarations (`functions`)
* function calls (`funcalls`)
* while statements (`whiles`)
* if statements (`ifs`)
* binary operations (`bin`)

The parameter `p` is the pointer to the tree, and other parameters are references to variables which will contain the results. A simple way of implementing the function `counts(...)` is to declare an auxiliary function like:
```
int count(TreeNodePtr p, Categ cat);
```

which recursively counts nodes of a given category. (The details depend on your implementation of the tree.)

It is suggested that you also include in `tree.h` all declarations describing the nodes of your tree; they will be necessary in the next task (code generation from trees).

You should also implement an auxiliary function to dump the contents of a node and recursively of its subtrees. Such a function can be very useful in debugging your code, but will not be called by the test program. The header of this function could be, for instance:
```
 void dumpTree(TreeNodePtr p, int indent);
```

where `p` is a pointer to the node (root of a tree) and `indent` is the amount of indentation for this node — it should be increased in recursive calls for subtrees in order to improve readability.

The submission will be processed by the following sequence of Linux commands:
```
bison -d -o parser.c parser.y
flex -i -o scanner.c  scanner.l
gcc -std=c99 -pedantic -o test_tree *.c
for each test NN:
    test_tree  <  progNN.sl  >  resultNN
```

There are three sets of test files. Tests 00 to 27 cover the minimal part of SL to be implemented. Optional tests 31 to 37 cover type declarations and arrays. Finally, optional tests 41 to 43 cover functions as parameters. The correct option should be chosen for submission.

### Remarks
* Up to four additional `.c` or `.h` files may be submitted; they will be included in the compilation process (`*.c`).
* File `all.zip` (see auxiliary files) provides the test program mentioned above as well as the test files and the results.
* Maximum number of submissions is 20 — test your solutions exhaustively before submission!
* This task is worth 15% of the final project grade.
* Due to the number of tests and the processing required for each of them, the results of each submission may take up to 30 seconds or even more, depending on the current load of the SuSy system.

### Obs
* The `all.zip` file mentioned had the files contained in the directory `tests` and the files `src/test_tree.h` and `test_tree.c`.
* This syntax tree is based on the scanner implemented previously. The scanner project can be found on [sl-scanner](https://github.com/sabrina-beck/sl-scanner)
* This syntax tree is also based on the parser implemented previously. The parser project can be found on [sl-parser](https://github.com/sabrina-beck/sl-parser)

## Project Execution

In order to run the program use the following command:
```
make run
```

In order to run the tests use:
```
make test
```