#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>


enum UserCmd{
    USER_CMD_QUIT = 0,
    USER_CMD_SHOW_THIS_PROMPT,
    USER_CMD_SEEK,
    USER_CMD_INSPECT,
    USER_CMD_CONSUME,
    USER_CMD_DISPLAY,
    USER_CMD_OPEN,
    USER_CMD_HELP,

    // for counting purposes
    USER_CMD_COUNT,

    USER_CMD_NONE
};

static FILE*  input_file = NULL;
static char* input_file_path = NULL;
static int len = 16 * 10;
static int bytes_per_group = 2;
static int groups_per_line = 8;
static int str_mode;


typedef union Var
{
    int8_t  as_i8;
    int16_t as_i16;
    int32_t as_i32;
    int64_t as_i64;

    uint8_t  as_u8;
    uint16_t as_u16;
    uint32_t as_u32;
    uint64_t as_u64;

    uint8_t  as_x8;
    uint16_t as_x16;
    uint32_t as_x32;
    uint64_t as_x64;

    float    as_f32;
    double   as_f64;

    void*    as_ptr;
} Var;

typedef struct Mc_stream_t{
    void*  data;
    size_t size;
    size_t capacity;
} Mc_stream_t;


int mc_compare_str(const char* str1, const char* str2, int _only_compare_till_first_null){
    unsigned int i = 0;
    for( ; str1[i] && (str1[i] == str2[i]); i+=1);
    return (str1[i] == str2[i]) || (_only_compare_till_first_null && (!str1[i] || !str2[i]));
}

// streams size bytes of data to stream
// \param data the data to stream, pass NULL to allocate the memory but not stream it
// \returns pointer to beggining of streamed data in stream
void* mc_stream(Mc_stream_t* stream, const void* data, size_t size){
    if(size + stream->size > stream->capacity){
        if(stream->capacity == 0) stream->capacity = size + stream->size;
        else stream->capacity *= 1 + (size_t)((size + stream->size) / stream->capacity);
        void* old_data = stream->data;
        stream->data = malloc(stream->capacity);
        memcpy(stream->data, old_data, stream->size);
        free(old_data);
    }
    void* const dest = (void*) (((char*) stream->data) + stream->size);
    if(data) memcpy(dest, data, size);
    stream->size += size;
    return dest;
}

// streams size bytes of data to stream properly aligned
// \param data the data to stream, pass NULL to allocate the memory but not stream it
// \returns pointer to beggining of streamed data in stream
void* mc_stream_aligned(Mc_stream_t* stream, const void* data, size_t size, size_t alignment){

    if(alignment <= 1){
        return mc_stream(stream, data, size);
    }

    const size_t pad = (stream->size - (stream->size % alignment)) % alignment;
    
    stream->size += pad;

    return mc_stream(stream, data, size);
}

// works like mc_stream but streams a null treminated string
void mc_stream_str(Mc_stream_t* stream, const char* data){

    if(!data) return;

    const size_t size = strlen(data);

    mc_stream(stream, data, (size + 1) * sizeof(char));
}

Mc_stream_t mc_create_stream(size_t capacity){
    return (Mc_stream_t){.data = malloc(capacity), .size = 0, .capacity = capacity};
}

void mc_destroy_stream(Mc_stream_t stream){
    free(stream.data);
}

static int get_word_count(const char* str, int* biggest_word){
    if(!str) return -1;

    int words = 0;
    int word_len_max = 0;

    for(; *str == ' ' || *str == '\t' || *str == '\n'; str+=1);

    while(*str){

        int len = 0;

        for(; *str != '\0' && *str != ' ' && *str != '\t' && *str != '\n'; str+=1){
            if(*str == '\\'){
                if(*(++str) == '\0'){
                    return -1;
                }
            }
            len += 1;
        }

        for(; *str == ' ' || *str == '\t' || *str == '\n'; str+=1);

        words += 1;
        if(len > word_len_max) word_len_max = len;
    }

    if(biggest_word) *biggest_word = word_len_max;
    return words;
}

static char* get_next_word(const char* str, char* output, int* len, int max_len){

    if(!str || !output) return NULL;

    for(; *str == ' ' || *str == '\t' || *str == '\n'; str+=1);

    int _len = 0;

    for(; *str != '\0' && *str != ' ' && *str != '\t' && *str != '\n' && _len < max_len; str+=1){
        if(*str == '\\'){
            if(*(++str) == '\0'){
                return NULL;
            }
        }
        output[_len++] = *str;
    }
    output[_len] = '\0';

    if(len) *len = _len;

    return (char*) str;
}

// \returns 0 on success, 1 if stdin was closed, or -1 on failure
static int get_user_prompt(Mc_stream_t* stream, int* _argc, char*** _argv){

    const size_t begin = stream->size;
    for(int c = fgetc(stdin); c && c != '\n'; ){
        char* inputstr = (char*) (stream->data);
        for(; stream->size < stream->capacity && c && c != '\n' && c != EOF; c = fgetc(stdin)){
            inputstr[stream->size++] = (char) c;
        }
        if(c == EOF){
            printf("\n");
            return 1;
        }
        // resize stream
        mc_stream(stream, NULL, 0);
    }
    mc_stream_str(stream, "");

    int biggest_word;
    const int word_count = get_word_count(((char*) stream->data) + begin, &biggest_word);

    if(word_count < 0) return -1;

    char** argv = (char**) mc_stream_aligned(stream, NULL, word_count * sizeof(argv[0]), sizeof(argv[0]));

    char* promptstr = (char*) (((char*) stream->data) + begin);

    for(int i = 0; i < word_count; i+=1){
        argv[i] = promptstr;
        int promptstr_len;
        promptstr = get_next_word(promptstr, promptstr, &promptstr_len, biggest_word);
        promptstr +=1;
    }
    if(_argc) *_argc = word_count;
    if(_argv) *_argv = argv;

    return 0;
}

static int parse_uint(const char* str, int untill_last_numeric){

    if(str[0] < '0' || str[0] > '9') return -1;

    int output = str[0] - '0';

    for(int i = 1; str[i]; i+=1){
        if(str[i] < '0' || str[i] > '9')
            return untill_last_numeric? output : -1;
        output = (output * 10) + (str[i] - '0');
    }
    return output;
}

static int get_cmd_code(const char* cmd){

    if(mc_compare_str(cmd, "quit", 0) || mc_compare_str(cmd, "exit", 0) || mc_compare_str(cmd, "q", 0) || mc_compare_str(cmd, "e", 0))
        return USER_CMD_QUIT;

    if(mc_compare_str(cmd, "show_this_prompt", 0)) return USER_CMD_SHOW_THIS_PROMPT;

    if(mc_compare_str(cmd, "inspect", 0)) return USER_CMD_INSPECT;
    if(mc_compare_str(cmd, "consume", 0)) return USER_CMD_CONSUME;
    if(mc_compare_str(cmd, "seek"   , 0)) return USER_CMD_SEEK;
    if(mc_compare_str(cmd, "display", 0)) return USER_CMD_DISPLAY;
    if(mc_compare_str(cmd, "open"   , 0)) return USER_CMD_OPEN;
    if(mc_compare_str(cmd, "help"   , 0)) return USER_CMD_HELP;

    return USER_CMD_NONE;
}

int display(FILE* input, FILE* output, long offset, int _len, int _bytes_per_group, int _groups_per_line, int _str_mode){

    if(!output) return 0;

    if(output == stdout) printf("\x1B[2J\x1B[H\n");

    if(!input){
        fprintf(output, "no file...\n");
        return 0;
    }

    const long pos = ftell(input);

    fseek(input, offset, SEEK_CUR);

    const int line_count = _len / (_bytes_per_group * _groups_per_line);
    int linedigits = 1;
    for(int l = line_count; l / 10; l /= 10) linedigits += 1;

    fprintf(
        output,
        "file: %s  %lu to %lu\n"
        "lines: %i    groups per line: %i    bytes per group: %i\n\n",
        input_file_path, pos, pos + len,
        line_count + !!(_len % (bytes_per_group * groups_per_line)),
        groups_per_line,
        bytes_per_group
    );

    int c = fgetc(input);

    for(int line = 0; line < line_count && c != EOF; line+=1){
        fprintf(output, "%*i- ", linedigits, line);
        for(int group = 0; group < _groups_per_line && c != EOF; group+=1){
            for(int byte = 0; byte < _bytes_per_group && c != EOF; byte+=1){
                if(_str_mode)
                    fprintf(output, "%c", isprint(c)? (char) c : '.');
                else
                    fprintf(output, "%.2x", c);
                c = fgetc(input);
            }
            if(!_str_mode)
                fputc(' ', output);
        }
        fputc('\n', output);
    }
    const int line_remainder = _len % (bytes_per_group * groups_per_line);
    const int grange = line_remainder / _bytes_per_group;
    const int brange = line_remainder % _bytes_per_group;
    if(line_remainder)
        fprintf(output, "%.*i- ", linedigits, line_count);
    
    
    for(int group = 0; group < grange && c != EOF; group+=1){
        for(int byte = 0; byte < _bytes_per_group && c != EOF; byte+=1){
            if(_str_mode)
                fprintf(output, "%c", isprint(c)? c : '.');
            else
                fprintf(output, "%.2x", c);
            c = fgetc(input);
        }
        if(!_str_mode)
            fputc(' ', output);
    }
    for(int byte = 0; byte < brange && c != EOF; byte += 1){
        if(_str_mode)
            fprintf(output, "%c", isprint(c)? c : '.');
        else
            fprintf(output, "%.2x", c);
        c = fgetc(input);
    }
    fputc('\n', output);

    if(c == EOF){
        fprintf(output, "EOF REACHED\n");
    }

    return fseek(input, pos, SEEK_SET);
}

static int help(int what){

    switch (what)
    {
    case USER_CMD_QUIT:
        printf(
            "quit:\n"
            "\tquits the aplication, exit, e and q are equivalent to this\n"
        );
        break;
    case USER_CMD_SHOW_THIS_PROMPT:
        printf(
            "show_this_prompt <optional: args>...:\n"
            "\tshows the complete provided prompt, for debugging purposes\n"
        );
        break;
    case USER_CMD_SEEK:
        printf(
            "seek <position> --<optional: kwargs>...:\n"
            "\tseeks file position\n"
            "\tyou can use <kwarg>:\n"
            "\t\tend: to seek relative to the file's end\n"
            "\t\tcurrent: to seek relative to current file position\n"
            "\t\tset: to set the file position\n"
        );
        break;
    case USER_CMD_INSPECT:
        printf(
            "inspect <type>...:\n"
            "\tread sizeof(<type>) bytes as <type> for inspection\n"
            "\tsupported type are:\n"
            "\t\ti8:  8  bits integer\n"
            "\t\ti16: 16 bits integer\n"
            "\t\ti32: 32 bits integer\n"
            "\t\ti64: 64 bits integer\n"
            "\t\tu8:  8  bits unsigned integer\n"
            "\t\tu16: 16 bits unsigned integer\n"
            "\t\tu32: 32 bits unsigned integer\n"
            "\t\tu64: 64 bits unsigned integer\n"
            "\t\tx8:  8  bits hexadecimal\n"
            "\t\tx16: 16 bits hexadecimal\n"
            "\t\tx32: 32 bits hexadecimal\n"
            "\t\tx64: 64 bits hexadecimal\n"
            "\t\tf32: 32 bits float\n"
            "\t\tf64: 64 bits float\n"
        );
        break;
    case USER_CMD_CONSUME:
        printf(
            "consume <type>...:\n"
            "\tread sizeof(<type>) bytes as <type> for inspection and then skips it (iteratively)\n"
            "\tsupported type are:\n"
            "\t\ti8:  8  bits integer\n"
            "\t\ti16: 16 bits integer\n"
            "\t\ti32: 32 bits integer\n"
            "\t\ti64: 64 bits integer\n"
            "\t\tu8:  8  bits unsigned integer\n"
            "\t\tu16: 16 bits unsigned integer\n"
            "\t\tu32: 32 bits unsigned integer\n"
            "\t\tu64: 64 bits unsigned integer\n"
            "\t\tx8:  8  bits hexadecimal\n"
            "\t\tx16: 16 bits hexadecimal\n"
            "\t\tx32: 32 bits hexadecimal\n"
            "\t\tx64: 64 bits hexadecimal\n"
            "\t\tf32: 32 bits float\n"
            "\t\tf64: 64 bits float\n"
        );
        break;
    case USER_CMD_DISPLAY:
        printf(
            "display <optional: offset position> -<optional: flags>... --<optional: kwarg>:\n"
            "\tdisplays the file's contents starting from offset position, or just update the display if no offset position is provided\n"
            "\tyou can use <flag>:\n"
            "\t\ts <offset>: to set the offset start position\n"
            "\t\tl <lenght>: to set the byte lenght to display\n"
            "\t\tg <groups per line>: to set the groups per line count\n"
            "\t\tb <bytes per group>: to set the bytes per group count\n"
            "\tyou can use <kwarg>:\n"
            "\t\tmake_default: to make this the default display configuration\n"
            "\t\tstr: to display valid characters\n"
            "\t\thex: to display in standard hexadecimal mode\n"
            "\t\tend: to take the offset position relative to the file's end\n"
            "\t\tcurrent: to take the offset position relative to the file's current position (default)\n"
            "\t\tset: to take the offset position relative to the file's begin\n"
        );
        break;
    case USER_CMD_OPEN:
        printf(
            "open <optional: file path>:\n"
            "\topens file at file path, or reopens the current one if no file path is provided\n"
            "\tthis will close previous file\n"
        );
        break;
    case USER_CMD_HELP:
        printf(
            "help <optional: command>...:\n"
            "\tprints a help message about passed commands, or all commands if none are provided\n"
        );
        break;
    default:
        printf("no help message for %i\n", what);
        return 1;
    }


    return 0;
}

static inline int missing_input_file_error(FILE* output, const int err){
    fprintf(output, "missing input file, provide one by entering 'open <file path>' first\n");
    return err;
}

// \returns -1 to quit aplication, 0 on success or 1 on failure
static int perform_prompt(FILE* f, FILE* outputf, int cmd, int prompt_argc, char** prompt_argv){

    if(prompt_argc < 1){
        fprintf(outputf, "[ERROR] no command in prompt, empty prompt\n");
        return 1;
    }
    switch(cmd){
    case USER_CMD_QUIT:
        return -1;
    case USER_CMD_SHOW_THIS_PROMPT:
        printf("prompt with %i arguments and command %i:\n", prompt_argc, cmd);
        for(int i = 0; i < prompt_argc; i+=1){
            printf("\targument %i- '%s'\n", i, prompt_argv[i]);
        }
        return 0;
    case USER_CMD_SEEK: {
        if(!input_file) return missing_input_file_error(outputf, 1);
        int seek_mode = SEEK_CUR;
        int op = 0;

        for(int i = 1; i < prompt_argc; i+=1){
            if(mc_compare_str(prompt_argv[i], "--end", 0)){
                seek_mode = SEEK_END;
                continue;
            }
            if(mc_compare_str(prompt_argv[i], "--current", 0)){
                seek_mode = SEEK_CUR;
                continue;
            }
            if(mc_compare_str(prompt_argv[i], "--set", 0)){
                seek_mode = SEEK_SET;
                continue;
            }
            op = parse_uint(prompt_argv[i] + (prompt_argv[i][0] == '-'), 0);
            if(op < 0){
                fprintf(outputf, "[ERROR] %s command expected %ith argument to be valid int, got '%s' instead\n", prompt_argv[0], i, prompt_argv[i]);
                op = -1;
                return 1;
            }
            if(prompt_argv[i][0] == '-')
                op *= -1;
        }
        fseek(f, op, seek_mode);
        display(f, outputf, 0, len, bytes_per_group, groups_per_line, str_mode);
        return 0;
    }
    case USER_CMD_INSPECT:
    case USER_CMD_CONSUME: {
        if(!input_file) return missing_input_file_error(outputf, 1);
        if(prompt_argc < 2){
            fprintf(outputf, "[ERROR] command %s expects at least 1 argument, got %i instead\n", prompt_argv[0], prompt_argc - 1);
            return 1;
        }
        long pos = 0;
        if(prompt_argv[0][0] == 'i');
            pos = ftell(f);

        for(int i = 1; i < prompt_argc; i+=1){
            #define INSPECT(OUT, TYPE)\
                if(mc_compare_str(prompt_argv[i], #TYPE, 0)){\
                    if(fread(&OUT.as_ ## TYPE, 1, sizeof(OUT.as_ ## TYPE), f) != sizeof(op.as_ ## TYPE)){\
                        fprintf(outputf, "[ERROR] could not read " #TYPE "\n");\
                        return 1;\
                    }\
                    fprintf(outputf, "as_" #TYPE ": %"PRI ## TYPE"\n", op.as_ ## TYPE);\
                    continue;\
                }
            Var op;
            INSPECT(op, i8);
            INSPECT(op, i16);
            INSPECT(op, i32);
            INSPECT(op, i64);
            INSPECT(op, u8);
            INSPECT(op, u16);
            INSPECT(op, u32);
            INSPECT(op, u64);
            INSPECT(op, x8);
            INSPECT(op, x16);
            INSPECT(op, x32);
            INSPECT(op, x64);
            if(mc_compare_str(prompt_argv[i], "f32", 0)){
                if(fread(&op.as_f32, 1, sizeof(op.as_f32), f) != sizeof(op.as_f32)){
                    fprintf(outputf, "[ERROR] could not read f32\n");
                    return 1;
                }
                fprintf(outputf, "as_f32: %f\n", op.as_f32);
                continue;
            }
            if(mc_compare_str(prompt_argv[i], "f64", 0)){
                if(fread(&op.as_f64, 1, sizeof(op.as_f64), f) != sizeof(op.as_f64)){
                    fprintf(outputf, "[ERROR] could not read f64\n");
                    return 1;
                }
                fprintf(outputf, "as_f64: %f\n", op.as_f64);
                continue;
            }
            fprintf(outputf, "[ERROR] command %s expected valid type at argument %i, got '%s' instead\n", prompt_argv[0], i, prompt_argv[i]);
        }
        if(prompt_argv[0][0] == 'i')
            fseek(f, pos, SEEK_SET);

        return 0;
    }
    case USER_CMD_DISPLAY: {
        int whence = SEEK_CUR;
        int _offset = 0;
        int _len = len;
        int _bytes_per_group = bytes_per_group;
        int _groups_per_line = groups_per_line;
        int _str_mode = str_mode;

        #define DISPLAY_GET_FARG(output, flag, what)\
            if(mc_compare_str(prompt_argv[i], flag, 0)){\
                if(++i >= prompt_argc){\
                    fprintf(outputf, "[ERROR] command %s expects " what " after %s\n", prompt_argv[0], prompt_argv[i]);\
                    return 1;\
                }\
                output = parse_uint(prompt_argv[i], 0);\
                if(output <= 0){\
                    fprintf(outputf, "[ERROR] command %s expects valid " what " after %s, got '%s' instead\n",\
                    prompt_argv[0], prompt_argv[i - 1], prompt_argv[i]);\
                    return 1;\
                }\
                continue;\
            }

        for(int i = 1; i < prompt_argc; i+=1){
            if(mc_compare_str(prompt_argv[i], "-s", 0)){
                if(++i >= prompt_argc){
                    fprintf(outputf, "[ERROR] command %s expects offset position after %s\n", prompt_argv[0], prompt_argv[i]);
                    return 1;
                }
                const int is_neg = (prompt_argv[i][0] == '-')? 1 : 0;
                _offset = parse_uint(prompt_argv[i] + is_neg, 0);
                if(_offset < 0){
                    fprintf(outputf, "[ERROR] command %s expects valid offset position after %s, got '%s' instead\n",
                    prompt_argv[0], prompt_argv[i - 1], prompt_argv[i]);
                    return 1;
                }
                else if(is_neg) _offset *= -1;
                continue;
            }
            DISPLAY_GET_FARG(_len, "-l", "lenght");
            DISPLAY_GET_FARG(_bytes_per_group, "-b", "bytes per group count");
            DISPLAY_GET_FARG(_groups_per_line, "-g", "groups_per_line count");
            if(mc_compare_str(prompt_argv[i], "--set", 0)){
                whence = SEEK_SET;
            }
            if(mc_compare_str(prompt_argv[i], "--current", 0)){
                whence = SEEK_CUR;
                continue;
            }
            if(mc_compare_str(prompt_argv[i], "--end", 0)){
                whence = SEEK_END;
                continue;
            }
            if(mc_compare_str(prompt_argv[i], "--str", 0)){
                _str_mode = 1;
                continue;
            }
            if(mc_compare_str(prompt_argv[i], "--hex", 0)){
                _str_mode = 0;
                continue;
            }
            if(mc_compare_str(prompt_argv[i], "--make_default", 0)){
                len             = _len;
                bytes_per_group = _bytes_per_group;
                groups_per_line = _groups_per_line;
                str_mode        = _str_mode;
                continue;
            }
            const int is_neg = (prompt_argv[i][0] == '-')? 1 : 0;
            _offset = parse_uint(prompt_argv[i] + is_neg, 0);
            if(_offset < 0){
                fprintf(outputf, "command %s expects valid offset in argument %i, got '%s' instead\n",
                prompt_argv[0], i, prompt_argv[i]);
                return 1;
            }
            else if(is_neg) _offset *= -1;

        }
        const long fp = ftell(f);
        fseek(f, _offset, whence);
        display(f, outputf, 0, _len, _bytes_per_group, _groups_per_line, _str_mode);
        fseek(f, fp, SEEK_SET);
    }
        return 0;
    case USER_CMD_OPEN:
        if(prompt_argc > 2){
            fprintf(outputf, "[ERROR] command %s expects optional 1 argument, file path, got %i instead\n", prompt_argv[0], prompt_argc - 1);
            return 1;
        }
        if(prompt_argc < 2){
            if(!input_file_path){
                return missing_input_file_error(outputf, 1);
            }
            const long pos = ftell(input_file);
            FILE* const infile = fopen(input_file_path, "rb");
            if(!infile){
                fprintf(outputf, "[ERROR] could not reopen '%s'\n", input_file_path);
                return 1;
            }
            if(input_file) fclose(input_file);
            input_file = infile;
            f = input_file;
            if(fseek(input_file, pos, SEEK_SET)){
                perror("fseek");
                return 1;
            }
        }
        else{
            FILE* const infile = fopen(prompt_argv[1], "rb");
            if(!infile){
                fprintf(outputf, "[ERROR] could not open '%s'\n", prompt_argv[1]);
                return 1;
            }
            if(input_file) fclose(input_file);
            if(input_file_path) free(input_file_path);
            input_file = infile;
            size_t input_file_path_len = 0;
            for(; prompt_argv[1][input_file_path_len]; input_file_path_len+=1);
            input_file_path = malloc(input_file_path_len + 1);
            for(size_t i = 0; i < input_file_path_len; i+=1)
                input_file_path[i] = prompt_argv[1][i];
            input_file_path[input_file_path_len] = '\0';
        }
        display(f, outputf, 0, len, bytes_per_group, groups_per_line, str_mode);
        return 0;
    case USER_CMD_HELP:
        if(prompt_argc <= 1){
            for(int i = 0; i < USER_CMD_COUNT; i+=1){
                if(help(i)){
                    fprintf(stderr, "[ERROR] help is missing %i cmd\n", i);
                    return 1;
                }
            }
        }
        for(int i = 1; i < prompt_argc; i+=1){
            const int what = get_cmd_code(prompt_argv[i]);
            if(what == USER_CMD_NONE){
                printf("no command %s\n", prompt_argv[i]);
                continue;
            }
            if(help(what)){
                fprintf(stderr, "[ERROR] help is missing %i cmd, aka '%s'\n", i, prompt_argv[i]);
                return 1;
            }
        }
        return 0;
    default:
        fprintf(stderr, "[ERROR] unknown command '%s'\n", prompt_argv[0]);
        return 1;
    }
}

int main(int argc, char** argv){

    int start_position = 0;

    for(int i = 1; i < argc; i+=1){
        if(mc_compare_str(argv[i], "-s", 0)){
            if(++i >= argc){
                fprintf(stderr, "[ERROR] expected start position after %s\n", argv[i - 1]);
                return 1;
            }
            const int is_negative = !!(argv[i][0] == '-');
            start_position = parse_uint(argv[i] + is_negative, 0);
            if(start_position < 0){
                fprintf(stderr, "[ERROR] expected valid position after %s, got '%s' instead\n", argv[i - 1], argv[i]);
                return 1;
            }
            if(is_negative) start_position *= -1;
            continue;
        }
        if(mc_compare_str(argv[i], "-l", 0)){
            if(++i >= argc){
                fprintf(stderr, "[ERROR] expected byte lenght after %s\n", argv[i - 1]);
                return 1;
            }
            len = parse_uint(argv[i], 0);
            if(len <= 0){
                fprintf(stderr, "[ERROR] expected valid byte lenght after %s, got '%s' instead\n", argv[i - 1], argv[i]);
                return 1;
            }
            continue;
        }
        if(mc_compare_str(argv[i], "-g", 0)){
            if(++i >= argc){
                fprintf(stderr, "[ERROR] expected groups_per_line after %s\n", argv[i - 1]);
                return 1;
            }
            groups_per_line = parse_uint(argv[i], 0);
            if(groups_per_line <= 0){
                fprintf(stderr, "[ERROR] expected valid groups_per_line after %s, got '%s' instead\n", argv[i - 1], argv[i]);
                return 1;
            }
            continue;
        }
        if(mc_compare_str(argv[i], "-b", 0)){
            if(++i >= argc){
                fprintf(stderr, "[ERROR] expected bytes_per_group after %s\n", argv[i - 1]);
                return 1;
            }
            bytes_per_group = parse_uint(argv[i], 0);
            if(bytes_per_group <= 0){
                fprintf(stderr, "[ERROR] expected valid bytes_per_group after %s, got '%s' instead\n", argv[i - 1], argv[i]);
                return 1;
            }
            continue;
        }
        if(input_file){
            fprintf(stderr, "[ERROR] multiple input files provided\n");
            return 1;
        }
        input_file = fopen(argv[1], "rb");
        if(!input_file){
            fprintf(stderr, "[ERROR] could not open '%s'\n", input_file_path);
            return 1;
        }
        size_t input_file_path_len = 0;
        for(; argv[1][input_file_path_len]; input_file_path_len+=1);
        input_file_path = malloc(input_file_path_len + 1);
        for(size_t i = 0; i < input_file_path_len; i+=1)
            input_file_path[i] = argv[1][i];
        input_file_path[input_file_path_len] = '\0';
    }

    if(start_position && input_file){
        if(start_position >= 0)
            fseek(input_file, start_position, SEEK_SET);
        else
            fseek(input_file, start_position, SEEK_END);
    }

    Mc_stream_t stream = mc_create_stream(1024);

    display(input_file, stdout, 0, len, bytes_per_group, groups_per_line, str_mode);

    for(size_t scope = stream.size; feof(stdin) == 0; stream.size = scope){

        int prompt_argc;
        char** prompt_argv;

        printf("\n>>> ");
        fflush(stdout);
        int i = get_user_prompt(&stream, &prompt_argc, &prompt_argv);
        // stdin closed
        if(i == -1)
            break;
        // error
        if(i){
            fprintf(stderr, "[ERROR] could not get user prompt, try agin\n");
            continue;
        }

        if(prompt_argc < 1){
            display(input_file, stdout, 0, len, bytes_per_group, groups_per_line, str_mode);
            continue;
        }

        const int cmd = get_cmd_code(prompt_argv[0]);

        if(cmd == USER_CMD_NONE){
            printf("no command for '%s'\n", prompt_argv[0]);
            printf("enter 'help' to check possible commands\n");
            continue;
        }
        i = perform_prompt(input_file, stdout, cmd, prompt_argc, prompt_argv);
        // quit aplication signal
        if(i == -1) break;
        // error
        if(i){
            printf("command failed ^ ^ ^\n");
        }
        
    }


    if(input_file) fclose(input_file);
    if(input_file_path) free(input_file_path);
    mc_destroy_stream(stream);

    return 0;
}







