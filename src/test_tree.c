
/*
   Test of the tree builder SL.
   
   Returns code execution 0 even in case of errors so as not to
   confuse SuSy.
*/

/* Last update: "test_tree.c: 2020-10-15 (Thu)  11:43:29 -03 (tk)" */

#define MAIN

#include <stdio.h>
#include <stdlib.h>
#include "test_tree.h"
#include "tree.h"
//#include "tree_visualizer.h"

extern int line_num;

int yyerror(char *s) {
  
  printf("Error detected on line %d.\n",line_num);
  printf("Last token read: '%s'\n\n",yytext);
  return 0;
  
}

int main(int argc, char **argv) {

  int functions, funcalls, whiles, ifs, bin;
  void *tree; // test program does not need to know the type

  if (yyparse()!=0)
    return 0;  // error message printed already

  tree = getTree();
  counts(tree, &functions, &funcalls, &whiles, &ifs, &bin);
  printf("%d function declaration(s)\n", functions);
  printf("%d function call(s)\n", funcalls);
  printf("%d loop(s)\n", whiles);
  printf("%d conditional(s)\n", ifs);
  printf("%d binary operation(s)\n", bin);

  //dumpTree(tree);
  
  return 0;

}

