#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846

typedef struct {
    double real;
    double imag;
} Complex;

void fft(Complex *a, int n) {
    if (n <= 1) return;
    
    Complex *even = (Complex*) malloc(n/2 * sizeof(Complex));
    Complex *odd = (Complex*) malloc(n/2 * sizeof(Complex));
    
    for (int i = 0; i < n / 2; i++) {
        even[i] = a[i*2];
        odd[i] = a[i*2+1];
    }
    
    fft(even, n/2);
    fft(odd, n/2);
    
    for (int i = 0; i < n / 2; i++) {
        double t = -2 * PI * i / n;
        Complex exp_i = {cos(t), sin(t)};
        Complex odd_component = {exp_i.real * odd[i].real - exp_i.imag * odd[i].imag,
                                 exp_i.real * odd[i].imag + exp_i.imag * odd[i].real};
        a[i].real = even[i].real + odd_component.real;
        a[i].imag = even[i].imag + odd_component.imag;
        a[i + n/2].real = even[i].real - odd_component.real;
        a[i + n/2].imag = even[i].imag - odd_component.imag;
    }
    
    free(even);
    free(odd);
}

void transform_2d(Complex **data, int width, int height) {
    // Aplicar FFT em cada linha
    for (int i = 0; i < height; i++) {
        fft(data[i], width);
    }
    
    // Aplicar FFT em cada coluna
    Complex *column = (Complex*) malloc(height * sizeof(Complex));
    for (int j = 0; j < width; j++) {
        for (int i = 0; i < height; i++) {
            column[i] = data[i][j];
        }
        fft(column, height);
        for (int i = 0; i < height; i++) {
            data[i][j] = column[i];
        }
    }
    free(column);
}

void save_pgm(const char *filename, Complex **data, int width, int height) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Erro ao abrir o arquivo para escrita!\n");
        return;
    }
    fprintf(file, "P2\n%d %d\n255\n", width, height);
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int magnitude = (int)(log(1 + sqrt(data[i][j].real * data[i][j].real + data[i][j].imag * data[i][j].imag)) * 255 / log(256));
            fprintf(file, "%d ", magnitude);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

Complex** read_pgm(const char *filename, int *width, int *height) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erro ao abrir o arquivo!\n");
        return NULL;
    }
    
    char buffer[16];
    fgets(buffer, sizeof(buffer), file);
    if (buffer[0] != 'P' || buffer[1] != '2') {
        printf("Formato inválido!\n");
        fclose(file);
        return NULL;
    }
    
    fscanf(file, "%d %d", width, height);
    int max_val;
    fscanf(file, "%d", &max_val);
    
    Complex **data = (Complex**) malloc(*height * sizeof(Complex*));
    for (int i = 0; i < *height; i++) {
        data[i] = (Complex*) malloc(*width * sizeof(Complex));
        for (int j = 0; j < *width; j++) {
            int pixel;
            fscanf(file, "%d", &pixel);
            data[i][j].real = pixel;
            data[i][j].imag = 0;
        }
    }
    fclose(file);
    return data;
}

int main(int argc, char **argv) {
    int width, height;
    Complex **image = read_pgm(argv[1], &width, &height);
    if (!image) return 1;
    
    transform_2d(image, width, height);
    save_pgm(argv[2], image, width, height);
    
    for (int i = 0; i < height; i++) free(image[i]);
    free(image);
    return 0;
}
