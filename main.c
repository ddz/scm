#include <stdio.h>

extern int yyparse();

int main(int argc, char* argv[])
{
    return yyparse();
}
