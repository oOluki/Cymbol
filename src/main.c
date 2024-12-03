#include "cymbol.h"
#include "cymath.h"
#include "parser.h"


CymContext* contexts = NULL;

struct Config{
    int context_count;
    int current_context;
    int capacity;
} config = {1, 0, 1};

int digit(char c){
    switch (c){
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    default : return -1;
    }
}

int get_literal(const char* str){

    int i = 0;
    int _10expn = 1;
    size_t size = 0;

    for(; str[size]; size+=1);

    for(int n = size - 1; n > -1; n-=1){

        const int d = digit(str[n]);

        if(d < 0){
            fprintf(
                stderr,
                "[ERROR] Expected Int Literal, Got '%s' Instead. "
                "Character '%c' Is Not A Digit\n",
                str, str[n]
            );
            return -1;
        }

        i += d * _10expn;
        _10expn *= 10;

    }

    return i;    
}

// returns -1 to request exit, an error code or 0 on success
int perform_command(UserPrompt prompt){

    switch (prompt.type)
    {
    case USRPROMPT_EXIT: return -1;
    case USRPROMPT_ERROR: return 1;
    case USRPROMPT_CLEARMEM:
        for(int i = 0; i < config.context_count; i+=1){
            cym_destroy_context(contexts + i);
        }
        config.context_count = 1;
        config.current_context = 0;
        config.capacity = 1;
        free(contexts);
        contexts = (CymContext*)malloc(sizeof(CymContext));
        break;
    case USRPROMPT_ADDCONTEXT:
        if(config.context_count + 1 > config.capacity){
            CymContext* ncontexts = (CymContext*)malloc((config.context_count + 1) * sizeof(CymContext));
            memcpy(ncontexts, contexts, config.context_count * sizeof(CymContext));
            free(contexts);
            contexts = ncontexts;
            config.capacity += 1;
        }
        contexts[config.context_count] = cym_create_context(10);
        config.current_context = config.context_count;
        config.context_count += 1;
        break;
    case USRPROMPT_DELCONTEXT:
        if(config.context_count < 2) break;
        cym_destroy_context(contexts + config.current_context);
        for(size_t i = config.current_context; i < config.context_count - 1; i+=1){
            contexts[i] = contexts[i + 1];
        }
        config.context_count -= 1;
        if(config.current_context && config.current_context == config.context_count){
            config.current_context -= 1;
        }
        break;
    case USRPROMPT_CHANGECONTEXT:
        prompt.operand1.as_index = get_literal(prompt.operand1.as_str);
        if(prompt.operand1.as_index < 1 || prompt.operand1.as_index > config.context_count){
            fprintf(stderr, "[ERROR] Can't Change To Non Existing Context '%zu'\n", prompt.operand1.as_index);
            return 2;
        }
        config.current_context = prompt.operand1.as_index - 1;
        break;
    case USRPROMPT_LOADCONTEXT:
        if(config.context_count + 1 > config.capacity){
            CymContext* ncontexts = (CymContext*)malloc((config.context_count + 1) * sizeof(CymContext));
            memcpy(ncontexts, contexts, config.context_count * sizeof(CymContext));
            free(contexts);
            contexts = ncontexts;
            config.capacity += 1;
        }
        contexts[config.context_count] = cym_create_context(10);
        config.current_context = config.context_count;
        config.context_count += 1;
        cym_load_context(contexts + config.current_context, prompt.operand1.as_str);
        break;
    case USRPROMPT_SAVECONTEXT:
        cym_save_context(contexts + config.current_context, prompt.operand1.as_str);
        break;
    case USRPROMPT_MERGECONTEXTS:
        prompt.operand1.as_index = get_literal(prompt.operand1.as_str);
        if(prompt.operand1.as_index == config.current_context) return 0;
        if(prompt.operand1.as_index < 1 || prompt.operand1.as_index > config.current_context){
            fprintf(stderr, "[ERROR] Can't Merge With Non Existing Context '%zu'\n", prompt.operand1.as_index);
            return 3;
        }
        cym_merge_contexts(contexts + config.current_context, contexts + prompt.operand1.as_index);
        cym_destroy_context(contexts + prompt.operand1.as_index);
        for(size_t i = prompt.operand1.as_index; i < config.context_count - 1; i+=1){
            contexts[i] = contexts[i + 1];
        }
        if(config.context_count > 1) config.context_count -= 1;
        break;
    case USRPROMPT_ADDCYM:
        prompt.operand1.as_index = parse_type(prompt.operand1.as_str);
        prompt.operand3.as_index = get_literal(prompt.operand3.as_str);
        prompt.operand4.as_index = get_literal(prompt.operand4.as_str);
        cym_push_data(
            contexts + config.current_context, prompt.operand2.as_str,
            (CymAtomType)prompt.operand1.as_index, prompt.operand3.as_index, prompt.operand4.as_index, NULL
        );
        break;
    case USRPROMPT_DELCYM:
        cym_delete_cymbol(
            contexts + config.current_context,
            cym_get_cymbol(
                contexts + config.current_context,
                prompt.operand1.as_str
            )
        );
        break;
    case USRPROMPT_SHOWCYM:
        cym_display_cymbol(
            contexts + config.current_context,
            cym_get_cymbol(
                contexts + config.current_context,
                prompt.operand1.as_str
            )
        );
        break;
    
    default:
        CLEAR_VIEW;
        break;;
    }

    return 0;
}


int main(int argc, char** argv){

    Mc_token_t* token_buffer = mc_create_token_buffer(100, 1000);
    
    contexts = (CymContext*)malloc(sizeof(CymContext));

    contexts[0] = cym_create_context(1);

    if(argc < 2){

        
        char buffer[1000];

        do{            
            printf(
                "\tcurrent context: %i of %i (capacity=%i)\n\n(Cymbol) ",
                config.current_context + 1, config.context_count, config.capacity
            );

            memset(buffer, 0, 1000);
            fgets(buffer, 999, stdin);

            size_t token_count = mc_tokenize(&token_buffer, (unsigned int)0, buffer, NULL, "", "");

            const UserPrompt prompt = parse(token_count, token_buffer);

            const int response = perform_command(prompt);

            if(response < 0) break;

        } while(!feof(stdin));

        printf("\n");

        mc_destroy_token_buffer(token_buffer);

        return 0;
    }

    size_t token_count = 0;

    for(int i = 1; i < argc; i+=1){

        token_count += mc_tokenize(
            &token_buffer, token_count,
            argv[i], "", "-", ""
        );

    }

    UserPrompt prompt = parse(token_count, token_buffer);

    mc_destroy_token_buffer(token_buffer);

    return perform_command(prompt) ^ (int)(-1);
}


