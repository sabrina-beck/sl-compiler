void SemanticError(char *s);
int yyerror();

//* Avoids warnings */
#include <stdio.h>
int yylex();
int yyparse ();
int fileno(FILE *);

