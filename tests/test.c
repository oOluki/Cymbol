#include "../include/cymbol.h"
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char** argv){

    Cymdata data = cym_create_cymdata(1000);

    cym_add_scalar(&data, "dummy", 15);

    cym_add_scalar(&data, "e", 2.7182);

    cym_add_scalar(&data, "pi", 3.14159);

    size_t symbol = cym_get_cymbol(&data, "dummy");

    size_t e = cym_get_cymbol(&data, "e");

    size_t pi = cym_get_cymbol(&data, "pi");

    cym_display_cymbol(&data, symbol);

    cym_display_cymbol(&data, e);

    cym_display_cymbol(&data, pi);


    return 0;
}


