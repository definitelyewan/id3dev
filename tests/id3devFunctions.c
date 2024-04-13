#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "id3dev.h"
#include "id3v2/id3v2.h"
#include "id3v1/id3v1.h"

static void id3CreateAndDestroy_allInOne(void **state){

    ID3 *metadata = id3Create(id3v2TagFromFile("asserts/sorry4dying.mp3"), id3v1TagFromFile("asserts/sorry4dying.mp3"));

    assert_non_null(metadata);

    id3Destroy(&metadata);
}


int main(){
    
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(id3CreateAndDestroy_allInOne)

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}