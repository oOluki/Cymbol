#ifndef CYMPARSER_HEADER
#define CYMPARSER_HEADER

#include <stdio.h>
#include "myc/__init__.h"

typedef enum UserPromptType
{

    USERPROMPT_NONE = 0,

    USERPROMPT_CLEARVIEW,
    USERPROMPT_CLEARMEM,

    USRPROMPT_CLEARCONTEXT,
    USRPROMPT_CHANGECONTEXT,
    USRPROMPT_LOADCONTEXT,
    USRPROMPT_SAVECONTEXT,
    USRPROMPT_MERGECONTEXTS,
    USRPROMPT_ADDCYM,
    USRPROMPT_DELCYM,
    USRPROMPT_SETCYM,
    USRPROMPT_SHOWCYM

} UserPromptType;


typedef struct UserPrompt
{
    UserPromptType type;
    union Operand
    {
        size_t      as_index;
        void*       as_ptr;
        const char* as_str
    } operand1, operand2, operand3;

    uint64_t flags;
    
} UserPrompt;


uint64_t get_flag(char c){
    switch (c)
    {
    case 'c':
        break;
    
    default:
        break;
    }

    fprintf(stderr, "[ERROR] Invalid Flag '%c'\n", c);

    return 0;
}


UserPrompt parse(size_t token_count, const Mc_token_t* token_buffer){

    UserPrompt output;

    output.flags = 0;
    output.type = USERPROMPT_NONE;
    output.operand1.as_index = 0;
    output.operand2.as_index = 0;
    output.operand3.as_index = 0;

    int err = 0;

    // parsing flags
    for(size_t i = 0; i < token_count; i+=1){
        const char* token_str = mc_get_token_data(token_buffer, token_buffer[i]);

        if(mc_compare_str(token_str, "-", 1)){
            if(i + 1 > token_count){
                fprintf(stderr, "[ERROR] Expected Flag Identifier After '-'\n");
                err = 1;
                continue;
            } else if(mc_str_size(token_str) != 1){
                fprintf(stderr, "[ERROR] Invalid Flag Identifier '%s'\n", token_str);
                err = 1;
                continue;
            }

            const uint64_t flag = get_flag(token_str[0]);

            if(!flag) err = 1;

            output.flags |= flag;

        }
    }


    return output;


}




#endif // =====================  END OF FILE CYMPARSER_HEADER ===========================