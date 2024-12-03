#include "../include/cymbol.h"
#include "../include/cymath.h"
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char** argv){

    CYM_FLOAT

    y[] = {1, 2, 3, 4, 5, 9},

    x[] = {1, 2, 3, 4, 5, 9},

    dy[] = {0.001, 0.001, 0.001, 0.001, 0.001, 0.001},

    dx[] = {0.001, 0.001, 0.001, 0.001, 0.001, 0.001},

    a, b, r, da, db;

    const unsigned int n = sizeof(x) / sizeof(CYM_FLOAT);

    for(size_t i = 0; i < n; i++){
        y[i] = 1/3.0* x[i];
    }

    if(cym_rlinear_fit(x, y, dx, dy, n, &a, &b, &da, &db, &r)){
        printf("fit failed\n");
    };

    printf("(%f pm %f), (%f pm %f), %f\n", a, da, b, db, r);


    return 0;
}
