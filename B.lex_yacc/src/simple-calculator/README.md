## Description

    This is a simple calculator following Wayne Cochran's 3-part series on YT.

## Compilation

    lex calc.l
    yacc -d calc.y
    cc lex.yy.c y.tab.c -o calc
