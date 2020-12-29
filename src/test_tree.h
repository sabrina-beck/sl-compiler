//* Avoids warnings */
#include <stdio.h>
int yylex();
int yyparse ();
int yyerror();
int fileno(FILE *);

extern char *yytext;
