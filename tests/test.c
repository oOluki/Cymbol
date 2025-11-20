#define CYMBOL_IMPLEMENTATION
#define CYMATH_IMPLEMENTATION
#include "../cymbol.h"
#include "../cymath.h"


int main(){

    int buff1[10];

    int buff2[10] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    };

    cym_pack_data(buff1,
        sizeof(int), &buff2[0],
        sizeof(int), &buff2[1],
        sizeof(int), &buff2[2],
        sizeof(int), &buff2[3],
        sizeof(int), &buff2[4],
        sizeof(int), &buff2[5],
        sizeof(int), &buff2[6],
        sizeof(int), &buff2[7],
        sizeof(int), &buff2[8],
        sizeof(int), &buff2[9]
    );

    for(int i = 0; i < sizeof(buff1) / sizeof(buff1[0]); i++){
        printf("%i\n", buff1[i]++);
    }

    cym_unpack_data(buff1,
        sizeof(int), &buff2[0],
        sizeof(int), &buff2[1],
        sizeof(int), &buff2[2],
        sizeof(int), &buff2[3],
        sizeof(int), &buff2[4],
        sizeof(int), &buff2[5],
        sizeof(int), &buff2[6],
        sizeof(int), &buff2[7],
        sizeof(int), &buff2[8],
        sizeof(int), &buff2[9]
    );
    printf("\n============\n");
    for(int i = 0; i < sizeof(buff1) / sizeof(buff1[0]); i++){
        printf("%i\n", buff2[i]);
    }

    return 0;
}





