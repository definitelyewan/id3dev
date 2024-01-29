#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "id3v2.h"
#include "byteStream.h"

static void playground(void **state){

    uint8_t header[10] = {'I','D','3',0,0,0,0

    };
}

int main(){
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(playground)

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}