#ifndef CYMBOL_HEADER
#define CYMBOL_HEADER

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum CymbolType{

    CYM_NONE = 0,
    CYM_SCALAR,
    CYM_VECTOR,
    CYM_MATRICE

} CymbolType;

typedef struct Cym_stream{
    // data buffer
    char*  data;
    // size of the stream in bytes
    size_t   size;
    // capacity of the stream in bytes
    size_t   capacity;
} Cym_stream;

typedef struct Cymdata {

    Cym_stream symbols;
    Cym_stream data;

} Cymdata;

typedef struct Cymbol{
    size_t str;
    CymbolType type;
    union{
        size_t as_index;
        double as_float;
    } data;
} Cymbol;

Cymdata cym_create_cymdata(size_t init_cap){
    Cymdata output;
    output.data.capacity = init_cap + 1;
    output.data.size = 1;
    output.data.data = (char*)malloc(init_cap);
    output.symbols.capacity = init_cap + 1;
    output.symbols.size = sizeof(Cymbol);
    output.symbols.data = (char*)malloc(init_cap);
    return output;
}

void cym_destroy_cymdata(Cymdata* data){
    free(data->data.data);
    free(data->symbols.data);
    data->data.capacity = 0;
    data->data.size = 1;
    data->data.data = NULL;
    data->symbols.capacity = 0;
    data->symbols.size = 1;
    data->symbols.data = NULL;
}

static inline int cym_compare_str(const char* str1, const char* str2){

    size_t i = 0;
    for( ; str1[i]; i+=1){
        if(str1[i] != str2[i]) return 0;
    }
    return !str2[i];
}

void cym_stream(Cym_stream* stream, const void* data, size_t size){
    if(size + stream->size > stream->capacity){
        char* old_data = stream->data;
        stream->capacity *= 1 + (size_t)((size + stream->size) / stream->capacity);
        stream->data = (char*)malloc(stream->capacity);
        memcpy(stream->data, old_data, stream->size);
        free(old_data);
    }
    memcpy(stream->data + stream->size, data, size);
    stream->size += size;
}

#define CYM_STRM_GET(STREAM, TYPE, INDEX) *(TYPE*)(STREAM.data + INDEX * sizeof(TYPE))


void cym_add_scalar(Cymdata* data, const char* name, double value){
    Cymbol cymbol;
    cymbol.str = data->data.size;
    cymbol.type = CYM_SCALAR;
    cymbol.data.as_float = value;
    cym_stream(&data->symbols, &cymbol, sizeof(cymbol));
    size_t str_size = 0;
    for (; name[str_size]; str_size += 1);
    cym_stream(&data->data, name, str_size + 1);
}

size_t cym_get_cymbol(Cymdata* data, const char* name){

    for(size_t i = 1; i < data->symbols.size / sizeof(Cymbol); i+=1){
        const Cymbol cymbol = CYM_STRM_GET(data->symbols, Cymbol, i);
        if(cym_compare_str(name, data->data.data + cymbol.str)){
            return i;
        }
    }

    return 0;
}

void cym_display_cymbol(Cymdata* data, size_t cymbol){
    const Cymbol _cymbol = CYM_STRM_GET(data->symbols, Cymbol, cymbol);
    const char* name = (char*)(data->data.data + _cymbol.str);
    if(_cymbol.type == CYM_SCALAR){
        printf(
            "\\==X(Cymbol)X==\\\n\n"
            "name: %s\ntype: scalar\ndata: %f\n\n",
            name, _cymbol.data.as_float
        );
    } else{
        printf(
            "\\==X(Cymbol)X==\\\n\n"
            "name: %s\ntype: scalar\ndata: NONE\n", name
        );
    }
}


#endif // =====================  END OF FILE CYMBOL_HEADER ===========================