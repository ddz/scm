/* $Id$ */
/*
 * Use funopen(3) to make a string behave like a read-only FILE*.
 */

#include <stdio.h>

/*
 * State structure used by string stream functions
 */
typedef struct {
    const char*  str;     /* The string to be read from           */
    size_t len;           /* Total length of str                  */
    size_t pos;           /* Current position in str              */
    size_t left;          /* Number of characters left to be read */
} str_state_t;

int str_readfn(void* cookie, char* buf, int n)
{
    str_state_t* state = (str_state_t*)cookie;
    size_t amt;

    if (state->left > n)
	amt = n;
    else
	amt = state->left;
    
    if (amt == 0)
	return 0;
    
    strncpy(buf, state->str, amt);
    state->pos += amt;
    state->left -= amt;
    return amt;
}

FILE* fstropen(const char* str)
{
    str_state_t* state = (str_state_t*)malloc(sizeof(str_state_t));
    state->str = str;
    state->len = strlen(str);
    state->pos = 0;
    state->left = state->len;
    return fropen(state, str_readfn);
}
