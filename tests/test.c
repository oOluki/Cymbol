#define CYM_IMPLEMENTATION
#define CYMATH_IMPLEMENTATION
#include "../include/cymbol.h"
#include "../include/cymath.h"






int main(){

    CymContext context = cym_create_context(100);

    const char* u = "hello world";

    uint64_t cym = cym_push_str(&context, "test cymbol", u);

    cym_save_context(&context, "context.bin");

    cym_destroy_context(&context);

    cym_load_context(&context, "context.bin");

    cym = cym_get_cymbol(&context, "test cymbol");

    cym_display_cymbol(&context, cym);

    char buff[sizeof("hello world")];

    memcpy(buff, cym_get_cymbol_data(&context, NULL, NULL, cym), sizeof("hello world"));

    cym_destroy_context(&context);

    return !cym_compare_str(buff, u);
}





