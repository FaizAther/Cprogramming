#!/bin/sh
    
lex calc.l
yacc -d calc.y
cc lex.yy.c y.tab.c -o calc
