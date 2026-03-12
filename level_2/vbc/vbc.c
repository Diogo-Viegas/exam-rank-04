#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct s_parser {
    char *s;
    int i;
} t_parser;

void error_token(char c) {
    if (c == '\0')
        printf("Unexpected end of input\n");
    else
        printf("Unexpected token '%c'\n", c);
    exit(1);
}

int expression(t_parser *p);

// Factors: Digits or ( Expression )
int factor(t_parser *p) {
    if (isdigit(p->s[p->i])) {
        return p->s[p->i++] - '0';
    } else if (p->s[p->i] == '(') {
        p->i++; // Skip '('
        int res = expression(p);
        if (p->s[p->i] != ')')
            error_token(p->s[p->i]);
        p->i++; // Skip ')'
        return res;
    }
    error_token(p->s[p->i]);
    return 0;
}

// Terms: Multiplication
int term(t_parser *p) {
    int res = factor(p);
    while (p->s[p->i] == '*') {
        p->i++;
        res *= factor(p);
    }
    return res;
}

// Expressions: Addition
int expression(t_parser *p) {
    int res = term(p);
    while (p->s[p->i] == '+') {
        p->i++;
        res += term(p);
    }
    return res;
}

int main(int ac, char **av) {
    if (ac != 2) return 1;

    t_parser p = {av[1], 0};
    
    // An empty string is an unexpected end of input
    if (p.s[p.i] == '\0') {
        error_token('\0');
    }

    int result = expression(&p);

    // If the parser finished but there are trailing characters (like extra ')')
    if (p.s[p.i] != '\0') {
        error_token(p.s[p.i]);
    }

    printf("%d\n", result);
    return 0;
}