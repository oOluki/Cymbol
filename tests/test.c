#include "../include/cymbol.h"
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char** argv){

    CymContext context;

    cym_load_context(&context, "save.bin");

    size_t cymbol = cym_get_cymbol(&context, "data");

    cym_display_cymbol(&context, cymbol);

    float o[] = {
        0.5, 0, 5
    };

    cym_push_data(&context, "o", CYM_FLOAT32, 1, 3, o);

    size_t oi = cym_get_cymbol(&context, "o");

    cym_display_cymbol(&context, oi);

    cym_destroy_context(&context);

    return 0;
}
/*
    CymContext context = cym_create_context(1000);

    float data[] = {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };

    cym_push_data(&context, "data", CYM_FLOAT32, 3, 3, data);

    size_t cymbol = cym_get_cymbol(&context, "data");

    cym_display_cymbol(&context, cymbol);

    cym_save_context(&context, "save.bin");

    cym_destroy_context(&context);


    return 0;
}

*/
