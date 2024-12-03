#ifndef MC_STRING_HEADER
#define MC_STRING_HEADER

#include "setup.h"



typedef struct Mc_string_t
{
    // not necessarily null terminated
    char* c_str;
    Mc_size_t size;
} Mc_string_t;

// if you only wish to compare the strings up to where the first one terminates, pass _only_compare_till_first_null=1
// otherwise pass _only_compare_till_first_null=0
static inline int mc_compare_str(const char* str1, const char* str2, int _only_compare_till_first_null){
    if(_only_compare_till_first_null){
        for(Mc_uint_t i = 0; str1[i] && str2[i]; i+=1){
            if(str1[i] != str2[i]) return 0;
        }
        return 1;
    }
    
    Mc_uint_t i = 0;
    for( ; str1[i]; i+=1){
        if(str1[i] != str2[i]) return 0;
    }
    return !str2[i];
}

static inline Mc_size_t mc_str_size(const char* str){
    Mc_size_t size = 0;
    for( ; str[size]; size+=1);
    return size;
}

// returns the first found character position relative to the offset, or -1 if none are found
static inline Mc_int_t mc_find_char(const char* str, char c, Mc_int_t offset){
    str += offset;
    for(Mc_int_t i = 0; str[i]; i+=1){
        if(c == str[i]) return i;
    }
    return -1;
}

// works similar to cym_find_char but does so to any of the characters in the charbuff str
static inline Mc_int_t mc_find_chars(const char* str, const char* charbuff, Mc_int_t offset){
    str += offset;
    for(Mc_int_t i = 0; str[i]; i+=1){
        for(Mc_int_t j = 0; charbuff[j]; j+=1){
            if(charbuff[j] == str[i]) return i;
        }
    }
    return -1;
}


static inline int mc_scompare_str(const Mc_string_t str1, const Mc_string_t str2){
    if(str1.size != str2.size){
        return 0;
    }
    for(Mc_size_t i = 0; i < str1.size; i+=1){
        if(str1.c_str[i] != str2.c_str[i]){
            return 0;
        }
    }
    return 1;
}

// returns the first found character position relative to the offset, or -1 if none are found
static inline Mc_int_t mc_sfind_char(Mc_string_t str, char c, Mc_int_t offset){
    str.c_str += offset;
    for(Mc_int_t i = 0; i < str.size; i+=1){
        if(c == str.c_str[i]) return i;
    }
    return -1;
}

// works similar to cym_sfind_char but does so to any of the characters in the charbuff str
static inline Mc_int_t mc_sfind_chars(Mc_string_t str, Mc_string_t charbuffstr, Mc_int_t offset){
    str.c_str += offset;
    for(Mc_int_t i = 0; i < str.size; i+=1){
        for(Mc_int_t j = 0; j < charbuffstr.size; j+=1){
            if(charbuffstr.c_str[j] == str.c_str[i]) return i;
        }
    }
    return -1;
}




#endif // END OF FILE

