#!/bin/bash
echo carousel_model.c has `wc -l carousel_model.c` lines

time gcc -c -O0 -S carousel_model.c
echo carousel_model.s has `wc -l carousel_model.s` lines

time gcc -c -O3 -S carousel_model.c
echo carousel_model.s has `wc -l carousel_model.s` lines

time clang -c -O0 -S carousel_model.c
echo carousel_model.s has `wc -l carousel_model.s` lines

time clang -c -O4 -S carousel_model.c
echo carousel_model.s has `wc -l carousel_model.s` lines
