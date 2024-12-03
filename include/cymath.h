#ifndef CMATH_HEADER
#define CMATH_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CYM_ABS
#define CYM_ABS(X) (((X) > 0)? X : -X)
#endif

#ifndef CYM_FLOAT
#define CYM_FLOAT double
#endif


// função inverso da raiz quadrada (Quake III) modificada para melhor precisão e funcionalidade multiplataforma
double cym_Q_rsqrt(double number) {
    const double x2 = number * 0.5;
    const double threehalfs = 1.5;

    union {
        double d;
        uint64_t i;
    } conv = {number};

    conv.i = 0x5fe6ec85e7de30da - (conv.i >> 1);

    conv.d = conv.d * (threehalfs - (x2 * conv.d * conv.d));
    conv.d = conv.d * (threehalfs - (x2 * conv.d * conv.d));
    conv.d = conv.d * (threehalfs - (x2 * conv.d * conv.d));
    conv.d = conv.d * (threehalfs - (x2 * conv.d * conv.d));
    conv.d = conv.d * (threehalfs - (x2 * conv.d * conv.d));

    return conv.d;
}



// if the norm of vector is 0 the ouput vector will be filled with 0
CYM_FLOAT cym_normalize_vec(const CYM_FLOAT* vector, unsigned int size, CYM_FLOAT* ouput){
    CYM_FLOAT norm2 = 0;

    for(unsigned int i = 0; i < size; i+=1){
        norm2 += vector[i] * vector[i];
    }

    if(norm2 == 0.0f){
        for(unsigned int i = 0; i < size; i+=1){
            ouput[i] = 0.0f;
        }
        return 0.0f;
    }

    const CYM_FLOAT rnorm = cym_Q_rsqrt(norm2);

    for(unsigned int i = 0; i < size; i+=1){
        ouput[i] = vector[i] * rnorm;
    }

    return 1.0f / rnorm;

}

void cym_mat_transpose(const CYM_FLOAT* input, unsigned int sizex, unsigned int sizey, CYM_FLOAT* output){
    for(unsigned int i = 0; i < sizey; i+=1){
        for(unsigned int j = 0 ; j < sizex; j+=1){
            output[j * sizey + i] = input[i * sizex + j];
        }
    }
}

void cym_mat_scale(CYM_FLOAT scalar, const CYM_FLOAT* mat, unsigned int sizex, unsigned int sizey, CYM_FLOAT* output){
    for(unsigned int i = 0; i < sizey; i+=1){
        for (unsigned int j = 0; j < sizex; j+=1){
            output[i * sizex + j] = scalar * mat[i * sizex + j];
        }
    }
}

void cym_mat_sum(const CYM_FLOAT* mat1, const CYM_FLOAT* mat2, unsigned int sizex, unsigned int sizey, CYM_FLOAT* output){
    for(unsigned int i = 0; i < sizey; i+=1){
        for (unsigned int j = 0; i < sizex; j+=1){
            output[i * sizex + j] = mat1[i * sizex + j] + mat2[i * sizex + j];
        }
    }
}

void cym_mat_sub(const CYM_FLOAT* mat1, const CYM_FLOAT* mat2, unsigned int sizex, unsigned int sizey, CYM_FLOAT* output){
    for(unsigned int i = 0; i < sizey; i+=1){
        for (unsigned int j = 0; j < sizex; j+=1){
            output[i * sizex + j] = mat1[i * sizex + j] - mat2[i * sizex + j];
        }
    }
}

void cym_mat_multiply(const CYM_FLOAT* mat_1, unsigned int mat1_sizex, unsigned int mat1_sizey,
    const CYM_FLOAT* mat_2, unsigned int mat2_sizex, CYM_FLOAT* output){

    const unsigned long ran = mat1_sizey * mat2_sizex;

    for(unsigned long i = 0; i < ran; i +=1){
        output[i] = 0;
    }

    for(unsigned int i = 0; i < mat1_sizey; i += 1){
        for(unsigned int j = 0; j < mat2_sizex; j += 1)
        for(unsigned int j1 = 0; j1 < mat1_sizex; j1 += 1){
            output[i * mat2_sizex + j] += mat_1[i * mat1_sizex + j1] * mat_2[j1 * mat2_sizex + j];
        }
    }

}

// solves the system a * x = y through gauss method, outputing the result to x
// this modifies the memory at a and y
void cym_solve_gauss(CYM_FLOAT* a, CYM_FLOAT* y, unsigned int size, CYM_FLOAT* x){

    #ifdef alloca
    size_t* index = (size_t*)alloca(2 * size * sizeof(size_t));
    #else
    size_t* index = (size_t*)malloc(2 * size * sizeof(size_t));
    #endif

    // a variable to store the pivot line
    int64_t t = 0;

    for(unsigned int m = 0; m < size; m += 1){
        t = -1;
        for(unsigned int n = 0; n < size; n += 1){

            if(a[n * size + m] != 0 && t >= 0){ // if a pivot was found

                const CYM_FLOAT scale = a[n * size + m] / a[t * size + m];
                // using x to temporarilly store a[t] * (a[n][m] / a[t][m]) {a[n] is a vector/matrice_line}
                cym_mat_scale(scale, a + t * size, size, 1, x);
                // a[n] -= a[t] * (a[n][m] / a[t][m])
                cym_mat_sub(a + n * size, x, size, 1, a + n * size);
                // y[n] -= y[t] * (a[n][m] / a[t][m])
                y[n] -= y[t] * scale;

            } else if(a[n * size + m] != 0){ // otherwise set the pivot
                t = n;
            }
        }
        if(t >= 0){// if there was a pivot send its line to the end

            const CYM_FLOAT yy = y[t];

            for(unsigned int n = t + 1; n < size; n+=1){
                y[n - 1] = y[n];
            }

            y[size - 1] = yy;

            for(unsigned int i = 0; i < size; i+=1){
                x[i] = a[t * size + i];

                for(unsigned int n = t + 1; n < size; n+=1){
                    a[(n - 1) * size + i] = a[n * size + i];
                }

                a[(size - 1) * size + i] = x[i];
            }

        }
    }

    for(unsigned int m = 0; m < size; m += 1){
        for(unsigned int n = 0; n < size; n += 1){
            if(a[n * size + m]){
                x[n] = y[m] / a[n * size + m];
            }
        }
    }

    #ifndef alloca
    free(index);
    #endif
}

// takes a vector and outputs an unitary matrice with its first column proportional to that vector
// \returns 0 on success or 1 if the input vector is zero (it still writes a 0 matrice to output)
int cym_make_unitary(const CYM_FLOAT* input, unsigned int size, CYM_FLOAT* output){

    CYM_FLOAT norm2 = 0.0f;
    CYM_FLOAT rnorm = 0.0f;

    for(unsigned int i = 0; i < size; i+=1){
        norm2 += input[i] * input[i];
    }

    if(norm2 == 0.0f){
        for(size_t i = 0; i < size * size; i+=1){
            output[i] = 0.0f;
        }
        return 1;
    }

    rnorm = cym_Q_rsqrt(norm2);

    for(unsigned int i = 0; i < size; i+=1){

        output[i * size] = input[i] * rnorm;

    }
    

    unsigned int j = 1;

   while((j < size) && (input[j - 1] == 0.0f)){
        output[(j - 1) * size + j] = 1.0f;
        j += 1;
    }

    for(; j < size; j+=1){

        if(input[j] != 0.0f){
            CYM_FLOAT dummy = 0.0f;

            for(unsigned int i = 0; i < j; i+=1){
                output[i * size + j] = input[i];
                dummy -= input[i] * input[i];
            }

            output[j * (size + 1)] = dummy / input[j];

            norm2 = output[j * (size + 1)] * output[j * (size + 1)] - dummy;

            rnorm = cym_Q_rsqrt(norm2);

            for(unsigned int i = 0; i < j + 1; i+=1){
                output[i * size + j] *= rnorm;
            }
            
        } else{
            output[j * (size + 1)] = 1.0f;
        }

        
    }

    return 0;
}

int cym_test_unitary(const CYM_FLOAT* mat, unsigned int size, double accuracy){
    CYM_FLOAT dag[size * size];

    cym_mat_transpose(mat, size, size, dag);

    CYM_FLOAT I[size * size];

    cym_mat_multiply(mat, size, size, dag, size, I);

    int is_un = 1;

    for(int i = 0; i < size && is_un; i++){
        for(int j = 0; j < size && is_un; j++){
            if(i == j){
                is_un = CYM_ABS(I[i * size + j] - 1) < accuracy;
            } else{
                is_un = CYM_ABS(I[i * size + j]) < accuracy;
            }
        }
    }
    if(!is_un){
        return 0;
    }

    cym_mat_multiply(dag, size, size, mat, size, I);

    for(int i = 0; i < size && is_un; i++){
        for(int j = 0; j < size && is_un; j++){
            if(i == j){
                is_un = CYM_ABS(I[i * size + j] - 1) < accuracy;
            } else{
                is_un = CYM_ABS(I[i * size + j]) < accuracy;
            }
        }
    }

    return is_un;

}

void cym_print_mat(const CYM_FLOAT* mat, unsigned int sizex, unsigned int sizey){
    for(unsigned int i = 0; i < sizey; i++){
        for(unsigned int j = 0; j < sizex; j++){
            printf("%f, ", mat[i * sizex + j]);
        }
        printf("\n");
    }
}

// X=======================X (DATA ANALYSIS) X==================X


// performs a linear fit of the form y = A*x + B
void cym_linear_fit(const CYM_FLOAT* x, const CYM_FLOAT* y, size_t number_of_points, CYM_FLOAT* a, CYM_FLOAT* b, CYM_FLOAT* r){

    CYM_FLOAT sumx  = 0;
    CYM_FLOAT sumx2 = 0;
    CYM_FLOAT sumy  = 0;
    CYM_FLOAT sumy2 = 0;
    CYM_FLOAT sumyx = 0;

    for (size_t i = 0; i < number_of_points; i++){
        sumx  += x[i];
        sumx2 += x[i] * x[i];
        sumy  += y[i];
        sumy2 += y[i] * y[i];
        sumyx += y[i] * x[i];
    }

    const CYM_FLOAT ar_numerador  = (number_of_points * sumyx - sumx * sumy);
    const CYM_FLOAT a_denominador = (number_of_points * sumx2 - sumx * sumx);

    *a = ar_numerador / a_denominador;
    *b = (sumy - (*a) * sumx) / number_of_points;
    *r = ar_numerador * cym_Q_rsqrt((number_of_points * sumy2 - sumy * sumy) * a_denominador);
    

}

// performs a wheighted linear fit of the form y = A*x + B
int cym_rlinear_fit(const CYM_FLOAT* x, const CYM_FLOAT* y, const CYM_FLOAT* dx, const CYM_FLOAT* dy,
        size_t number_of_points, CYM_FLOAT* a, CYM_FLOAT* b, CYM_FLOAT* da, CYM_FLOAT* db ,CYM_FLOAT* r){

    CYM_FLOAT sumx  = 0;
    CYM_FLOAT sumx2 = 0;
    CYM_FLOAT sumy  = 0;
    CYM_FLOAT sumy2 = 0;
    CYM_FLOAT sumyx = 0;

    for (size_t i = 0; i < number_of_points; i++){
        sumx  += x[i];
        sumx2 += x[i] * x[i];
        sumy  += y[i];
        sumy2 += y[i] * y[i];
        sumyx += y[i] * x[i];
    }

    const CYM_FLOAT ar_numerador  = (number_of_points * sumyx - sumx * sumy);
    const CYM_FLOAT a_denominador = (number_of_points * sumx2 - sumx * sumx);

    CYM_FLOAT av = ar_numerador / a_denominador;
    CYM_FLOAT bv = (sumy - (*a) * sumx) / number_of_points;
    *r           = ar_numerador * cym_Q_rsqrt((number_of_points * sumy2 - sumy * sumy) * a_denominador);

    CYM_FLOAT last_b = 0;
    CYM_FLOAT dav    = 0;
    CYM_FLOAT dbv    = 0;

    int iterations = 0;

    while (CYM_ABS(bv - last_b) > dbv && iterations++ < 1000){
        last_b = bv;

        CYM_FLOAT sumw   = 0;
        CYM_FLOAT sumwx  = 0;
        CYM_FLOAT sumwx2 = 0;
        CYM_FLOAT sumwy  = 0;
        CYM_FLOAT sumwxy = 0;

        for (size_t i = 0; i < number_of_points; i+=1)
        {
            const CYM_FLOAT w = 1.0 / (dy[i] * dy[i] + dx[i] * dx[i] * av * av);

            sumw   += w;
            sumwx  += w * x[i];
            sumwx2 += w * x[i] * x[i];
            sumwy  += w * y[i];
            sumwxy += w * x[i] * y[i];

        }

        const CYM_FLOAT val = sumw * sumwx2 - sumwx * sumx;

        av  = (sumw * sumwxy - sumwx * sumwy) / val;
        bv  = (sumwy - sumwx * av) / sumw;
        dav = 1.0 / cym_Q_rsqrt(sumw / val);
        dbv = 1.0 / cym_Q_rsqrt(sumwx2 / val);
        

    }
    
    if(iterations > 1000){
        return -1;
    }

    *a  = av;
    *b  = bv;
    *da = dav;
    *db = dbv;

    return 0;
}


#ifdef __cplusplus
}
#endif

#endif // =====================  END OF FILE CMATH_HEADER ===========================