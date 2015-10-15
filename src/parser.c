#include "y.tab.h"

extern int yyparse();

void sql_parser()
{
    yyparse();
}
