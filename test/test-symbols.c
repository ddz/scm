/* $Id$ */

#include <stdio.h>
#include "scheme.h"

char* symchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!$%&*/:<=>?^_~0123456789+-.@";

void test_simple()
{
    char* a = "aye";
    char* b = "aye";
    char* c = "aye";

    make_symbol(a, 4);
    make_symbol(b, 4);
    make_symbol(c, 4);
}

void test_random() {
    int i, n = strlen(symchars);
    char buf[1024];

    srand(0);
    
    for (i = 0; i < 1000000; i++) {
        int j, s = rand() % 6;
        for (j = 0; j < s; j++)
            buf[j] = symchars[rand() % n];
        buf[j] = '\0';

        make_symbol(buf, s);
    }
}

int main(int argc, char* argv[])
{
    test_simple();
    test_random();

    return 0;
}

