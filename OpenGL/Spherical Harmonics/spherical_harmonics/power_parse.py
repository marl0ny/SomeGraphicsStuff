#!/usr/bin/python3
"""
Replace terms in an expression such as a^b and a**b
with pow(a, b)
"""
from typing import Tuple, List


def is_alpha_numeric_or_underscore(letter: str) -> bool:
    """
    Check if a letter is alphanumeric or an underscore.
    """
    # TODO need to handle values like 12e-23.
    return letter.isalpha() or letter.isnumeric() \
            or letter == '_' or letter == '.'


def get_pow_expr_loc(expr: str, i: int) -> Tuple[int, int]:
    """
    Given the index of the power symbol in a power term,
    return the start and end of the power term relative to the
    start of the string.

    Parameters:
     expr: the entire mathematical expression that contains
     the power term.
     i: the location of the power symbol.
    
    """
    p_stack: List[str] = []
    j: int = i + 1
    while (j < len(expr) and
            (
             (is_alpha_numeric_or_underscore(expr[j])
              or expr[j] == '(' or p_stack
             ) or
             (not p_stack and ((expr[j: j+2] == '**'
                                or expr[j-1: j+1] =='**'
                               ) or expr[j] == '^')
             )
            )
          ):
        if expr[j] == '(':
            p_stack.append('(')
        if expr[j] == ')':
            p_stack.pop()
        j += 1
    p_stack = []
    k = i - 2 if expr[i] == '*' else i - 1
    while ((is_alpha_numeric_or_underscore(expr[k]) 
            or expr[k] ==')' or p_stack)
            and k >= 0):
        if expr[k] == ')':
            p_stack.append(')')
        if expr[k] == '(':
            p_stack.pop()
        k -= 1
    return k+1, j


def pow_ops_to_callables(expr: str, 
                         is_whitespace_removed: bool = False) -> str:
    """
    Replace all power terms that use the '**' operator
    with the pow callable function equivalent.

    Parameters:
     expr: the mathematical expression.
     is_whitespace_removed: whether there is whitespace
     in the expression or not.

    >>> expr = 'z**3'
    >>> z = 2.0
    >>> eval(pow_ops_to_callables(expr))
    8.0
    >>> expr = 'a*(x-0.5)**2 + b*(y-0.5)**2'
    >>> a, b, x, y = 10.0, 20.0, 1.0, 2.0
    >>> eval(pow_ops_to_callables(expr))
    47.5
    >>> from math import cos, pow
    >>> expr = 'x**2 + (y + z)**(x + t**2)**cos(u)'
    >>> x, y, z, y, t, u = 1.0, 2.0, 3.0, 4.0, 1.0, 0.0
    >>> eval(pow_ops_to_callables(expr))
    50.0
    >>> expr = 'x**2 + (y + z)**(x**(z + t) + t**2)**cos(z**(t-x)**z)'
    >>> x, y, z, y, t, u = 1.0, 2.0, 3.0, 4.0, 1.0, 0.0
    >>> eval(pow_ops_to_callables(expr))
    17.943644342697727
    >>> expr = 'a*(x-0.5)^2 + b*(y-0.5)^2'
    >>> a, b, x, y = 10.0, 20.0, 1.0, 2.0
    >>> eval(pow_ops_to_callables(expr))
    47.5
    >>> expr = 'x^2 + (y + z)^(x^(z + t) + t^2)^cos(z^(t-x)^z)'
    >>> x, y, z, y, t, u = 1.0, 2.0, 3.0, 4.0, 1.0, 0.0
    >>> eval(pow_ops_to_callables(expr))
    17.943644342697727


    """
    if not is_whitespace_removed:
        expr = expr.replace(' ', '') 
    i: int = 0
    while (i < len(expr)):
        c: str = expr[i]
        if c == '^' or (c == '*' and i > 0 and 
                        expr[i-1] == '*'):
            start, finish = get_pow_expr_loc(expr, i)
            pow_expr = pow_op_to_callable(expr[start: finish])
            expr = expr[:start] + pow_expr + expr[finish:]
        i += 1
    return expr


def pow_op_to_callable(expr: str) -> str:
    """
    Replace a string expression of the form 'a**b' or 'a^b' 
    to 'pow(a, b)'. Expressions of the form 'a**(b**c**...)' 
    will be replaced with 'pow(a, b**c**...)'.

    A necessary precondition is that the expression either contains
    '^' or '**'.

    Parameters:
     expr: string expression of the form 'a**b'
    """
    caret, stars = expr.find('^'), expr.find('**')
    if stars >= 0 and caret < 0:
        bin_tokens = expr.split('**', 1)
    elif stars < 0 and caret >= 0:
        bin_tokens = expr.split('^', 1)
    else:
        bin_tokens = (expr.split('^', 1) if caret < stars 
                      else expr.split('**', 1))
    return 'pow(' + bin_tokens[0] + ',' + bin_tokens[1] + ')'


# expr = 'a*(x-0.5)**2 + b*(y-0.5)**2'
# print(pow_ops_to_callables(expr))
# expr = 'x^2 + (y + z)^(x^(z + t) + t^2)^cos(z^(t-x)^z)'
# print(pow_ops_to_callables(expr))
