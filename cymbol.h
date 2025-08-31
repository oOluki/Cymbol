#ifndef CYMBOL_HEADER
#define CYMBOL_HEADER

#include <inttypes.h>
#include <stdint.h>

#ifndef __need_size_t
    #define	__need_size_t
    #define CYM_UNDEF__need_size_t
#endif
#ifndef __need_NULL
    #define	__need_NULL
    #define CYM_UNDEF__need_NULL
#endif

#include <stddef.h>

#ifdef CYM_UNDEF__need_size_t
    #undef CYM_UNDEF__need_size_t
    #undef __need_size_t
#endif
#ifdef CYM_UNDEF__need_NULL
    #undef CYM_UNDEF__need_NULL
    #undef __need_NULL
#endif

#include <stdarg.h>



#ifndef CYM_MEMCPY

    #ifdef _STRING_H
        #define CYM_MEMCPY(DEST, SRC, SIZE) memcpy((DEST), (SRC), (SIZE))
    #else
        #define CYM_MEMCPY(DEST, SRC, SIZE)\
            for(size_t i = 0; i < (SIZE); i+=1) ((uint8_t*)(DEST))[i] = ((uint8_t*)(SRC))[i]
    #endif

#endif

// expands a typed pointer to the target's type size followed by itself
// usefull for passing arguments into cym_pack_dataRW, cym_unpack_dataRW, etc...
#define CYM_UNPACK_MEMPTR(x) (size_t) sizeof(*x), (void*) (x)

#define CYM_ARRAY_ITER(INDEX, ARRAY, CODE) for(size_t INDEX = 0; INDEX < sizeof(ARRAY) / sizeof(ARRAY[0]); INDEX+=1){ CODE }

#define CYM_TYPE_TO_ATOM_MAP(X) _Generic((X){0},\
    uint8_t       :     CYMATOM_U8,\
    int8_t        :     CYMATOM_I8,\
    uint16_t      :     CYMATOM_U16,\
    int16_t       :     CYMATOM_I16,\
    uint32_t      :     CYMATOM_U32,\
    int32_t       :     CYMATOM_I32,\
    float         :     CYMATOM_F32,\
    uint64_t      :     CYMATOM_U64,\
    int64_t       :     CYMATOM_I64,\
    double        :     CYMATOM_F64,\
    default       :     CYMATOM_NONE\
    )

#define CYM_TYPE_TO_CTYPE_MAP(X) _Generic((X){0},\
    unsigned char       :       CYMCTYPE_UNSIGNED_CHAR      ,\
    char                :       CYMCTYPE_CHAR               ,\
    signed char         :       CYMCTYPE_SIGNED_CHAR        ,\
    unsigned short      :       CYMCTYPE_UNSIGNED_SHORT     ,\
    short               :       CYMCTYPE_SHORT              ,\
    unsigned int        :       CYMCTYPE_UNSIGNED_INT       ,\
    int                 :       CYMCTYPE_INT                ,\
    float               :       CYMCTYPE_FLOAT              ,\
    unsigned long       :       CYMCTYPE_UNSIGNED_LONG      ,\
    long                :       CYMCTYPE_LONG               ,\
    double              :       CYMCTYPE_DOUBLE             ,\
    unsigned long long  :       CYMCTYPE_UNSIGNED_LONG_LONG ,\
    long long           :       CYMCTYPE_LONG_LONG          ,\
    long double         :       CYMCTYPE_LONG_DOUBLE        ,\
    default             :       CYMCTYPE_NONE                \
    )


#ifndef CYMDEF
    
    #define CYMDEF extern

#endif

enum CymAtomTypes{
    CYMATOM_NONE = 0,
    CYMATOM_U8,
    CYMATOM_I8,
    CYMATOM_U16,
    CYMATOM_I16,
    CYMATOM_U32,
    CYMATOM_I32,
    CYMATOM_F32,
    CYMATOM_U64,
    CYMATOM_I64,
    CYMATOM_F64,

    // for counting purposes
    CYMATOM_COUNT
};

enum CymCtypes{
    CYMCTYPE_NONE = 0,
    CYMCTYPE_UNSIGNED_CHAR,
    CYMCTYPE_CHAR,
    CYMCTYPE_SIGNED_CHAR,
    CYMCTYPE_UNSIGNED_SHORT,
    CYMCTYPE_SHORT,
    CYMCTYPE_SIGNED_SHORT = CYMCTYPE_SHORT,
    CYMCTYPE_UNSIGNED_INT,
    CYMCTYPE_INT,
    CYMCTYPE_SIGNED_INT = CYMCTYPE_INT,
    CYMCTYPE_FLOAT,
    CYMCTYPE_UNSIGNED_LONG,
    CYMCTYPE_LONG,
    CYMCTYPE_SIGNED_LONG,
    CYMCTYPE_DOUBLE,
    CYMCTYPE_UNSIGNED_LONG_LONG,
    CYMCTYPE_LONG_LONG,
    CYMCTYPE_SIGNED_LONG_LONG,
    CYMCTYPE_LONG_DOUBLE,
    CYMCTYPE_PTR,
    CYMCTYPE_STR,
    CYMCTYPE_SIZE_T,

    // for counting purposes
    CYMCTYPE_COUNT
};

enum CymbolTypes{
    CYMBOL_NONE = 0,
    CYMBOL_MEMBLOCK,
    CYMBOL_PIECEWISE_MEMBLOCK,
    CYMBOL_CYMBOL_WRAPPER,

    CYMBOL_CSTR,
    CYMBOL_BIN,

    CYMBOL_CUSTOM,

    CYMBOL_TYPE_COUNT
};

typedef struct Cymbol Cymbol;

#ifdef __cplusplus
extern "C" {
#endif

// \returns the size of an atom_type given in CymAtomTypes enum in bytes, or 0 if the passed atom_type is invalid or does not exist
CYMDEF size_t cym_atom_size(int atom_type);

// \returns the size of a ctype given in CymCtypes enum in bytes, or 0 if the passed type is invalid or not supported
CYMDEF size_t cym_ctype_size(int ctype);

CYMDEF int cym_atom_from_ctype(int ctype);

CYMDEF int cym_ctype_from_atom(int atom_type);

// if compare_whole_cstr then only if the format matches completely a format cstr will the corresponding format id be returned,
// otherwise a format id will be returned even if a format cstr only matches format up to where format cstr ends.
// \returns a number identifier given in CymAtomTypes enum that corresponds to the format in the string
CYMDEF int cym_get_atomtype_from_format(const char* format, int compare_whole_cstr);

// if compare_whole_cstr then only if the format matches completely a format cstr will the corresponding format id be returned,
// otherwise a format id will be returned even if a format cstr only matches format up to where format cstr ends.
// \returns a number identifier given in CymCtypes enum that corresponds to the format in the string
CYMDEF int cym_get_ctype_from_format(const char* format, int compare_whole_cstr);

CYMDEF const char* cym_atomtype_str(int atom_type);

CYMDEF const char* cym_ctype_str(int atom_type);

CYMDEF const char* cym_cymbol_type_str(int cymbol_type);

/*
    Packs a sequence of chunks of data into dest.
    For each sub chunk to read 2 arguments are required, the size of the chunk in bytes (size_t typed) and a pointer
    to write the respective sub chunk to (in this order), 0 valued argument marks the end of the variadics.
    Check the CYM_UNPACK_MEMPTR macro for auto expanding arguments to this function.
    \returns a pointer to the end of the last written chunk in dest
*/
CYMDEF void* _cym_pack_data(void* dest, ...);

#define cym_pack_data(dest, ...) _cym_pack_data(dest, __VA_ARGS__, (size_t) 0, NULL)

/*
    Takes a pointer to a chunk of data (src) and unpacks/reads sub chunks of specified sizes to specified pointers.
    For each sub chunk to read 2 arguments are required, the size of the sub chunk in bytes (size_t typed) and a pointer
    to write the respective sub chunk to (in this order), the sub chunks will be read in the order
    that the arguments are given, 0 valued argument marks the end of the variadics.
    Check the CYM_UNPACK_MEMPTR macro for auto expanding arguments to this function.
    \returns a pointer to the end of the last read sub chunk in src, or NULL on failure.
*/
CYMDEF void* _cym_unpack_data(const void* src, ...);

#define cym_unpack_data(src, ...) _cym_unpack_data(src, __VA_ARGS__, (size_t) 0, NULL)

/*
    Packs a sequence of values into dest.
    values are passed through variadics, where the types are given through the formated string.
    \returns a pointer to the end of the last written chunk in dest
*/
CYMDEF void* cym_pack_values(void* dest, const char* __format, ...);

/*
    Unpacks a sequence of values from src to passed pointers.
    pointers are passed through variadics, where the types are given through the formated string.
    \returns a pointer to the end of the last read chunk in src
*/
CYMDEF void* cym_unpack_values(const void* src, const char* __format, ...);

// same as cym_unpack_data, but for unpacking from a stream
CYMDEF size_t _cym_sunpack_data(void* stream, size_t(*stream_read)(void* dest, size_t _size, size_t n, void* stream), ...);

#define cym_sunpack_data(stream, stream_read, ...) _cym_sunpack_data(stream, stream_read, __VA_ARGS__, (size_t) 0, NULL)

// same as cym_unpack_data, but for packing to a stream
CYMDEF size_t _cym_spack_data(void* stream, size_t(*stream_write)(const void* src, size_t _size, size_t n, void* stream), ...);

#define cym_spack_data(stream, stream_write, ...) _cym_spack_data(stream, stream_write, __VA_ARGS__, (size_t) 0, NULL)

// same as cym_unpack_values, but for unpacking from a stream
CYMDEF size_t cym_sunpack_values(void* stream, size_t(*stream_read)(void* dest, size_t _size, size_t n, void* stream),
    const char* format, ...);

// same as cym_pack_data, but for packing to a stream
CYMDEF size_t cym_spack_values(void* stream, size_t(*stream_write)(const void* src, size_t _size, size_t n, void* stream),
    const char* format, ...);


#ifdef CYMBOL_IMPLEMENTATION // beginning of function implementations ========================================================


static inline int cym_compare_str(const char* str1, const char* str2, int only_compare_untill_first_null){

    if(!str1 || !str2) return 0;
    size_t i = 0;
    for(; str1[i] && (str1[i] == str2[i]); i+=1);
    return (str1[i] == str2[i]) || (only_compare_untill_first_null && (!str1[i] || !str2[i]));
}

CYMDEF size_t cym_atom_size(int atom_type){
    switch (atom_type)
    {
    case CYMATOM_U8:
    case CYMATOM_I8:
        return 1;
    case CYMATOM_U16:
    case CYMATOM_I16:
        return 2;
    case CYMATOM_U32:
    case CYMATOM_I32:
    case CYMATOM_F32:
        return 3;
    case CYMATOM_U64:
    case CYMATOM_I64:
    case CYMATOM_F64:
        return 4;
    
    default:
        return 0;
    }

    return 0;
}

CYMDEF size_t cym_ctype_size(int ctype){
    switch (ctype)
    {
    case CYMCTYPE_UNSIGNED_CHAR:        return sizeof(unsigned char);
    case CYMCTYPE_CHAR:                 return sizeof(char);
    case CYMCTYPE_SIGNED_CHAR:          return sizeof(signed char);
    case CYMCTYPE_UNSIGNED_SHORT:       return sizeof(unsigned short);
    case CYMCTYPE_SHORT:                return sizeof(short);
    case CYMCTYPE_UNSIGNED_INT:         return sizeof(unsigned int);
    case CYMCTYPE_INT:                  return sizeof(int);
    case CYMCTYPE_FLOAT:                return sizeof(float);
    case CYMCTYPE_UNSIGNED_LONG:        return sizeof(unsigned long);
    case CYMCTYPE_LONG:                 return sizeof(long);
    case CYMCTYPE_SIGNED_LONG:          return sizeof(signed long);
    case CYMCTYPE_DOUBLE:               return sizeof(double);
    case CYMCTYPE_UNSIGNED_LONG_LONG:   return sizeof(unsigned long long);
    case CYMCTYPE_LONG_LONG:            return sizeof(long long);
    case CYMCTYPE_SIGNED_LONG_LONG:     return sizeof(signed long long);
    case CYMCTYPE_LONG_DOUBLE:          return sizeof(long double);
    case CYMCTYPE_PTR:                  return sizeof(void*);
    case CYMCTYPE_STR:                  return sizeof(char*);
    case CYMCTYPE_SIZE_T:               return sizeof(size_t);
    
    default:
        return 0;
    }
    return 0;
}

CYMDEF int cym_atom_from_ctype(int ctype){
    switch (ctype)
    {
    case CYMCTYPE_UNSIGNED_CHAR:        return CYM_TYPE_TO_ATOM_MAP(unsigned char);
    case CYMCTYPE_CHAR:                 return CYM_TYPE_TO_ATOM_MAP(char);
    case CYMCTYPE_SIGNED_CHAR:          return CYM_TYPE_TO_ATOM_MAP(signed char);
    case CYMCTYPE_UNSIGNED_SHORT:       return CYM_TYPE_TO_ATOM_MAP(unsigned short);
    case CYMCTYPE_SHORT:                return CYM_TYPE_TO_ATOM_MAP(short);
    case CYMCTYPE_UNSIGNED_INT:         return CYM_TYPE_TO_ATOM_MAP(unsigned int);
    case CYMCTYPE_INT:                  return CYM_TYPE_TO_ATOM_MAP(int);
    case CYMCTYPE_FLOAT:                return CYM_TYPE_TO_ATOM_MAP(float);
    case CYMCTYPE_UNSIGNED_LONG:        return CYM_TYPE_TO_ATOM_MAP(unsigned long);
    case CYMCTYPE_LONG:                 return CYM_TYPE_TO_ATOM_MAP(long);
    case CYMCTYPE_SIGNED_LONG:          return CYM_TYPE_TO_ATOM_MAP(signed long);
    case CYMCTYPE_DOUBLE:               return CYM_TYPE_TO_ATOM_MAP(double);
    case CYMCTYPE_UNSIGNED_LONG_LONG:   return CYM_TYPE_TO_ATOM_MAP(unsigned long long);
    case CYMCTYPE_LONG_LONG:            return CYM_TYPE_TO_ATOM_MAP(long long);
    case CYMCTYPE_SIGNED_LONG_LONG:     return CYM_TYPE_TO_ATOM_MAP(signed long long);
    case CYMCTYPE_LONG_DOUBLE:          return CYM_TYPE_TO_ATOM_MAP(long double);
    case CYMCTYPE_PTR:                  return CYM_TYPE_TO_ATOM_MAP(void*);
    case CYMCTYPE_STR:                  return CYM_TYPE_TO_ATOM_MAP(char*);
    case CYMCTYPE_SIZE_T:               return CYM_TYPE_TO_ATOM_MAP(size_t);
    default:                            return CYMATOM_NONE;
    }
}

CYMDEF int cym_ctype_from_atom(int atom_type){
    switch (atom_type)
    {
    case CYMATOM_U8:    return CYM_TYPE_TO_CTYPE_MAP(uint8_t) ;
    case CYMATOM_I8:    return CYM_TYPE_TO_CTYPE_MAP(int8_t)  ;
    case CYMATOM_U16:   return CYM_TYPE_TO_CTYPE_MAP(uint16_t);
    case CYMATOM_I16:   return CYM_TYPE_TO_CTYPE_MAP(int16_t) ;
    case CYMATOM_U32:   return CYM_TYPE_TO_CTYPE_MAP(uint32_t);
    case CYMATOM_I32:   return CYM_TYPE_TO_CTYPE_MAP(int32_t) ;
    case CYMATOM_F32:   return CYM_TYPE_TO_CTYPE_MAP(float)   ;
    case CYMATOM_U64:   return CYM_TYPE_TO_CTYPE_MAP(uint64_t);
    case CYMATOM_I64:   return CYM_TYPE_TO_CTYPE_MAP(int64_t) ;
    case CYMATOM_F64:   return CYM_TYPE_TO_CTYPE_MAP(double)  ;
    default:            return CYMCTYPE_NONE;
    }
}

CYMDEF const char* cym_parse_format_preffixes(const char* format, size_t* before_dot, size_t* after_dot, int* asterix){

    if(!format) return NULL;

    while (*format == ' ') format+=1;
    
    int a = 0;
    size_t _count = 0;
    size_t ssize = SIZE_MAX;

    if(*format == '*'){
        format+=1;
        a |= 1;
    }
    else if(before_dot){
        
        if(*format > '9' || *format < '0')
            _count = 1;
        else for(; *format <= '9' && *format >= '0'; format+=1){
            _count = (_count * 10) + (size_t) (*format - '0');
        }
        *before_dot = _count;
    } else for(; *format <= '9' && *format >= '0'; format+=1);

    if(*format == '.'){
        if(*(++format) == '*'){
            a |= 2;
            format+=1;
        }
        else if(after_dot){
            ssize = 0;
            for(; *format <= '9' && *format >= '0'; format+=1){
                ssize = (ssize * 10) + (size_t) (*format - '0');
            }
            *after_dot = ssize;
        } else for(; *format <= '9' && *format >= '0'; format+=1);
    }
    else if(after_dot) *after_dot = ssize;

    if(asterix) *asterix = a;

    return format;
}

CYMDEF int cym_get_atomtype_from_format(const char* format, int compare_whole_cstr){

    if(cym_compare_str(format, PRIu8,  !compare_whole_cstr))    return CYMATOM_U8;
    if(cym_compare_str(format, PRIi8,  !compare_whole_cstr))    return CYMATOM_I8;
    if(cym_compare_str(format, PRIu16, !compare_whole_cstr))    return CYMATOM_U16;
    if(cym_compare_str(format, PRIi16, !compare_whole_cstr))    return CYMATOM_I16;
    if(cym_compare_str(format, PRIu32, !compare_whole_cstr))    return CYMATOM_U32;
    if(cym_compare_str(format, PRIi32, !compare_whole_cstr))    return CYMATOM_I32;
    if(cym_compare_str(format, "f",    !compare_whole_cstr))    return CYMATOM_F32;   
    if(cym_compare_str(format, PRIu64, !compare_whole_cstr))    return CYMATOM_U64;
    if(cym_compare_str(format, PRIi64, !compare_whole_cstr))    return CYMATOM_I64;
    if(cym_compare_str(format, "lf",   !compare_whole_cstr))    return CYMATOM_F64;
    
    return CYMATOM_NONE;
}



CYMDEF int cym_get_ctype_from_format(const char* format, int compare_whole_cstr){

    if(cym_compare_str(format, "hhu", !compare_whole_cstr))     return CYMCTYPE_UNSIGNED_CHAR;
    if(cym_compare_str(format, "c",   !compare_whole_cstr))     return CYMCTYPE_CHAR;
    if(cym_compare_str(format, "hhd", !compare_whole_cstr))     return CYMCTYPE_SIGNED_CHAR;
    if(cym_compare_str(format, "hhi", !compare_whole_cstr))     return CYMCTYPE_SIGNED_CHAR;

    if(cym_compare_str(format, "hd", !compare_whole_cstr))      return CYMCTYPE_SIGNED_SHORT;
    if(cym_compare_str(format, "hi", !compare_whole_cstr))      return CYMCTYPE_SHORT;
    if(cym_compare_str(format, "hu", !compare_whole_cstr))      return CYMCTYPE_UNSIGNED_SHORT;

    if(cym_compare_str(format, "u", !compare_whole_cstr))       return CYMCTYPE_UNSIGNED_INT;
    if(cym_compare_str(format, "i", !compare_whole_cstr))       return CYMCTYPE_INT;
    if(cym_compare_str(format, "d", !compare_whole_cstr))       return CYMCTYPE_SIGNED_INT;
    if(cym_compare_str(format, "x", !compare_whole_cstr))       return CYMCTYPE_INT;
    if(cym_compare_str(format, "X", !compare_whole_cstr))       return CYMCTYPE_INT;
    if(cym_compare_str(format, "o", !compare_whole_cstr))       return CYMCTYPE_INT;
    if(cym_compare_str(format, "O", !compare_whole_cstr))       return CYMCTYPE_INT;

    if(cym_compare_str(format, "f",     !compare_whole_cstr))   return CYMCTYPE_FLOAT;
    if(cym_compare_str(format, "lf",    !compare_whole_cstr))   return CYMCTYPE_DOUBLE;
    if(cym_compare_str(format, "Lf",    !compare_whole_cstr))   return CYMCTYPE_LONG_DOUBLE;

    if(cym_compare_str(format, "lu", !compare_whole_cstr))      return CYMCTYPE_UNSIGNED_LONG;
    if(cym_compare_str(format, "ld", !compare_whole_cstr))      return CYMCTYPE_SIGNED_LONG;
    if(cym_compare_str(format, "li", !compare_whole_cstr))      return CYMCTYPE_LONG;

    if(cym_compare_str(format, "llu", !compare_whole_cstr))     return CYMCTYPE_UNSIGNED_LONG_LONG;
    if(cym_compare_str(format, "lld", !compare_whole_cstr))     return CYMCTYPE_SIGNED_LONG_LONG;
    if(cym_compare_str(format, "lli", !compare_whole_cstr))     return CYMCTYPE_LONG_LONG;

    if(cym_compare_str(format, "p",   !compare_whole_cstr))     return CYMCTYPE_PTR;
    if(cym_compare_str(format, "s",   !compare_whole_cstr))     return CYMCTYPE_STR;

    if(cym_compare_str(format, "zu",  !compare_whole_cstr))     return CYMCTYPE_SIZE_T;
    
    return CYMCTYPE_NONE;
}

CYMDEF const char* cym_atomtype_str(int atom_type){
    switch (atom_type)
    {
    case CYMATOM_U8:    return "CYMATOM_U8" ;
    case CYMATOM_I8:    return "CYMATOM_I8" ;
    case CYMATOM_U16:   return "CYMATOM_U16";
    case CYMATOM_I16:   return "CYMATOM_I16";
    case CYMATOM_U32:   return "CYMATOM_U32";
    case CYMATOM_I32:   return "CYMATOM_I32";
    case CYMATOM_F32:   return "CYMATOM_F32";
    case CYMATOM_U64:   return "CYMATOM_U64";
    case CYMATOM_I64:   return "CYMATOM_I64";
    case CYMATOM_F64:   return "CYMATOM_F64";
    default:            return "CYMATOM_UNKNOWN";
    }
}

CYMDEF const char* cym_ctype_str(int ctype){
    switch (ctype)
    {
    case CYMCTYPE_UNSIGNED_CHAR:        return "CYMCTYPE_UNSIGNED_CHAR"     ; 
    case CYMCTYPE_CHAR:                 return "CYMCTYPE_CHAR"              ;
    case CYMCTYPE_SIGNED_CHAR:          return "CYMCTYPE_SIGNED_CHAR"       ; 
    case CYMCTYPE_UNSIGNED_SHORT:       return "CYMCTYPE_UNSIGNED_SHORT"    ; 
    case CYMCTYPE_SHORT:                return "CYMCTYPE_SHORT"             ;
    case CYMCTYPE_UNSIGNED_INT:         return "CYMCTYPE_UNSIGNED_INT"      ; 
    case CYMCTYPE_INT:                  return "CYMCTYPE_INT"               ;
    case CYMCTYPE_FLOAT:                return "CYMCTYPE_FLOAT"             ;
    case CYMCTYPE_UNSIGNED_LONG:        return "CYMCTYPE_UNSIGNED_LONG"     ; 
    case CYMCTYPE_LONG:                 return "CYMCTYPE_LONG"              ;
    case CYMCTYPE_SIGNED_LONG:          return "CYMCTYPE_SIGNED_LONG"       ; 
    case CYMCTYPE_DOUBLE:               return "CYMCTYPE_DOUBLE"            ;
    case CYMCTYPE_UNSIGNED_LONG_LONG:   return "CYMCTYPE_UNSIGNED_LONG_LONG"; 
    case CYMCTYPE_LONG_LONG:            return "CYMCTYPE_LONG_LONG"         ; 
    case CYMCTYPE_SIGNED_LONG_LONG:     return "CYMCTYPE_SIGNED_LONG_LONG"  ; 
    case CYMCTYPE_LONG_DOUBLE:          return "CYMCTYPE_LONG_DOUBLE"       ; 
    case CYMCTYPE_PTR:                  return "CYMCTYPE_PTR"               ; 
    case CYMCTYPE_STR:                  return "CYMCTYPE_STR"               ; 
    case CYMCTYPE_SIZE_T:               return "CYMCTYPE_SIZE_T"            ; 
    default:                            return "CTYPE_UNKNOWN"              ;
    }
}

CYMDEF const char* cym_cymbol_type_str(int cymbol_type){
    switch (cymbol_type)
    {
    case CYMBOL_NONE:   return "CYMBOL_NONE";
    default:            return "CYMBOL_UNKNOWN";
    }
}

CYMDEF void* _cym_pack_data(void* dest, ...){

    va_list args;
    va_start(args, dest);

    const void*  src  = NULL;

    for(size_t size = va_arg(args, size_t); size; size = va_arg(args, size_t)){

        src = va_arg(args, void*);
        if(!dest) break;

        CYM_MEMCPY(dest, src, size);
        dest = (uint8_t*)(dest) + size;
    }
    

    va_end(args);
    return dest;
}


CYMDEF void* _cym_unpack_data(const void* src, ...){

    va_list args;
    va_start(args, src);

    
    void*  dest  = NULL;

    for(size_t size = va_arg(args, size_t); size; size = va_arg(args, size_t)){

        dest = va_arg(args, void*);
        if(!dest) break;

        CYM_MEMCPY(dest, src, size);
        src = (uint8_t*)(src) + size;
    }
    

    va_end(args);
    return (void*) src;
}

CYMDEF void* cym_pack_values(void* dest, const char* __format, ...){

    #define ICYM_PACK_WRAPPER(TYPE) while(before_dot--) {\
        const TYPE d = va_arg(args, TYPE);\
        CYM_MEMCPY(dest, &d, sizeof(d));\
        dest = (uint8_t*)(dest) + sizeof(d);\
    }
    #define ICYM_PACK_WRAPPER_EX(TYPE0, TYPE1) while(before_dot--) {\
        const TYPE1 d = (TYPE1) va_arg(args, TYPE0);\
        CYM_MEMCPY(dest, &d, sizeof(d));\
        dest = (uint8_t*)(dest) + sizeof(d);\
    }

    va_list args;
    va_start(args, __format);

    for(; *__format; __format+=1){

        if(*__format == '%'){
            size_t before_dot = 0;
            size_t after_dot  = 0;
            int    asterix    = 0;

            __format = cym_parse_format_preffixes(__format + 1, &before_dot, &after_dot, &asterix);

            if(asterix & 1) before_dot = (size_t) va_arg(args, int);
            if(asterix & 2) after_dot  = (size_t) va_arg(args, int);

            const int _type = cym_get_ctype_from_format(__format, 0);

            switch (_type)
            {
            case CYMCTYPE_UNSIGNED_CHAR:        ICYM_PACK_WRAPPER_EX(int, unsigned char);   break;
            case CYMCTYPE_CHAR:                 ICYM_PACK_WRAPPER_EX(int, char);            break;
            case CYMCTYPE_SIGNED_CHAR:          ICYM_PACK_WRAPPER_EX(int, signed char);     break;
            case CYMCTYPE_UNSIGNED_SHORT:       ICYM_PACK_WRAPPER_EX(int, unsigned short);  break;
            case CYMCTYPE_SHORT:                ICYM_PACK_WRAPPER_EX(int, short);           break;
            case CYMCTYPE_INT:                  ICYM_PACK_WRAPPER(int);                     break;
            case CYMCTYPE_UNSIGNED_INT:         ICYM_PACK_WRAPPER(unsigned int);            break;
            case CYMCTYPE_FLOAT:                ICYM_PACK_WRAPPER_EX(double, float);        break;
            case CYMCTYPE_DOUBLE:               ICYM_PACK_WRAPPER(double);                  break;
            case CYMCTYPE_LONG_DOUBLE:          ICYM_PACK_WRAPPER(long double);             break;
            case CYMCTYPE_UNSIGNED_LONG:        ICYM_PACK_WRAPPER(unsigned long);           break;
            case CYMCTYPE_LONG:                 ICYM_PACK_WRAPPER(long);                    break;
            case CYMCTYPE_SIGNED_LONG:          ICYM_PACK_WRAPPER(signed long);             break;
            case CYMCTYPE_UNSIGNED_LONG_LONG:   ICYM_PACK_WRAPPER(unsigned long long);      break;
            case CYMCTYPE_LONG_LONG:            ICYM_PACK_WRAPPER(long long);               break;
            case CYMCTYPE_SIGNED_LONG_LONG:     ICYM_PACK_WRAPPER(signed long long);        break;
            case CYMCTYPE_PTR:                  ICYM_PACK_WRAPPER(void*);                   break;
            case CYMCTYPE_STR:{
                while(before_dot--){
                    char* dest_str = (char*) dest;
                    const char* str = va_arg(args, const char*);
                    size_t size = after_dot;
                    while(size-- && *str){
                        *dest_str = *str;
                        dest_str+=1;
                        str+=1;
                    }
                    *dest_str = '\0';
                    dest = (void*) (dest_str + 1);
                }
            }   break;
            case CYMCTYPE_SIZE_T:               ICYM_PACK_WRAPPER(size_t);                  break;
            
            default:                            break;                  
            }
            continue;
        }
    }

    #undef ICYM_PACK_WRAPPER
    #undef ICYM_PACK_WRAPPER_EX
    va_end(args);
    return dest;
}

CYMDEF void* cym_unpack_values(const void* src, const char* __format, ...){

    va_list args;
    va_start(args, __format);

    #define ICYM_UNPACK_WRAPPER(TYPE) while(before_dot--){\
        TYPE* const d = va_arg(args, TYPE*);\
        CYM_MEMCPY(d, src, sizeof(*d));\
        src = (uint8_t*)(src) + sizeof(*d);\
    }
    

    for(; *__format; __format+=1){

        if(*__format == '%'){
            size_t before_dot = 0;
            size_t after_dot  = 0;
            int    asterix    = 0;

            __format = cym_parse_format_preffixes(__format + 1, &before_dot, &after_dot, &asterix);

            if(asterix & 1) before_dot = (size_t) va_arg(args, int);
            if(asterix & 2) after_dot  = (size_t) va_arg(args, int);

            const int _type = cym_get_ctype_from_format(__format, 0);
            if(_type == CYMCTYPE_NONE) continue;

            switch (_type)
            {
            case CYMCTYPE_NONE:                 break;
            case CYMCTYPE_UNSIGNED_CHAR:        ICYM_UNPACK_WRAPPER(unsigned char);         break;
            case CYMCTYPE_CHAR:                 ICYM_UNPACK_WRAPPER(char);                  break;
            case CYMCTYPE_SIGNED_CHAR:          ICYM_UNPACK_WRAPPER(signed char);           break;
            case CYMCTYPE_UNSIGNED_SHORT:       ICYM_UNPACK_WRAPPER(unsigned short);        break;
            case CYMCTYPE_SHORT:                ICYM_UNPACK_WRAPPER(short);                 break;
            case CYMCTYPE_UNSIGNED_INT:         ICYM_UNPACK_WRAPPER(unsigned int);          break;
            case CYMCTYPE_INT:                  ICYM_UNPACK_WRAPPER(int);                   break;
            case CYMCTYPE_FLOAT:                ICYM_UNPACK_WRAPPER(float);                 break;
            case CYMCTYPE_DOUBLE:               ICYM_UNPACK_WRAPPER(double);                break;
            case CYMCTYPE_LONG_DOUBLE:          ICYM_UNPACK_WRAPPER(long double);           break;
            case CYMCTYPE_UNSIGNED_LONG:        ICYM_UNPACK_WRAPPER(unsigned long);         break;
            case CYMCTYPE_LONG:                 ICYM_UNPACK_WRAPPER(long);                  break;
            case CYMCTYPE_SIGNED_LONG:          ICYM_UNPACK_WRAPPER(signed long);           break;
            case CYMCTYPE_UNSIGNED_LONG_LONG:   ICYM_UNPACK_WRAPPER(unsigned long long);    break;
            case CYMCTYPE_LONG_LONG:            ICYM_UNPACK_WRAPPER(long long);             break;
            case CYMCTYPE_SIGNED_LONG_LONG:     ICYM_UNPACK_WRAPPER(signed long long);      break;
            case CYMCTYPE_PTR:                  ICYM_UNPACK_WRAPPER(void*);                 break;
            case CYMCTYPE_STR:{
                while(before_dot--){
                    const char* src_str = (char*) src;
                    char* dest = va_arg(args, char*);
                    size_t size = after_dot;
                    while(size-- && *src_str){
                        *dest = *src_str;
                        dest+=1;
                        src_str+=1;
                    }
                    *dest = '\0';
                    src = (void*) (src_str + 1);
                }
            }   break;
            case CYMCTYPE_SIZE_T:               ICYM_UNPACK_WRAPPER(size_t);                break;            
            
            default:                            break;
            }
        }
    }
    
    #undef ICYM_UNPACK_WRAPPER
    va_end(args);
    return (void*) src;
}


CYMDEF size_t _cym_sunpack_data(void* stream, size_t(*stream_read)(void* dest, size_t _size, size_t n, void* stream), ...){
    va_list args;
    va_start(args, stream_read);

    size_t read = 0;
    void*  dest  = NULL;

    for(size_t size = va_arg(args, size_t); size; size = va_arg(args, size_t)){

        dest = va_arg(args, void*);
        if(!dest) break;

        read += stream_read(dest, 1, size, stream);
    }
    

    va_end(args);
    return read;
}

CYMDEF size_t _cym_spack_data(void* stream, size_t(*stream_write)(const void* src, size_t _size, size_t n, void* stream), ...){
    va_list args;
    va_start(args, stream_write);

    size_t written = 0;
    void*  src  = NULL;

    for(size_t size = va_arg(args, size_t); size; size = va_arg(args, size_t)){

        src = va_arg(args, void*);
        if(!src) break;

        written += stream_write(src, 1, size, stream);
    }
    

    va_end(args);
    return written;
}


CYMDEF size_t cym_sunpack_values(void* stream, size_t(*stream_read)(void* dest, size_t _size, size_t n, void* stream),
    const char* format, ...){
    va_list args;
    va_start(args, format);

    #define ICYM_UNPACK_WRAPPER(TYPE) while(before_dot--){\
        TYPE* const d = va_arg(args, TYPE*);\
        read += stream_read(d, 1, sizeof(*d), stream);\
    }

    size_t read = 0;
    void*  dest  = NULL;

    for(; *format; format+=1){

        if(*format == '%'){
            size_t before_dot;
            size_t after_dot;
            int    asterix;

            format = cym_parse_format_preffixes(format + 1, &before_dot, &after_dot, &asterix);
            if(asterix & 1) before_dot = (size_t) va_arg(args, int);
            if(asterix & 2) after_dot  = (size_t) va_arg(args, int);

            const int _type = cym_get_ctype_from_format(format, 0);
            if(_type == CYMCTYPE_NONE) continue;

            switch (_type)
            {
            case CYMCTYPE_NONE:                 break;
            case CYMCTYPE_UNSIGNED_CHAR:        ICYM_UNPACK_WRAPPER(unsigned char);         break;
            case CYMCTYPE_CHAR:                 ICYM_UNPACK_WRAPPER(char);                  break;
            case CYMCTYPE_SIGNED_CHAR:          ICYM_UNPACK_WRAPPER(signed char);           break;
            case CYMCTYPE_UNSIGNED_SHORT:       ICYM_UNPACK_WRAPPER(unsigned short);        break;
            case CYMCTYPE_SHORT:                ICYM_UNPACK_WRAPPER(short);                 break;
            case CYMCTYPE_UNSIGNED_INT:         ICYM_UNPACK_WRAPPER(unsigned int);          break;
            case CYMCTYPE_INT:                  ICYM_UNPACK_WRAPPER(int);                   break;
            case CYMCTYPE_FLOAT:                ICYM_UNPACK_WRAPPER(float);                 break;
            case CYMCTYPE_DOUBLE:               ICYM_UNPACK_WRAPPER(double);                break;
            case CYMCTYPE_LONG_DOUBLE:          ICYM_UNPACK_WRAPPER(long double);           break;
            case CYMCTYPE_UNSIGNED_LONG:        ICYM_UNPACK_WRAPPER(unsigned long);         break;
            case CYMCTYPE_LONG:                 ICYM_UNPACK_WRAPPER(long);                  break;
            case CYMCTYPE_SIGNED_LONG:          ICYM_UNPACK_WRAPPER(signed long);           break;
            case CYMCTYPE_UNSIGNED_LONG_LONG:   ICYM_UNPACK_WRAPPER(unsigned long long);    break;
            case CYMCTYPE_LONG_LONG:            ICYM_UNPACK_WRAPPER(long long);             break;
            case CYMCTYPE_SIGNED_LONG_LONG:     ICYM_UNPACK_WRAPPER(signed long long);      break;
            case CYMCTYPE_PTR:                  ICYM_UNPACK_WRAPPER(void*);                 break;
            case CYMCTYPE_STR:{
                while(before_dot--){
                    char* dest = va_arg(args, char*);
                    size_t size = after_dot;
                    for(char c = 0; stream_read(&c, 1, sizeof(c), stream) == sizeof(c) && c && size; size -= 1){
                        *(dest++) = c;
                        read += sizeof(c);
                    }
                    *(dest++) = '\0';                        
                }
            }   break;
            case CYMCTYPE_SIZE_T:               ICYM_UNPACK_WRAPPER(size_t);                break;            
            
            default:                            break;
            }
            continue;
        }
    }
    
    #undef ICYM_UNPACK_WRAPPER
    va_end(args);
    return read;
}

CYMDEF size_t cym_spack_values(void* stream, size_t(*stream_write)(const void* src, size_t _size, size_t n, void* stream),
    const char* format, ...){
    #define ICYM_PACK_WRAPPER(TYPE) while(before_dot--) {\
        const TYPE d = va_arg(args, TYPE);\
        written += stream_write(&d, 1, sizeof(d), stream);\
    }
    #define ICYM_PACK_WRAPPER_EX(TYPE0, TYPE1) while(before_dot--) {\
        const TYPE1 d = (TYPE1) va_arg(args, TYPE0);\
        written += stream_write(&d, 1, sizeof(d), stream);\
    }

    va_list args;
    va_start(args, format);

    size_t written = 0;

    for(; *format; format+=1){

        if(*format == '%'){
            size_t before_dot;
            size_t after_dot;
            int    asterix;

            format = cym_parse_format_preffixes(format + 1, &before_dot, &after_dot, &asterix);

            if(asterix & 1) before_dot = (size_t) va_arg(args, int);
            if(asterix & 2) after_dot  = (size_t) va_arg(args, int);

            const int _type = cym_get_ctype_from_format(format, 0);

            switch (_type)
            {
            case CYMCTYPE_UNSIGNED_CHAR:        ICYM_PACK_WRAPPER_EX(int, unsigned char);   break;
            case CYMCTYPE_CHAR:                 ICYM_PACK_WRAPPER_EX(int, char);            break;
            case CYMCTYPE_SIGNED_CHAR:          ICYM_PACK_WRAPPER_EX(int, signed char);     break;
            case CYMCTYPE_UNSIGNED_SHORT:       ICYM_PACK_WRAPPER_EX(int, unsigned short);  break;
            case CYMCTYPE_SHORT:                ICYM_PACK_WRAPPER_EX(int, short);           break;
            case CYMCTYPE_INT:                  ICYM_PACK_WRAPPER(int);                     break;
            case CYMCTYPE_UNSIGNED_INT:         ICYM_PACK_WRAPPER(unsigned int);            break;
            case CYMCTYPE_FLOAT:                ICYM_PACK_WRAPPER_EX(double, float);        break;
            case CYMCTYPE_DOUBLE:               ICYM_PACK_WRAPPER(double);                  break;
            case CYMCTYPE_LONG_DOUBLE:          ICYM_PACK_WRAPPER(long double);             break;
            case CYMCTYPE_UNSIGNED_LONG:        ICYM_PACK_WRAPPER(unsigned long);           break;
            case CYMCTYPE_LONG:                 ICYM_PACK_WRAPPER(long);                    break;
            case CYMCTYPE_SIGNED_LONG:          ICYM_PACK_WRAPPER(signed long);             break;
            case CYMCTYPE_UNSIGNED_LONG_LONG:   ICYM_PACK_WRAPPER(unsigned long long);      break;
            case CYMCTYPE_LONG_LONG:            ICYM_PACK_WRAPPER(long long);               break;
            case CYMCTYPE_SIGNED_LONG_LONG:     ICYM_PACK_WRAPPER(signed long long);        break;
            case CYMCTYPE_PTR:                  ICYM_PACK_WRAPPER(void*);                   break;
            case CYMCTYPE_STR:{
                while(before_dot--){
                    const char* str = va_arg(args, const char*);
                    size_t size = 0;
                    for(; str[size]; size+=1);
                    size = (after_dot < size)? after_dot : size;
                    written += stream_write(str, 1, size * sizeof(char), stream);
                    const char c = '\0';
                    written += stream_write(&c, 1, sizeof(c), stream);
                }
            }   break;
            case CYMCTYPE_SIZE_T:               ICYM_PACK_WRAPPER(size_t);                  break;
            
            default:                            break;                  
            }
        }
    }

    #undef ICYM_PACK_WRAPPER
    #undef ICYM_PACK_WRAPPER_EX
    va_end(args);
    return written;
}

#endif // ======================== END OF FUNCTION IMPLEMENTATIONS ==================================================


#ifdef __cplusplus
}
#endif

#endif // =====================  END OF FILE CYMBOL_HEADER ===========================