/* $Id$ */

#include <stdio.h>
#include "scheme.h"

char* symchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!$%&*/:<=>?^_~0123456789+-.@";

int main(int argc, char* argv[])
{
    int i, n = strlen(symchars);
    char buf[1024];

    srand(0);
    
    for (i = 0; i < 10000000; i++) {
        int j, s = rand() % 6;
        for (j = 0; j < s; j++)
            buf[j] = symchars[rand() % n];
        buf[j] = '\0';

        make_symbol(buf, s);
    }
}
