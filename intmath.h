#ifndef INTMATH_H
#define INTMATH_H

#include <stdlib.h>

div_t adiv(int numerator, int denominator);
int iaquot(int numerator, int denominator);
int iamod(int numerator, int denominator);
div_t alinediv(int numerator, int factor, int addend, int denominator);
int alinequot(int numerator, int factor, int addend, int denominator);
int alinemod(int numerator, int factor, int addend, int denominator);

#endif
