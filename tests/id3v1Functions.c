#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include "id3v1.h"

static void id3v1HasTag_foundTag(void **state){
    (void) state; /* unused */
}


int main(){
    
    const struct CMUnitTest tests[] = {
        //id3v1HasTag tests
        cmocka_unit_test(id3v1HasTag_foundTag),

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}