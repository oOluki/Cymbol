#ifndef CMATH_HEADER
#define CMATH_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CYM_ABS
#define CYM_ABS(X) (((X) > 0)? (X) : -(X))
#endif

#ifndef CYM_FLOAT
#define CYM_FLOAT double
#endif

static inline CYM_FLOAT cym_absf(CYM_FLOAT x);
// função inverso da raiz quadrada (Quake III) modificada para melhor precisão e funcionalidade multiplataforma
// para double
double cym_Q_rsqrt_d(double number);
// if the norm of vector is 0 the ouput vector will be filled with 0
CYM_FLOAT cym_normalize_vec(const CYM_FLOAT* vector, unsigned int size, CYM_FLOAT* ouput);
// It is NOT safe to pass one of the input matrices as output to this funtion
void cym_mat_transpose(const CYM_FLOAT* input, unsigned int sizex, unsigned int sizey, CYM_FLOAT* output);
// It is safe to pass one of the input matrices as output to this funtion
void cym_mat_scale(CYM_FLOAT scalar, const CYM_FLOAT* mat, unsigned int sizex, unsigned int sizey, CYM_FLOAT* output);
// It is safe to pass one of the input matrices as output to this funtion
void cym_mat_sum(const CYM_FLOAT* mat1, const CYM_FLOAT* mat2, unsigned int sizex, unsigned int sizey, CYM_FLOAT* output);
// It is safe to pass one of the input matrices as output to this funtion
void cym_mat_sub(const CYM_FLOAT* mat1, const CYM_FLOAT* mat2, unsigned int sizex, unsigned int sizey, CYM_FLOAT* output);
// It is NOT safe to pass one of the input matrices as output to this funtion
void cym_mat_multiply(const CYM_FLOAT* mat_1, unsigned int mat1_sizex, unsigned int mat1_sizey,const CYM_FLOAT* mat_2, unsigned int mat2_sizex, CYM_FLOAT* output);
// solves the system a * x = y through gauss method, outputing the result to x
// this modifies the memory at a and y
void cym_solve_gauss(CYM_FLOAT* a, CYM_FLOAT* y, unsigned int size, CYM_FLOAT* x);
// takes a vector and outputs an unitary matrice with its first column proportional to that vector
// It is safe to pass the input vector as output to this funtion
// \returns 0 on success or 1 if the input vector is zero (it still writes a 0 matrice to output)
int cym_make_unitary(const CYM_FLOAT* input, unsigned int size, CYM_FLOAT* output);
int cym_test_unitary(const CYM_FLOAT* mat, unsigned int size, double accuracy);
void cym_mat_print(const char* name, const CYM_FLOAT* mat, unsigned int sizex, unsigned int sizey);

// X==============X DATA ANALYSIS X=================X

// performs a polynomial interolation and outputs the coefficients to output in order of smallest power coefficient to biggest
// \returns 0 on success, 1 otherwise
int cym_interpol(const CYM_FLOAT* x, const CYM_FLOAT* y, size_t number_of_points, CYM_FLOAT* output);
// performs a linear fit of the form y = A*x + B
void cym_linear_fit(const CYM_FLOAT* x, const CYM_FLOAT* y, size_t number_of_points, CYM_FLOAT* a, CYM_FLOAT* b, CYM_FLOAT* r);
// performs a wheighted linear fit of the form y = A*x + B
int cym_rlinear_fit(const CYM_FLOAT* x, const CYM_FLOAT* y, const CYM_FLOAT* dx, const CYM_FLOAT* dy, size_t number_of_points, CYM_FLOAT* a, CYM_FLOAT* b, CYM_FLOAT* da, CYM_FLOAT* db ,CYM_FLOAT* r);
// performs a polynomial fit of the form y = sum_n a_n * x^n
int cym_poly_fit(const CYM_FLOAT* x, const CYM_FLOAT* y, size_t number_of_points, int order, CYM_FLOAT* output);
CYM_FLOAT cym_newton_method(CYM_FLOAT (*function)(CYM_FLOAT), CYM_FLOAT guess, CYM_FLOAT value, CYM_FLOAT accuracy, CYM_FLOAT step);

void cym_minimize(CYM_FLOAT* input_data, size_t data_point_count, CYM_FLOAT(*model)(CYM_FLOAT*), CYM_FLOAT* param, size_t param_count);


#ifdef CYMATH_IMPLEMENTATION


static inline CYM_FLOAT cym_absf(CYM_FLOAT x){ return x > 0? x : -x; }

double cym_Q_rsqrt_d(double number) {
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

#define cym_sqrt(VALUE) (1.0 / cym_Q_rsqrt_d(VALUE))

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

    const CYM_FLOAT rnorm = cym_Q_rsqrt_d(norm2);

    for(unsigned int i = 0; i < size; i+=1){
        ouput[i] = vector[i] * rnorm;
    }

    return 1.0f / rnorm;

}

void cym_mat_transpose(const CYM_FLOAT* input, unsigned int sizex, unsigned int sizey, CYM_FLOAT* output){
    for(unsigned int i = 0; i < sizey; i+=1){
        for(unsigned int j = 0; j < sizex; j+=1){
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
        for (unsigned int j = 0; j < sizex; j+=1){
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

void cym_solve_gauss(CYM_FLOAT* a, CYM_FLOAT* y, unsigned int size, CYM_FLOAT* x){

    // a variable to store the pivot line
    int64_t t = 0;
    // stores the number of times a line got sent to the end
    size_t number_of_swaps = 0;

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
                // making sure the value at the pivot's column was zerod as it should
                a[n * size + m] = 0;

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

    for(unsigned int i = 0; i < size; i += 1){
        x[i] = y[i] / a[i * size + i];
    }

}

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

    rnorm = cym_Q_rsqrt_d(norm2);

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

            rnorm = cym_Q_rsqrt_d(norm2);

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

void cym_mat_print(const char* name, const CYM_FLOAT* mat, unsigned int sizey, unsigned int sizex){
    if(name) printf("\n%s[%u][%u]:\n\t",name, sizey, sizex);
    else     printf("\n(NOT NAMED)[%u][%u]:\n\t", sizey, sizex);

    for(unsigned int i = 0; i < sizey; i++){
        for(unsigned int j = 0; j < sizex; j++){
            printf("%f, ", mat[i * sizex + j]);
        }
        printf("\n");
    }
    printf("\n");
}

// X===============================================X (DATA ANALYSIS) X=================================================X

int cym_interpol(const CYM_FLOAT* x, const CYM_FLOAT* y, size_t number_of_points, CYM_FLOAT* output){

    CYM_FLOAT* a = (CYM_FLOAT*)malloc(number_of_points * number_of_points * sizeof(CYM_FLOAT) + number_of_points * sizeof(CYM_FLOAT));

    CYM_FLOAT* y_ = a + number_of_points * number_of_points;

    for(size_t i = 0; i < number_of_points; i+=1){
        y_[i] = y[i];
    }

    for(size_t i = 0; i < number_of_points; i+=1){
        a[i * number_of_points] = 1.0;
        const CYM_FLOAT xn = x[i];
        for(size_t j = 1; j < number_of_points; j+=1){
            a[i * number_of_points + j] = a[i * number_of_points + j - 1] * xn;
        }
    }

    cym_solve_gauss(a, y_, number_of_points, output);

    free(a);

    return 0;
}

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
    *r = ar_numerador * cym_Q_rsqrt_d((number_of_points * sumy2 - sumy * sumy) * a_denominador);
}

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

    const CYM_FLOAT a_numerador  = (number_of_points * sumyx - sumx * sumy);
    const CYM_FLOAT a_denominador = (number_of_points * sumx2 - sumx * sumx);

    CYM_FLOAT av = a_numerador / a_denominador;
    CYM_FLOAT bv = (sumy - av * sumx) / number_of_points;
    *r           = a_numerador * cym_Q_rsqrt_d((number_of_points * sumy2 - sumy * sumy) * a_denominador);

    CYM_FLOAT last_b = bv + 1;
    CYM_FLOAT dav    = 0;
    CYM_FLOAT dbv    = 0;

    int iterations = 0;

    while(CYM_ABS(bv - last_b) > dbv && iterations++ < 1000){
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

        const CYM_FLOAT val = sumw * sumwx2 - sumwx * sumwx;

        av  = (sumw * sumwxy - sumwx * sumwy) / val;
        bv  = (sumwy - sumwx * av) / sumw;
        dav = 1.0 / cym_Q_rsqrt_d(sumw / val);
        dbv = 1.0 / cym_Q_rsqrt_d(sumwx2 / val);
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

/*
    x0, x1, x2, x3 = x0y
    x1, x2, x3, x4 = x1y
    x2, x3, x4, x5 = x2y
    x3, x4, x5, x6 = x3y
*/
int cym_poly_fit(const CYM_FLOAT* x, const CYM_FLOAT* y, size_t number_of_points, int order, CYM_FLOAT* output){

    const int columns = order + 1;
    const int rows    = order + 1;

    CYM_FLOAT* system = (CYM_FLOAT*)malloc(
        (order + 1) * (order + 1) * sizeof(CYM_FLOAT) + number_of_points * sizeof(CYM_FLOAT)
    );
    CYM_FLOAT* y__ = system + number_of_points * number_of_points;

    for(size_t m = 1; m < columns; m+=1){
        system[m] = 0;
    }

    for(size_t n = 0; n < rows; n+=1){
        system[n * columns + (columns - 1)] = 0;
    }

    y__[0] = 0;

    for(size_t i = 0; i < number_of_points; i+=1){

        const CYM_FLOAT yi = y[i];

        CYM_FLOAT xi = x[i];

        y__[0] += yi;

        for(size_t m = 1; m < columns; m+=1){

            y__[m] += xi * yi;

            system[m] += xi;
            xi *= x[i];

        }

        for(size_t n = 1; n < rows; n+=1){
            system[n * columns + (columns - 1)] += xi;
            xi *= x[i];
        }

    }

    system[0] = number_of_points;

    for(size_t n = 1; n < rows; n+=1){
        for(size_t m = 0; m < columns - 1; m+=1){

            system[n * columns + m] = system[(n - 1) * columns + (m + 1)];

        }
    }

    cym_solve_gauss(system, y__, order + 1, output); 

    free(system);

    return 0;
}


CYM_FLOAT cym_newton_method(CYM_FLOAT (*function)(CYM_FLOAT), CYM_FLOAT guess, CYM_FLOAT value, CYM_FLOAT accuracy, CYM_FLOAT step){
    
    const int max_iterations = 1000;

    int i = 0;

    CYM_FLOAT fx = function(guess) - value;

    while (CYM_ABS(fx) > accuracy && i < max_iterations){

        const CYM_FLOAT dfx = function(guess + step) - value - fx;

        guess = guess - fx / dfx;

        fx = function(guess) - value;

        i += 1;
    }
    
    
    return guess;
}

/* TODO:
typedef struct CYM_IMAT
{
    CYM_FLOAT* data;
    size_t     rows;
    size_t     columns;
    size_t     stride;
} CYM_IMAT;


static inline void cym_smatmul(const CYM_IMAT mat1, const CYM_IMAT mat2, CYM_FLOAT* output){

    for(size_t i = 0; i < mat1.rows; i+=1){
        for(size_t j = 0; j < mat2.columns; j+=1){
            output[i * mat2.columns + j] = 0;
            for(size_t n = 0; n < mat1.columns; n+=1){
                output[i * mat2.columns + j] += mat1.data[i * mat1.stride + n] * mat2.data[n * mat2.stride + j];
            }
        }
    }

}
*/
/* TODO:
void cym_multi_param_fit(CYM_FLOAT* input_data, size_t data_point_count,
    CYM_FLOAT(*model)(CYM_FLOAT*), CYM_FLOAT* param, size_t param_count){

    CYM_FLOAT cost;

    const CYM_FLOAT accuracy = 1e-6, step = 1e-6;

    CYM_FLOAT* output = (CYM_FLOAT*)malloc((data_point_count + param_count) * sizeof(CYM_FLOAT));
    CYM_FLOAT* grad   = output + data_point_count;

    CYM_IMAT w = (CYM_IMAT){
        .data    = param,
        .rows    = param_count,
        .columns = 1,
        .stride  = 1
    };
    CYM_IMAT x = (CYM_IMAT){
        .data    = input_data,
        .rows    = data_point_count,
        .columns = param_count,
        .stride  = param_count + 1
    };
    CYM_IMAT y = (CYM_IMAT){
        .data   = input_data + param_count,
        .rows   = data_point_count, .columns = 1,
        .stride = param_count + 1
    };

    int i = 0;

    do {
        
        cym_smatmul(x, w, output);

        cost = 0;
        for(size_t i = 0; i < data_point_count; i+=1){
            cost += (output[i] - y.data[i * y.stride]) * (output[i] - y.data[i * y.stride]);
        }

        for(size_t i = 0; i < param_count; i+=1){
            const CYM_FLOAT oparam = param[i];

            param[i] += step;

            cym_smatmul(x, w, output);

            CYM_FLOAT ncost = 0;
            for(size_t i = 0; i < data_point_count; i+=1){
                ncost += (output[i] - y.data[i * y.stride]) * (output[i] - y.data[i * y.stride]);
            }
            param[i] = oparam;

            grad[i] = (ncost - cost) / step;
        }
        for(size_t i = 0; i < param_count; i+=1){
            param[i] += grad[i];
        }        

    } while (cost > accuracy && i++ < 1000);
    
    free(output);

}
*/

#endif // ======================== END OF FUNCTION IMPLEMENTATIONS =========================


#ifdef __cplusplus
}
#endif

#endif // =====================  END OF FILE CMATH_HEADER ===========================