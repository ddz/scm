/* $Id$ */

#include <stdio.h>
#include <string.h>
#include "map.h"

char* symchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!$%&*/:<=>?^_~0123456789+-.@";

/*
 * hash_pjw
 */
size_t hash_string(const void* v)
{
    const char* p = (const char*)v;
    register unsigned int h = 0, g, l, n = strlen(p);
    for (l = n; l > 0; --l) {
        h = (h<<4) + *p++;
        if ((g = h&0xf0000000)) {
            h = h ^ (g>>24);
            h = h^g;
        }
    }
    return h&0xfffffff;

}

void test_simple()
{
    map_t ht;
    char* a = "aye";
    char* b = "bee";
    char* c = "sea";
    void* v;
    
    map_init(&ht, hash_string, (map_cmp_t)strcmp);
    map_put(&ht, a, a);
    map_put(&ht, b, b);
    map_put(&ht, c, c);

    v = map_get(&ht, a);
    
    if (map_get(&ht, a) != a ||
        map_get(&ht, b) != b ||
        map_get(&ht, c) != c)
        printf("Hash get error\n");
}

void test_random()
{
    map_t ht;
    int i, n = strlen(symchars);
    char buf[1024];

    srand(0);

    map_init(&ht, hash_string, (map_cmp_t)strcmp);
    
    for (i = 0; i < 1000000; i++) {
        char* str;
        int j, s = rand() % 6;
        for (j = 0; j < s; j++)
            buf[j] = symchars[rand() % n];
        buf[j] = '\0';

        str = strdup(buf);
        map_put(&ht, str, str);
    }
}

int main(int argc, char* argv[])
{
    test_simple();
    test_random();

    return 0;
}
