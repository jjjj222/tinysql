number      [0-9]
letter      [A-Za-z]
%{

%}


%%

[\t ]+          /*ignorewhitespace*/;

{number}+         { printf("%s is number\n", yytext); }

[a-zA-Z]+       { printf("%s: is not a verb\n", yytext); }

.|\n            {ECHO;}

%%

//main()
//{
//    printf("QQ\n");
//    yylex();
//    return 0;
////[\t ]+          /*ignorewhitespace*/;
////letter+         { printf("%s: is not a verb\n", yytext); }
//}
