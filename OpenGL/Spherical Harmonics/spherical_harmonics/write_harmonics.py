#!/usr/bin/python3
import sympy as sym
from power_parse import pow_ops_to_callables
import re


f = """/*This file is generated using Sympy.
*/
#include <complex>
#include "harmonics.h"


std::complex<double> spherical_harmonics(int n, int m, double x, double y, double z) {
    double pi = 3.141592653589793;
    double r = std::sqrt(x*x + y*y + z*z);
    double r_xy = std::sqrt(x*x + y*y);
    double theta = std::acos(z/(r + 1e-30));
    double phi = std::asin(y/(r_xy + 1e-30));
    if (x <= 0.0)
        phi = pi - phi;
    std::complex <double>val;
    std::complex <double>I {0.0, 1.0};
    using std::pow;
    using std::sqrt;
    using std::cos;
    using std::sin;
    using std::exp;
"""

N = 11
for n in range(0, N):
    for m in range(-n, n+1, 1):
        if n == 0:
            f += ('    ' + f'if (n == {n} && m == {m})' + ' {\n')
        else:
            f += (f'else if (n == {n} && m == {m})' + ' {\n')
        z = sym.Symbol('z')
        x = sym.Symbol('x')
        y = sym.Symbol('y')
        r = sym.Symbol('r')
        theta = sym.Symbol('theta')
        phi = sym.Symbol('phi')
        sym_expr = sym.expand((sym.Ynm(n, m, theta, phi).expand(func=True)))
        # sym_expr = sym_expr.subs(sym.cos(theta), z/r)
        # sym_expr = sym_expr.subs(sym.sin(theta), sym.sqrt(x*x + y*y)/r)
        # sym_expr = sym_expr.subs(sym.exp(m*I*phi), (x )*sym.sqrt(x*x + y*y)/r)
        expr = str(sym_expr)
        expr_lst = [c for c in expr]
        expr_lst2 = []
        for i, c in enumerate(expr_lst):
            if (expr_lst[i].isnumeric() and i + 1 < len(expr_lst) 
                and (not expr[i+1].isnumeric())):
                expr_lst2.append(c + '.0')
            else:
                expr_lst2.append(c)
        expr = ''.join(expr_lst2)
        expr = pow_ops_to_callables(expr)
        f += ('        ' + 'val = ' + expr + ';\n    } ')


f += '    return val;\n}\n'
# print(f)

with open('harmonics.cpp', 'w') as f_:
    f_.write(f)