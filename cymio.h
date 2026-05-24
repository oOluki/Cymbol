/*
    MIT License

    Copyright (c) 2024 oOluki

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

/*
    very simple implementation of reading key directly from terminal and drawing color text

    to access the implementation use #define CYM_IO_IMPLEMENTATION
*/

#ifndef _CYM_IO_H
#define _CYM_IO_H


#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>



#ifdef __cplusplus
extern "C" {
#endif


int cym_read_key();

const char* cym_get_foreground_color_string(char* buff, uint32_t foregroung_color);

const char* cym_get_background_color_string(char* buff, uint32_t background_color);

const char* cym_get_color_string(char* buff, uint32_t foregroung_color, uint32_t background_color);

void cym_put_color_char(char c, uint32_t foregroung_color, uint32_t background_color);

void cym_print_color_cstr(const char* cstr, uint32_t foregroung_color, uint32_t background_color);



#ifdef CYM_IO_IMPLEMENTATION

#if defined(__linux__) || defined(__APPLE__)

#include <unistd.h>
#include <termios.h>

    int cym_read_key() {
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        const int c = fgetc(stdin);

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return c;
    }

#elif defined(_WIN32)

    #include <conio.h>

    int cym_read_key() { return _getch(); }

#else

    int cym_read_key() { return fgetc(stdin); }

#endif // read_key definition


const char* cym_get_foreground_color_string(char* buff, uint32_t foregroung_color){

    const uint8_t fr = (foregroung_color >>  0) & 0xFF;
    const uint8_t fg = (foregroung_color >>  8) & 0xFF;
    const uint8_t fb = (foregroung_color >> 16) & 0xFF;
    const uint8_t fa = (foregroung_color >> 24) & 0xFF;

    sprintf(
        buff,
        "\x1b[38;2;%" PRIu8 ";%" PRIu8 ";%" PRIu8 "m",
        fr, fg, fb
    );

    return buff;
}

const char* cym_get_background_color_string(char* buff, uint32_t background_color){

    const uint8_t br = (background_color >>  0) & 0xFF;
    const uint8_t bg = (background_color >>  8) & 0xFF;
    const uint8_t bb = (background_color >> 16) & 0xFF;
    const uint8_t ba = (background_color >> 24) & 0xFF;

    sprintf(
        buff,
        "\x1b[48;2;%" PRIu8 ";%" PRIu8 ";%" PRIu8 "m",
        br, bg, bb
    );

    return buff;
}

const char* cym_get_color_string(char* buff, uint32_t foregroung_color, uint32_t background_color){

    const uint8_t fr = (foregroung_color >>  0) & 0xFF;
    const uint8_t fg = (foregroung_color >>  8) & 0xFF;
    const uint8_t fb = (foregroung_color >> 16) & 0xFF;
    const uint8_t fa = (foregroung_color >> 24) & 0xFF;


    const uint8_t br = (background_color >>  0) & 0xFF;
    const uint8_t bg = (background_color >>  8) & 0xFF;
    const uint8_t bb = (background_color >> 16) & 0xFF;
    const uint8_t ba = (background_color >> 24) & 0xFF;

    sprintf(
        buff,
        "\x1b[38;2;%" PRIu8 ";%" PRIu8 ";%" PRIu8 "m"
        "\x1b[48;2;%" PRIu8 ";%" PRIu8 ";%" PRIu8 "m",
        fr, fg, fb,
        br, bg, bb
    );

    return buff;
}

static inline const char* __cym__get_color_string(uint32_t foregroung_color, uint32_t background_color){
    static char buff[64];
    return cym_get_color_string(buff, foregroung_color, background_color);
}

void cym_put_color_char(char c, uint32_t foregroung_color, uint32_t background_color){
    printf("%s%c\x1b[0m", __cym__get_color_string(foregroung_color, background_color), c);
}

void cym_print_color_cstr(const char* cstr, uint32_t foregroung_color, uint32_t background_color){
    printf("%s%s\x1b[0m", __cym__get_color_string(foregroung_color, background_color), cstr);
}


#endif // END OF #ifdef CYM_IO_IMPLEMENTATION


#ifdef __cplusplus
}
#endif

#endif // =====================  END OF FILE _CYM_IO_H ===========================