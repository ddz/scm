/* $Id$ */

#include <assert.h>
#include <stdio.h>
#include "types.h"

int test_imm_types()
{
    assert(IS_FIXNUM(MAKE_FIXNUM(0)));
    assert(IS_FIXNUM(MAKE_FIXNUM(1)));
    assert(IS_BOOL(BOOL_TRUE));
    assert(IS_BOOL(BOOL_FALSE));
    assert(IS_CHAR(MAKE_CHAR('a')));
    assert(IS_UNIQ(UNIQ_NIL));
    assert(IS_UNIQ(UNIQ_EOF));
    assert(IS_UNIQ(UNIQ_UNSPEC));
    assert(IS_UNIQ(UNIQ_UNDEF));
	
    return 0;
}

int main(int argc, char* argv[])
{
    test_imm_types();

    return 0;
}
