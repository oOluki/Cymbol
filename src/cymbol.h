#ifndef CYMBOL_HEADER
#define CYMBOL_HEADER

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

// X=================X (START) X========================X

enum CymMemBlock{

    CYM_MEMBLOCK_SYMBOLS = 0x53594d,
    CYM_MEMBLOCK_DATA    = 44415441

};

typedef enum CymAtomType{

    CYM_ATOMTYPENONE = 0,

    CYM_INT8,
    CYM_INT16,
    CYM_INT32,
    CYM_INT64,

    CYM_UINT8,
    CYM_UINT16,
    CYM_UINT32,
    CYM_UINT64,

    CYM_FLOAT32,
    CYM_FLOAT64

} CymAtomType;

typedef enum CymbolType{

    CYM_CYMBOLTYPENONE = 0,

    CYM_SCALAR,
    CYM_VECTOR,
    CYM_MATRICE,

} CymbolType;

typedef struct CymbolMetaData{

    uint8_t     atom_type;
    uint8_t     type;
    uint64_t    size1;
    uint64_t    size2;

} CymbolMetaData;


typedef struct CymStream{
    // data buffer
    char*    data;
    // size of the stream in bytes
    uint64_t size;
    // capacity of the stream in bytes
    uint64_t capacity;
} CymStream;

typedef struct CymContext {

    uint64_t  flags;
    uint8_t   float_display_precision;

    uint64_t  sizeof_extra_data;
    void*     extra_data;

    CymStream symbols;
    CymStream data;

} CymContext;

typedef struct Cymbol{
    uint64_t str;
    uint64_t data;
} Cymbol;

#ifdef __cplusplus
extern "C" {
#endif

// X=================X (CORE FUNCTIONALITY) X========================X

CymContext cym_create_context(uint64_t init_cap){
    CymContext output;
    output.data.capacity = init_cap + 1;
    output.data.size = 1;
    output.data.data = (char*)malloc(init_cap + 1);
    output.symbols.capacity = init_cap + 1;
    output.symbols.size = sizeof(Cymbol);
    output.symbols.data = (char*)malloc(init_cap + sizeof(Cymbol));
    return output;
}

void cym_destroy_context(CymContext* context){
    if(context->data.data) free(context->data.data);
    if(context->symbols.data) free(context->symbols.data);
    context->data.capacity = 0;
    context->data.size = 0;
    context->data.data = NULL;
    context->symbols.capacity = 0;
    context->symbols.size = 0;
    context->symbols.data = NULL;
}

static inline int cym_compare_str(const char* str1, const char* str2){

    uint64_t i = 0;
    for( ; str1[i]; i+=1){
        if(str1[i] != str2[i]) return 0;
    }
    return !str2[i];
}

void cym_stream(CymStream* stream, const void* data, uint64_t size){
    if(size + stream->size > stream->capacity){
        char* old_data = stream->data;
        stream->capacity *= 1 + (uint64_t)((size + stream->size) / stream->capacity);
        stream->data = (char*)malloc(stream->capacity);
        memcpy(stream->data, old_data, stream->size);
        free(old_data);
    }
    memcpy(stream->data + stream->size, data, size);
    stream->size += size;
}

void cym_stream_str(CymStream* stream, const char* str){
    uint64_t size = 0;
    for(; str[size]; size+=1);
    cym_stream(stream, str, size + 1);
}

#define CYM_STRM_GET(STREAM, TYPE, INDEX) *(TYPE*)(STREAM.data + INDEX * sizeof(TYPE))


static inline uint64_t cym_get_atom_size(uint8_t atom_type){
    switch (atom_type)
    {
    case CYM_INT8:
    case CYM_UINT8:
        return 1;
    case CYM_INT16:
    case CYM_UINT16:
        return 2;
    case CYM_INT32:
    case CYM_UINT32:
    case CYM_FLOAT32:
        return 4;
    case CYM_INT64:
    case CYM_UINT64:
    case CYM_FLOAT64:
        return 8;
    
    default:
        break;
    }
    return 0;
}

// returns the id of the newly allocated cymbol
// if size1 * size2 == 0 nothing will happen
// if data == NULL the memory will be allocated but not set
uint64_t cym_push_data(CymContext* context, const char* name, CymAtomType atom_type, uint64_t size1, uint64_t size2, const void* data){
    if(size1 * size2 == 0){
        return 0;
    }

    Cymbol cymbol;
    CymbolMetaData meta_data;
    meta_data.atom_type = atom_type;
    meta_data.size1 = size1;
    meta_data.size2 = size2;

    if(size1 * size2 == 1){ // size1 == size2 == 1
        meta_data.type = CYM_SCALAR;
    } else if(size1 == 1 || size2 == 1){
        meta_data.type = CYM_VECTOR;
    } else{
        meta_data.type = CYM_MATRICE;
    }

    cymbol.str = context->data.size;
    cym_stream_str(&context->data, name);
    cymbol.data = context->data.size;

    cym_stream(&context->symbols, &cymbol, sizeof(Cymbol));

    uint64_t meta_size = sizeof(meta_data);

    cym_stream(&context->data, &meta_size, sizeof(uint64_t));

    cym_stream(&context->data, &meta_data, sizeof(meta_data));
    
    if(data) cym_stream(&context->data, data, size1 * size2 * cym_get_atom_size(atom_type));
    else{
        const size_t memsize = size1 * size2 * cym_get_atom_size(atom_type);
        if(context->data.size + memsize > context->data.capacity){
            char* old_data = context->data.data;
            context->data.capacity *= 1 + (uint64_t)((memsize + context->data.size) / context->data.capacity);
            context->data.data = (char*)malloc(context->data.capacity);
            memcpy(context->data.data, old_data, context->data.size);
            free(old_data);
        }
        context->data.size += memsize;
    }

    return (context->symbols.size / sizeof(Cymbol)) - 1;
}

// you can't delete the 0th cymbol, just so you know
void cym_delete_cymbol(CymContext* context, uint64_t cymbolid){

    if(cymbolid == 0) return;

    const Cymbol cymbol = CYM_STRM_GET(context->symbols, Cymbol, cymbolid);

    char* strdata = context->data.data + cymbol.str;

    CymbolMetaData* meta_data = (CymbolMetaData*)(context->data.data + cymbol.data + sizeof(uint64_t));

    size_t skip =
        cym_get_atom_size(meta_data->atom_type) * meta_data->size1 * meta_data->size2
        + sizeof(CymbolMetaData) + ((char*)meta_data) - strdata;

    memcpy(strdata, strdata + skip, context->data.size - (strdata - context->data.data));

    context->data.size -= skip;

    memcpy(
        context->symbols.data + cymbolid * sizeof(Cymbol),
        context->symbols.data + (cymbolid + 1) * sizeof(Cymbol),
        context->symbols.size - cymbolid * sizeof(Cymbol)
    );

    context->symbols.size -= sizeof(Cymbol);

}


uint64_t cym_get_cymbol(CymContext* context, const char* name){

    const uint64_t range = context->symbols.size / sizeof(Cymbol);

    for(uint64_t i = 1; i < range; i+=1){
        const Cymbol cymbol = *(Cymbol*)(context->symbols.data + i * sizeof(Cymbol));
        
        if(cym_compare_str(name, context->data.data + cymbol.str)){
            return i;
        }
    }

    return 0;
}

// Keep in mind that the pointers are only valid untill the context's data stream has its capacity resized
// \returns a pointer to the raw data of the cymbol with id cymbolid
// \param meta_data a pointer to write the meta data to, pass NULL to ignore this
// \param name a pointer to the C_string to write the cymbol's name, pass NULL to ignore this
void* cym_get_cymbol_data(CymContext* context, CymbolMetaData* meta_data, const char** name, uint64_t cymbolid){
    Cymbol cymbol = CYM_STRM_GET(context->symbols, Cymbol, cymbolid);

    if(name) *name = context->data.data + cymbol.str;

    char* data = context->data.data + cymbol.data;

    data += sizeof(uint64_t);
    if(meta_data) *meta_data = *(CymbolMetaData*)data;

    // skipping meta data
    data += sizeof(CymbolMetaData);

    return data;
}

void cym_transfer_data(CymContext* context, uint64_t cymbolid, CymbolMetaData* meta_data, void* data){
    Cymbol cymbol = CYM_STRM_GET(context->symbols, Cymbol, cymbolid);

    char* _data = context->data.data + cymbol.data + sizeof(uint64_t);
    if(meta_data) *meta_data = *(CymbolMetaData*)_data;
    if(data == NULL) return;

    uint64_t sizeof_atom = cym_get_atom_size(meta_data->atom_type);

    memcpy(data, _data + sizeof(CymbolMetaData), sizeof_atom * meta_data->size1 * meta_data->size2);
}

// pass output = NULL to write the output to the same cymbol's data
void cym_apply_fun(CymContext* context, uint64_t cymbolid, void* function, void* output){
    Cymbol cymbol = CYM_STRM_GET(context->symbols, Cymbol, cymbolid);

    char* data = context->data.data + cymbol.data + sizeof(uint64_t);

    const CymbolMetaData meta_data = *(CymbolMetaData*)data;

    data += sizeof(CymbolMetaData);

    if(output == NULL) output = data;

    #define CYM_INTERNAL_APPLY_FUN(TYPE, SIZE)\
        for(size_t i = 0; i < SIZE; i+=1){\
            ((TYPE*)output)[i] = ((TYPE(*)(TYPE))(function)) (((TYPE*)data)[i]);\
        }

    switch (meta_data.atom_type)
    {
    case CYM_INT8:
        CYM_INTERNAL_APPLY_FUN(int8_t, meta_data.size1 * meta_data.size2);
        break;
    case CYM_INT16:
        CYM_INTERNAL_APPLY_FUN(int16_t, meta_data.size1 * meta_data.size2);
        break;
    case CYM_INT32:
        CYM_INTERNAL_APPLY_FUN(int32_t, meta_data.size1 * meta_data.size2);
        break;
    case CYM_INT64:
        CYM_INTERNAL_APPLY_FUN(int64_t, meta_data.size1 * meta_data.size2);
        break;

    case CYM_UINT8:
        CYM_INTERNAL_APPLY_FUN(uint8_t, meta_data.size1 * meta_data.size2);
        break;
    case CYM_UINT16:
        CYM_INTERNAL_APPLY_FUN(uint16_t, meta_data.size1 * meta_data.size2);
        break;
    case CYM_UINT32:
        CYM_INTERNAL_APPLY_FUN(uint32_t, meta_data.size1 * meta_data.size2);
        break;
    case CYM_UINT64:
        CYM_INTERNAL_APPLY_FUN(uint64_t, meta_data.size1 * meta_data.size2);
        break;
    
    case CYM_FLOAT32:
        CYM_INTERNAL_APPLY_FUN(float, meta_data.size1 * meta_data.size2);
        break;
    case CYM_FLOAT64:
        CYM_INTERNAL_APPLY_FUN(double, meta_data.size1 * meta_data.size2);
        break;
    
    default:
        break;
    }

    #undef CYM_INTERNAL_APPLY_FUN


}

void cym_display_cymbol(CymContext* context, uint64_t cymbolid){

    if(cymbolid == 0 || cymbolid > context->symbols.size / sizeof(Cymbol)){
        printf("\nCymbol Not Found\n");
        return;
    }

    const Cymbol cymbol = CYM_STRM_GET(context->symbols, Cymbol, cymbolid);

    printf("(Cymbol)\n\tname = %s\n", context->data.data + cymbol.str);
    
    CymbolMetaData meta_data;
    void* data = cym_get_cymbol_data(context, &meta_data, NULL, cymbolid);

    #define CYM_INTERNAL_PRINTEX(ATOM_TYPE, FORMAT, SIZE1, SIZE2, DATA)\
    printf(\
        "\tatom type: " #ATOM_TYPE "\n"\
        "\tsize = (%zu, %zu)\n\t",\
        SIZE1, SIZE2\
    );\
    for(uint64_t n = 0; n < SIZE1; n+=1){\
        for(uint64_t m = 0; m < SIZE2; m+=1){\
            printf("%" FORMAT ", ", ((ATOM_TYPE*)DATA)[n * SIZE2 + m]);\
        } printf("\n\t");\
    }
    

    switch (meta_data.atom_type)
    {
    case CYM_INT8:
        CYM_INTERNAL_PRINTEX(int8_t, PRId8, meta_data.size1, meta_data.size2, data);
        break;
    case CYM_INT16:
        CYM_INTERNAL_PRINTEX(int16_t, PRId16, meta_data.size1, meta_data.size2, data);
        break;
    case CYM_INT32:
        CYM_INTERNAL_PRINTEX(int32_t, PRId32, meta_data.size1, meta_data.size2, data);
        break;
    case CYM_INT64:
        CYM_INTERNAL_PRINTEX(int64_t, PRId64, meta_data.size1, meta_data.size2, data);
        break;

    case CYM_UINT8:
        CYM_INTERNAL_PRINTEX(uint8_t, PRIu8, meta_data.size1, meta_data.size2, data);
        break;
    case CYM_UINT16:
        CYM_INTERNAL_PRINTEX(uint16_t, PRIu16, meta_data.size1, meta_data.size2, data);
        break;
    case CYM_UINT32:
        CYM_INTERNAL_PRINTEX(uint32_t, PRIu32, meta_data.size1, meta_data.size2, data);
        break;
    case CYM_UINT64:
        CYM_INTERNAL_PRINTEX(uint64_t, PRIu64, meta_data.size1, meta_data.size2, data);
        break;
    
    case CYM_FLOAT32:
        CYM_INTERNAL_PRINTEX(float, "f", meta_data.size1, meta_data.size2, data);
        break;
    case CYM_FLOAT64:
        CYM_INTERNAL_PRINTEX(double, "f", meta_data.size1, meta_data.size2, data);
        break;
    
    default:
        break;
    }

    #undef CYM_INTERNAL_PRINTEX

    printf("\n");
}

void cym_merge_contexts(CymContext* dest, CymContext* src){
    
    const size_t src_cymcount = src->symbols.size / sizeof(Cymbol);

    for(size_t i = 1; i < src_cymcount; i+=1){
        const char* src_str;
        CymbolMetaData src_meta;

        void* srcdata = cym_get_cymbol_data(src, &src_meta, &src_str, i);

        const uint64_t destcym = cym_push_data(dest, src_str, src_meta.atom_type, src_meta.size1, src_meta.size2, srcdata);
    }

}

int cym_save_context(CymContext* context, const char* path){

    FILE* file = fopen(path, "wb");

    const uint64_t range = context->symbols.size / sizeof(Cymbol);

    if(!file){
        goto defer;
    }

    if(!fputs("$CYMBOL", file)){
        goto defer;
    }

    if(!fputs("SYM", file) || fwrite(&context->symbols.size, sizeof(context->symbols.size), 1, file) != 1){
        goto defer;
    }

    for(uint64_t i = 0; i < range; i+=1){
        const Cymbol cymbol = *(Cymbol*)(context->symbols.data + i * sizeof(Cymbol));
        
        if(fwrite(&cymbol.str, sizeof(cymbol.str), 1, file) != 1){
            goto defer;
        }
    }

    if(!fputs("DATA", file) || fwrite(&context->data.size, sizeof(context->data.size), 1, file) != 1 || fwrite(context->data.data, context->data.size, 1, file) != 1){
        goto defer;
    }
    
    fclose(file);

    return 0;

    defer:
        if(file) fclose(file);
        fprintf(stderr, "[ERROR] Cymbol: Error While Saving Context To '%s'\n", path);
        return -1;
}

int cym_load_context(CymContext* context, const char* path){

    cym_destroy_context(context);

    FILE* file = fopen(path, "rb");

    char buffer[10] = {
        '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0'
    };

    if(!file){
        goto defer;
    }

    if(!fgets(buffer, 8, file) || !cym_compare_str(buffer, "$CYMBOL")){
        goto defer;
    }

    if(!fgets(buffer, 4, file) || !cym_compare_str(buffer, "SYM")){
        goto defer;
    } memset(buffer, 0, 10);
    if(1 != fread(&context->symbols.capacity, 8, 1, file)){
        goto defer;
    }

    context->symbols.data = (char*)malloc(context->symbols.capacity);
    context->symbols.size = context->symbols.capacity;

    if(fread(context->symbols.data, context->symbols.capacity / 2, 1, file) != 1) goto defer;

    if(!fgets(buffer, 5, file) || !cym_compare_str(buffer, "DATA")){
        goto defer;
    } memset(buffer, 0, 10);
    if(1 != fread(&context->data.capacity, 8, 1, file)){
        goto defer;
    }

    context->data.data = (char*)malloc(context->data.capacity);
    context->data.size = context->data.capacity;

    if(fread(context->data.data, context->data.capacity, 1, file) != 1) goto defer;

    for(uint64_t i = 0; i < context->symbols.size / sizeof(Cymbol); i+=1){

        const uint64_t n = (context->symbols.size / sizeof(Cymbol)) - i - 1;
        Cymbol* cymbol = (Cymbol*)(context->symbols.data + n * sizeof(Cymbol));
        
        cymbol->str = *(uint64_t*)(context->symbols.data + n * sizeof(uint64_t));
        for(cymbol->data = cymbol->str; context->data.data[cymbol->data]; cymbol->data+=1);
        cymbol->data += 1;

    }

    fclose(file);

    return 0;

    defer:
        cym_destroy_context(context);
        if(file) fclose(file);
        fprintf(stderr, "[ERROR] Cymbol: Error While Loading Context From '%s'\n", path);
        return -1;

}

#ifdef __cplusplus
}
#endif

#endif // =====================  END OF FILE CYMBOL_HEADER ===========================