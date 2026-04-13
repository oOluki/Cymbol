
#ifdef STB_IMAGE_PATH
    #define STB_IMAGE_IMPLEMENTATION
    #include STB_IMAGE_PATH
#else
    #error missing STB_IMAGE_PATH
#endif // END OF #ifdef 
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int find_index(uint32_t color, stbi_uc* pixels, int w, int h, int comp){

    if(pixels == NULL) return -1;

    for(int i = 0; i < h; i+=1){
        for(int j = 0; j < w; j+=1){
            switch (comp)
            {
            case 1:
                if((uint32_t) ((uint8_t*)(pixels))[i * w + j] == color)
                    return i * w + j;
                break;
            case 2:
                if((uint32_t) ((uint16_t*)(pixels))[i * w + j] == color)
                    return i * w + j;
                break;
            case 4:
                if((uint32_t) ((uint32_t*)(pixels))[i * w + j] == color)
                    return i * w + j;
                break;
            
            default:
                return -1;
            }
        }
    }
    return -1;
}

int get(stbi_uc** src, void* dest, int comp){
    switch (comp)
    {
    case 1:
        *((uint8_t*)(dest)) = *((uint8_t*)(*src));
        *(uint8_t**)(src)  += 1;
        return 0;
    case 2:
        *((uint16_t*)(dest)) = *((uint16_t*)(*src));
        *(uint16_t**)(src)  += 1;
        return 0;
    case 4:
        *((uint32_t*)(dest)) = *((uint32_t*)(*src));
        *(uint32_t**)(src)  += 1;
        return 0;
    
    default:
        return 1;
    }
    return 0;
}

static inline void print_help(const char* executable){
    printf(
        "converts an image to a C style array of uint32 pixels or indices to a palette\n"
        "usage %s <input_image> -<optional: flags>... --<optional: help>\n"
        "flags can be:\n"
        "\to <output>: write output to provided file\n"
        "\ti <input>: take input from file provided\n"
        "\tp <palette>: map pixels to palette\n"
        "\tn <name>: use name for the output array's name\n"
        "\ttw <tile width>: sets a tile width, the output will sepparete tiles with ' '\n"
        "\tth <tile height>: sets tile height, the program will sepparate tiles with new lines\n"
        "\tx <x>: sets the x position offset of pixels to skip\n"
        "\ty <y>: sets the y position offset of pixels to skip\n"
        "\tw <width>: sets the width of the image to be read\n"
        "\th <height>: sets the height of the image to be read\n",
        executable
    );
}

static int parse_uint(const char* str){
    int output = 0;

    for(int i = 0; str[i]; i+=1){
        if(str[i] > '9' || str[i] < '0') return -1;
        output = (output * 10) + (str[i] - '0');
    }
    return output;
}

int cmp_str(const char* str1, const char* str2){
    int i = 0;
    for(; str1[i] && str1[i] == str2[i]; i+=1);
    return str1[i] == str2[i];
}

static inline int flag_expects_uint_error(const char* flag, const char* arg){
    fprintf(stderr, "[ERROR] flag %s expects uint argument, got %s instead\n", flag, arg? arg : "nothing");
    return 1;
}

#define FLAG_GET_UINT(output, ...) do {\
    if(i + 1 >= argc) return flag_expects_uint_error(argv[i], NULL);\
    __VA_ARGS__ output = parse_uint(argv[i + 1]);\
    if(output <= 0) return flag_expects_uint_error(argv[i], argv[i + 1]);\
    i += 1;\
} while(0)

int main(const int argc, const char* const* const argv){

    int input   = -1;
    int output  = -1;
    int palette = -1;
    int ax       = 0;
    int ay       = 0;
    int aw       = -1;
    int ah       = -1;
    int tw      = -1;
    int th      = -1;
    const char* name = NULL;

    int pw;
    int ph;
    int pcomp;
    stbi_uc* ppixels = NULL;
    int pn = 0;

    int sheetw;
    int sheeth;
    int sheetcomp;
    stbi_uc* pixels = NULL;
    int n = 0;

    if(argc < 2){
        print_help(argv[0]);
        fprintf(stderr, "[ERROR] Expected Input Png\n");
        return 1;
    }
    for(int i = 1; i < argc; i+=1){
        if(cmp_str(argv[i], "--help")){
            print_help(argv[0]);
            return 0;
        }
        if(cmp_str(argv[i], "-o")){
            if(++i >= argc){
                fprintf(stderr, "[ERROR] expected output file path after %s\n", argv[i-1]);
                return 1;
            }
            if(output > 0){
                fprintf(stderr, "[ERROR] multiple output paths, at arguments %i and %i\n", output, i);
                return 1;
            }
            output = i;
            continue;
        }
        if(cmp_str(argv[i], "-i")){
            if(++i >= argc){
                fprintf(stderr, "[ERROR] expected input file path after %s\n", argv[i-1]);
                return 1;
            }
            if(input > 0){
                fprintf(stderr, "[ERROR] multiple input paths, at arguments %i and %i\n", input, i);
                return 1;
            }
            input = i;
            continue;
        }
        if(cmp_str(argv[i], "-p")){
            if(++i >= argc){
                fprintf(stderr, "[ERROR] expected palette file path after %s\n", argv[i-1]);
                return 1;
            }
            if(palette > 0){
                fprintf(stderr, "[ERROR] multiple palette paths, at arguments %i and %i\n", palette, i);
                return 1;
            }
            palette = i;
            continue;
        }
        if(cmp_str(argv[i], "-n")){
            if(++i >= argc){
                fprintf(stderr, "[ERROR] expected name after %s\n", argv[i-1]);
                return 1;
            }
            name = argv[i];
            continue;
        }
        if(cmp_str(argv[i], "-x")){
            FLAG_GET_UINT(ax);
            continue;
        }
        if(cmp_str(argv[i], "-y")){
            FLAG_GET_UINT(ay);
            continue;
        }
        if(cmp_str(argv[i], "-w")){
            FLAG_GET_UINT(aw);
            continue;
        }
        if(cmp_str(argv[i], "-h")){
            FLAG_GET_UINT(ah);
            continue;
        }
        if(cmp_str(argv[i], "-tw")){
            FLAG_GET_UINT(tw);
            continue;
        }
        if(cmp_str(argv[i], "-th")){
            FLAG_GET_UINT(th);
            continue;
        }
        if(input > 0){
            fprintf(stderr, "[ERROR] multiple input paths, at arguments %i and %i\n", input, i);
            return 1;
        }
        input = i;
    }

    if(input < 0){
        fprintf(stderr, "[ERROR] missing input file\n");
        return 1;
    }

    #define DEFER_ERROR(...) do { fprintf(stderr, "[ERROR] " __VA_ARGS__); err = 1; goto defer; } while(0)

    int err = 0;

    FILE* const fo = (output < 0)? stdout : fopen(argv[output], "w");

    if(!fo){
        fprintf(stderr, "[ERROR] Could not open %s\n", argv[output]);
        return 1;
    }

    pixels = stbi_load(argv[input], &sheetw, &sheeth, &sheetcomp, 0);

    const int w = (aw <= 0 || ax + aw > sheetw)? sheetw : ax + aw;
    const int h = (ah <= 0 || ay + ah > sheeth)? sheeth : ay + ah;
    

    if(!pixels){
        DEFER_ERROR("Unable To Load '%s': %s\n", argv[input], stbi_failure_reason());
    }
    if(sheetcomp != 1 && sheetcomp != 2 && sheetcomp != 4){
        DEFER_ERROR("Invalid Compression %i For '%s'\n", sheetcomp, argv[input]);
    }

    if(palette > 0){
        ppixels = stbi_load(argv[palette], &pw, &ph, &pcomp, 0);
        if(!ppixels){
            DEFER_ERROR("Unable To Load '%s': %s\n", argv[palette], stbi_failure_reason());
            
        }
        if(pcomp != 1 && pcomp != 2 && pcomp != 4){
            stbi_image_free(pixels);
            stbi_image_free(ppixels);
            DEFER_ERROR("Invalid Compression %i For '%s'\n", sheetcomp, argv[palette]);
            
        }
        int dummy = pw * ph;
        while (dummy)
        {
            dummy /= 10;
            pn+=1;
        }        
    }


    fprintf(fo, "static const int %sw = %i;\n", name? name : "", sheetw);
    fprintf(fo, "static const int %sh = %i;\n", name? name : "", sheeth);

    if(!ppixels){
        switch (sheetcomp)
        {
        case 1:
            fprintf(fo, "static const uint8_t %s[] = {", name? name : "array");
            break;
        case 2:
            fprintf(fo, "static const uint16_t %s[] = {", name? name : "array");
            break;
        case 4:
            fprintf(fo, "static const uint32_t %s[] = {", name? name : "array");
            break;
        default:
            break;
        }
        stbi_uc* p = pixels;
        uint32_t o = 0;
        if(th <= 0) fprintf(fo, "\n");
        for(int i = ay; i < h; i+=1){
            if(th > 0 && !((i - ay) % th)) fprintf(fo, "\n");
            for(int j = ax; j < w; j+=1){
                if(tw > 0 && !((j - ax) % tw)) fprintf(fo, "   ");
                get(&p, &o, sheetcomp);
                fprintf(fo, " 0x%.*x,", sheetcomp * 2, o);
            }
            fprintf(fo, "\n");
        }
    }
    else {
        fprintf(fo, "static const int %s[] = {", name? name : "array");
        stbi_uc* p = pixels;
        uint32_t color = 0;
        if(th <= 0) fprintf(fo, "\n");
        for(int i = ay; i < h; i+=1){
            if(th > 0 && !((i - ay) % th)) fprintf(fo, "\n");
            for(int j = ax; j < w; j+=1){
                if(tw > 0&& !((j - ax) % tw)) fprintf(fo, "   ");
                get(&p, &color, sheetcomp);
                const int o = find_index(color, ppixels, pw, ph, pcomp);
                if(o < 0){
                    DEFER_ERROR("Could Not Find %" PRIu32 " From '%s' In '%s'\n", color, argv[1], argv[2]);
                }
                fprintf(fo, " %.*u,", pn, o);
            }
            fprintf(fo, "\n");
        }
    }

    fprintf(fo, "};\n");

    defer:
    if(pixels) stbi_image_free(pixels);
    if(ppixels) stbi_image_free(ppixels);
    if(fo && fo != stdout) fclose(fo);

    return err;
}
