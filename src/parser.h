#ifndef CYMPARSER_HEADER
#define CYMPARSER_HEADER

#include <stdio.h>
#include "myc/__init__.h"


#if defined(__linux__) || defined(APPLE)
    #define CLEAR_VIEW system("clear");
#elif defined(_WIN32)
    #define CLEAR_VIEW system("cls");
#else
    #define CLEAR_VIEW printf("\n\n\n\n\n\n\n\n\n\n\n");
#endif


typedef enum UserPromptType
{

    USRPROMPT_NONE = 0,
    USRPROMPT_ERROR,

    USRPROMPT_EXIT,
    USRPROMPT_CLEARVIEW,
    USRPROMPT_CLEARMEM,

    USRPROMPT_ADDCONTEXT,
    USRPROMPT_DELCONTEXT,
    USRPROMPT_CHANGECONTEXT,
    USRPROMPT_LOADCONTEXT,
    USRPROMPT_SAVECONTEXT,
    USRPROMPT_MERGECONTEXTS,
    USRPROMPT_ADDCYM,
    USRPROMPT_DELCYM,
    USRPROMPT_SETCYM,
    USRPROMPT_SHOWCYM

} UserPromptType;

typedef struct UserPromptInst
{
    UserPromptType type;
    uint8_t expected_operands;
} UserPromptInst;

typedef union {
    size_t      as_index;
    void*       as_ptr;
    const char* as_str;
} Operand;

typedef struct UserPrompt
{
    UserPromptType type;
    Operand operand1;
    Operand operand2;
    Operand operand3;
    Operand operand4;
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


UserPromptInst get_prompt_inst(const char* prompt_inst){

    if(mc_compare_str(prompt_inst, "exit", 0))  return (UserPromptInst){USRPROMPT_EXIT, 0};
    if(mc_compare_str(prompt_inst, "cv", 0))    return (UserPromptInst){USRPROMPT_CLEARVIEW, 0};
    if(mc_compare_str(prompt_inst, "cm", 0))    return (UserPromptInst){USRPROMPT_CLEARMEM, 0};
    if(mc_compare_str(prompt_inst, "nc", 0))    return (UserPromptInst){USRPROMPT_ADDCONTEXT, 0};
    if(mc_compare_str(prompt_inst, "delc", 0))  return (UserPromptInst){USRPROMPT_DELCONTEXT, 0};
    if(mc_compare_str(prompt_inst, "chc", 0))   return (UserPromptInst){USRPROMPT_CHANGECONTEXT, 1};
    if(mc_compare_str(prompt_inst, "load", 0))  return (UserPromptInst){USRPROMPT_LOADCONTEXT, 1};
    if(mc_compare_str(prompt_inst, "save", 0))  return (UserPromptInst){USRPROMPT_SAVECONTEXT, 1};
    if(mc_compare_str(prompt_inst, "merge", 0)) return (UserPromptInst){USRPROMPT_MERGECONTEXTS, 1};
    if(mc_compare_str(prompt_inst, "new", 0))   return (UserPromptInst){USRPROMPT_ADDCYM, 4};
    if(mc_compare_str(prompt_inst, "del", 0))   return (UserPromptInst){USRPROMPT_DELCYM, 1};
    if(mc_compare_str(prompt_inst, "set", 0))   return (UserPromptInst){USRPROMPT_SETCYM, 2};
    if(mc_compare_str(prompt_inst, "print", 0)) return (UserPromptInst){USRPROMPT_SHOWCYM, 1};

    return (UserPromptInst){USRPROMPT_NONE, 0};
}

CymAtomType parse_type(const char* str){

    if(mc_compare_str(str, "i8", 0))  return CYM_INT8;
    if(mc_compare_str(str, "i16", 0)) return CYM_INT16;
    if(mc_compare_str(str, "i32", 0)) return CYM_INT32;
    if(mc_compare_str(str, "i64", 0)) return CYM_INT64;

    if(mc_compare_str(str, "u8", 0))  return CYM_UINT8;
    if(mc_compare_str(str, "u16", 0)) return CYM_UINT16;
    if(mc_compare_str(str, "u32", 0)) return CYM_UINT32;
    if(mc_compare_str(str, "u64", 0)) return CYM_UINT64;

    if(mc_compare_str(str, "f32", 0))  return CYM_FLOAT32;
    if(mc_compare_str(str, "f64", 0))  return CYM_FLOAT64;

    return CYM_ATOMTYPENONE;
}

UserPrompt parse(size_t token_count, const Mc_token_t* token_buffer){

    UserPrompt output;

    output.flags = 0;
    output.type = USRPROMPT_NONE;
    output.operand1.as_index = 0;
    output.operand2.as_index = 0;
    output.operand3.as_index = 0;

    int err = 0;

    // parsing flags
    //for(size_t i = 0; i < token_count; i+=1){
    //    const char* token_str = mc_get_token_data(token_buffer, token_buffer[i]);
//
    //    if(token_str[0] == '-'){
    //        if(!token_str[1]){
    //            fprintf(stderr, "[ERROR] Expected Flag Identifier After '-'\n");
    //            err = 1;
    //            continue;
    //        } else if(token_str[2]){ // flags should only have two characters, the standart '-' and the identifier
    //            fprintf(stderr, "[ERROR] Invalid Flag Identifier '%s'\n", token_str);
    //            err = 1;
    //            continue;
    //        }
//
    //        const uint64_t flag = get_flag(token_str[1]);
//
    //        if(!flag) err = 1;
//
    //        output.flags |= flag;
//
    //    }
    //}

    for(size_t i = 0; i < token_count; i+=1){
        const char* token_str = mc_get_token_data(token_buffer, token_buffer[i]);

        if(token_str[0] != '-'){ // token is not a flag
            UserPromptInst inst = get_prompt_inst(token_str);

            if(inst.expected_operands != token_count - i - 1){
                fprintf(
                    stderr, "[ERROR] Instruction %s Expects %u Operands, Got %zu Instead\n\n",
                    token_str, inst.expected_operands, token_count - i - 1
                );
                output.type = USRPROMPT_ERROR;
                return output;
            }

            output.type = inst.type;
            output.operand1.as_str = (inst.expected_operands > 0)? mc_get_token_data(token_buffer, token_buffer[i + 1]) : NULL;
            output.operand2.as_str = (inst.expected_operands > 1)? mc_get_token_data(token_buffer, token_buffer[i + 2]) : NULL;
            output.operand3.as_str = (inst.expected_operands > 2)? mc_get_token_data(token_buffer, token_buffer[i + 3]) : NULL;
            output.operand4.as_str = (inst.expected_operands > 3)? mc_get_token_data(token_buffer, token_buffer[i + 4]) : NULL;

            return output;
        }

    }

    if(err) output.type == USRPROMPT_ERROR;

    return output;
}




#endif // =====================  END OF FILE CYMPARSER_HEADER ===========================