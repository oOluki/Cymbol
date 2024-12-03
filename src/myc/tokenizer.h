#ifndef MC_TOKENIZER_HEADER
#define MC_TOKENIZER_HEADER

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "string.h"
#include "memory.h"

// tokens are just numbers, use them to get the corresponding token data through mc_get_token_data
typedef Mc_size_t Mc_token_t;

typedef struct Mc_tkn_metadata_t{

Mc_stream_t streamstring;
Mc_stream_t tokn_strm_bff;

} Mc_tkn_metadata_t;


// returns the number of generated tokens and writes the tokens key to 'token_buffer_pointer'
// the memory at the returned pointer has to be later freed via mc_destroy_tokens (do not use free)
// \param token_buffer_pointer a pointer to the buffer where the token keys will be stored
// \param buff_pos the position where to start buffering the tokens into the token_buffer
// \param string the string of to be tokenized
// \param __ignored the string of the characters to be ignored, pass null to ignore trivials '\ ' '\\n' '\\t'
// \param __special_characters the characters that should be considered tokens on their own
// \param __line_comment the string that represents a line comment
// \returns the number of tokenized tokens
unsigned int mc_tokenize(Mc_token_t** token_buffer_pointer, unsigned int buff_pos, const char* string,
    const char* __ignored, const char* __special_characters, const char* __line_comment){

        Mc_token_t* token_buffer = *token_buffer_pointer;
        
        Mc_tkn_metadata_t* meta = (Mc_tkn_metadata_t*)(token_buffer) - 1;
        Mc_stream_t stream = meta->streamstring;
        Mc_stream_t tokens = meta->tokn_strm_bff;

        const char* strbegin_1 = stream.data; // gotta store this pointer for later correcting the

        //printf(
        //    "meta:\n"
        //    "\tstream: size = %zu, cap = %zu\n"
        //    "\tbuffer: size = %zu, cap = %zu\n",
        //    stream.size, stream.capacity,
        //    tokens.size, tokens.capacity
        //);


        if(buff_pos * sizeof(Mc_token_t) + sizeof(Mc_tkn_metadata_t) < tokens.size){
            stream.size = token_buffer[buff_pos];
        }

        const char* ignored = __ignored? __ignored : " \t\n";               // avoiding null dereferencing
        const char* special_characters = __special_characters? __special_characters : "";
        const char* line_comment = __line_comment? __line_comment : "";

        //printf("configuration:\n");
        //printf("\tignored = '%s'\n", ignored);
        //printf("\tspecial characters = '%s'\n", special_characters);
        //printf("\tline_comment = '%s'\n", line_comment);   

        Mc_size_t last_token = 0;
        Mc_size_t counter = 0;
        int last_was_ignored = 0;
        unsigned int tkn_count = 0;

        for(; string[counter]; counter+=1){
            const char c = string[counter];

            if(c == line_comment[0]){
                int skip = mc_compare_str(string + counter, line_comment, 1);
                
                if(skip){
                    Mc_int_t to_ = mc_find_char(string, '\n', counter);

                    if(to_ < 0) break;
                    counter += to_;
                    last_token = counter + 1;
                    continue;
                }
            }

            Mc_byte_t is_ignored = (mc_find_char(ignored, c, 0) >= 0)? 1 : 0;
            if(is_ignored){
                if(!last_was_ignored && (last_token < counter)){ // add last token only if it wasn't a special or ignored character
                    Mc_token_t token = stream.size;                                     // be carefull, you cant simply point to the string in the stream,
                    mc_stream(&stream, string + last_token, counter - last_token);      // as the stream is dynamically reallocating memory
                    const char null_termc = '\0';                                       // instead the token will hold the index of the string in the stream
                    mc_stream(&stream, &null_termc, sizeof(null_termc));  
                    mc_stream(&tokens, &token, sizeof(token));
                    tkn_count += 1;
                }
                last_token = counter + 1;
                continue;
            } else last_was_ignored = 0;

            Mc_byte_t is_special = (mc_find_char(special_characters, c, 0) >= 0)? 1 : 0;

            if(is_special){
                if(last_token < counter){   // add last token only if it wasn't a special character
                    Mc_token_t token = stream.size;    // be carefull, you cant simply point to the string in the stream,
                    mc_stream(&stream, string + last_token, counter - last_token);      // as the stream is dynamically reallocating memory
                    const char null_termc = '\0';                                       // instead the token will hold the index of the string in the stream
                    mc_stream(&stream, &null_termc, sizeof(null_termc));  
                    mc_stream(&tokens, &token, sizeof(token));
                    tkn_count += 1;
                }
                Mc_token_t token = stream.size;
                const char dummybuff[2] = {c, '\0'};
                mc_stream(&stream, dummybuff, sizeof(dummybuff));
                mc_stream(&tokens, &token, sizeof(token));
                last_token = counter + 1;
                tkn_count += 1;
            }

        }

        if(last_token < counter){   // add last token only if appropriate
            Mc_token_t token = stream.size;    // be carefull, you cant simply point to the string in the stream,
            mc_stream(&stream, string + last_token, counter - last_token);      // as the stream is dynamically reallocating memory
            const char null_termc = '\0';                                       // instead the pointer will hold the index of the string in the stream
            mc_stream(&stream, &null_termc, sizeof(null_termc));  
            mc_stream(&tokens, &token, sizeof(token));
            tkn_count += 1;
        }
        

        *token_buffer_pointer = (Mc_token_t*)((Mc_tkn_metadata_t*)tokens.data + 1);

        meta = (Mc_tkn_metadata_t*)tokens.data;
        *meta = (Mc_tkn_metadata_t){.streamstring = stream, .tokn_strm_bff = tokens};


        return tkn_count;
}



// you can repeatedly use this bufffer on mc_tokenize only by the end
// should this be freed by mc_destroy_token_buffer (don't use free)
// \param tkn_icap the initial capacity (in token count) of the token buffer
// \param tknstrm_icap the initial capacity (in bytes) of the stream that will allocate the tokens strings
// \returns a buffer that can be used to hold the tokenized tokens by mc_tokenize
Mc_token_t* mc_create_token_buffer(Mc_size_t tkn_icap, Mc_size_t tknstrm_icap){

    Mc_tkn_metadata_t* mem_chunk = (Mc_tkn_metadata_t*)malloc(
        tkn_icap * sizeof(Mc_token_t) + sizeof(Mc_tkn_metadata_t)
    );

    mem_chunk->tokn_strm_bff.data = (Mc_byte_t*)(mem_chunk);
    mem_chunk->tokn_strm_bff.size = sizeof(Mc_tkn_metadata_t);
    mem_chunk->tokn_strm_bff.capacity = tkn_icap * sizeof(Mc_token_t) + sizeof(Mc_tkn_metadata_t);

    mem_chunk->streamstring.data = (Mc_byte_t*)malloc(tknstrm_icap);
    mem_chunk->streamstring.size = 0;
    mem_chunk->streamstring.capacity = tknstrm_icap;

    return (Mc_token_t*)(mem_chunk + 1);
}

// frees all the memory related to the tokenization process of tokens
// (their strings and the memory chunk containing them and the tokens)
// \param the pointer to the tokens to destroy
void mc_destroy_token_buffer(Mc_token_t* tokens){
    Mc_tkn_metadata_t* mem_chunk = (Mc_tkn_metadata_t*)tokens - 1;
    free(mem_chunk->streamstring.data);
    free(mem_chunk);
}


// returns the string related to the token in the token_buffer
static inline char* mc_get_token_data(const Mc_token_t* token_buffer, Mc_token_t token){
    return ((const Mc_tkn_metadata_t*)token_buffer - 1)->streamstring.data + token;
}

// buffers the strings related to all the tokens in the token_buffer to the corresponding string in buffer
// this is simply the meta_data.str_begin + token
void mc_buffer_token_data(const char** buffer, const Mc_token_t* token_buffer){
    const Mc_tkn_metadata_t* meta_data = (Mc_tkn_metadata_t*)token_buffer - 1;
    const Mc_size_t token_count = (meta_data->tokn_strm_bff.size - sizeof(Mc_tkn_metadata_t)) / sizeof(Mc_token_t);
    for(Mc_size_t i = 0; i < token_count; i+=1){
        buffer[i] = meta_data->streamstring.data + token_buffer[i];        
    }
}


#endif // =====================  END OF FILE MC_TOKENIZER_HEADER ===========================