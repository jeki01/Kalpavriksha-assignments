#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h> // for INT_MAX and INT_MIN

const char *p; // global pointer for parsing

// function to skip spaces
void skipSpaces(void) {
    while (*p == ' ') p++;
}

// helper function to check overflow
int willOverflow(long long a, long long b, char op) {
    switch (op) {
        case '+':
            if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
                return 1;
            break;

        case '-':
            if ((b < 0 && a > INT_MAX + b) || (b > 0 && a < INT_MIN + b))
                return 1;
            break;

        case '*':
            if (a != 0 && (a > INT_MAX / b || a < INT_MIN / b))
                return 1;
            break;

        case '/':
            if (b == 0 || (a == INT_MIN && b == -1))
                return 1;
            break;
    }
    return 0;
}


// It skips any leading spaces and checks the current character.
int solveFactor(int *err) {
    skipSpaces();
    if (*p == '\0') { *err = 1; return 0; }

    if (isdigit((unsigned char)*p)) {
        int num = 0;
        while (isdigit((unsigned char)*p)) {
            num = num * 10 + (*p - '0');
            p++;
        }
        return num;
    }
    // brackets not allowed
    else if (*p == '(' || *p == ')') {
        *err = 1;
        return 0;
    }
    else {
        *err = 1; // invalid character
        return 0;
    }
}

// term: handles * and /
int solveTerm(int *err) {
    int result = solveFactor(err);
    if (*err) return 0;
    skipSpaces();

    while (*p == '*' || *p == '/') {
        char op = *p;
        p++;
        int right = solveFactor(err);
        if (*err) return 0;

        if (op == '*') {
            if (willOverflow(result, right, '*')) {
                *err = 3; // overflow
                return 0;
            }
            result *= right;
        } else {
            if (right == 0) {
                *err = 2; // divide by zero
                return 0;
            }
            if (willOverflow(result, right, '/')) {
                *err = 3; // overflow
                return 0;
            }
            result /= right;
        }
        skipSpaces();
    }
    return result;
}

// expression: handles + and -
int solveExpr(int *err) {
    skipSpaces();
    int result = solveTerm(err);
    if (*err) return 0;
    skipSpaces();

    while (*p == '+' || *p == '-') {
        char op = *p;
        p++;
        int right = solveTerm(err);
        if (*err) return 0;

        if (willOverflow(result, right, op)) {
            *err = 3; // overflow
            return 0;
        }

        if (op == '+') result += right;
        else result -= right;

        skipSpaces();

        // check for invalid character anywhere in expression
        if (*p && !isdigit(*p) && *p != '+' && *p != '-' && *p != '*' && *p != '/' && *p != ' ') {
            *err = 1;
            return 0;
        }
    }
    return result;
}

int main() {
    char input[100];

    printf("Enter expression: ");
    if (!fgets(input, sizeof(input), stdin)) {
        printf("Error: Invalid expression.\n");
        return 0;
    }

    // check for expression too long
    if (input[strlen(input) - 1] != '\n') {
        printf("Error: Expression too long! Maximum allowed length is 99 characters.\n");
        int c;
        while ((c = getchar()) != '\n' && c != EOF); // flush
        return 0;
    }

    // remove newline
    input[strcspn(input, "\n")] = 0;

    p = input; // initialize global pointer
    int err = 0;
    int ans = solveExpr(&err);

    skipSpaces();
    if (*p != '\0') err = 1; // leftover invalid characters

    if (err == 1) {
        printf("Error: Invalid expression.\n");
    } else if (err == 2) {
        printf("Error: Division by zero.\n");
    } else if (err == 3) {
        printf("Error: Integer overflow.\n");
    } else {
        printf("%d\n", ans);
    }

    return 0;
}

