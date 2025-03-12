#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846

// Estrutura Imagem
typedef struct {
    int width;
    int height;
    unsigned char *data;
} Image;

// Estrutura Número Complexo
typedef struct {
    double real;
    double imag;
} Complex;

// Função para carregar uma imagem PGM
Image loadPGM(const char *filename) {
    Image img;
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erro ao abrir o arquivo!\n");
        exit(1);
    }

    char magic[3];
    fscanf(file, "%2s", magic);
    if (magic[0] != 'P' || magic[1] != '5') {
        printf("Formato PGM inválido!\n");
        exit(1);
    }

    fscanf(file, "%d %d", &img.width, &img.height);
    int maxVal;
    fscanf(file, "%d", &maxVal);
    fgetc(file); // Consumir o caractere de nova linha

    img.data = (unsigned char *)malloc(img.width * img.height);
    fread(img.data, 1, img.width * img.height, file);
    fclose(file);

    return img;
}

// Função para salvar uma imagem PGM
void savePGM(const char *filename, Image img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erro ao salvar a imagem!\n");
        exit(1);
    }

    fprintf(file, "P5\n%d %d\n255\n", img.width, img.height);
    fwrite(img.data, 1, img.width * img.height, file);
    fclose(file);
}

// Função para calcular a FFT 1D (Cooley-Tukey)
void fft1D(Complex *data, int n) {                               //Recebe um array de números complexos
    if (n <= 1) return;

    Complex *even = (Complex *)malloc(n / 2 * sizeof(Complex));  //prepara dois arrays de pares e impares com metade do tamanho original
    Complex *odd = (Complex *)malloc(n / 2 * sizeof(Complex));

    for (int i = 0; i < n / 2; i++) {                            //seta os dois arrays, separando o conteudo do data, em seus respectivos indices pares e impares originais
        even[i] = data[i * 2];
        odd[i] = data[i * 2 + 1];
    }

    fft1D(even, n / 2);                                          //chama a funcao para cada array
    fft1D(odd, n / 2);

    for (int i = 0; i < n / 2; i++) {                            //
        double t = -2 * PI * i / n;
        Complex twiddle = {cos(t), sin(t)};
        Complex temp = {twiddle.real * odd[i].real - twiddle.imag * odd[i].imag,
                        twiddle.real * odd[i].imag + twiddle.imag * odd[i].real};

        data[i].real = even[i].real + temp.real;
        data[i].imag = even[i].imag + temp.imag;
        data[i + n / 2].real = even[i].real - temp.real;
        data[i + n / 2].imag = even[i].imag - temp.imag;
    }

    free(even);
    free(odd);
}

// Função para calcular a FFT 2D
void fft2D(Complex *image, int width, int height) {
    for (int y = 0; y < height; y++)
        fft1D(&image[y * width], width);

    for (int x = 0; x < width; x++) {
        Complex *column = (Complex *)malloc(height * sizeof(Complex));
        for (int y = 0; y < height; y++)
            column[y] = image[y * width + x];

        fft1D(column, height);

        for (int y = 0; y < height; y++)
            image[y * width + x] = column[y];

        free(column);
    }
}

// Função principal
int main(int argc, char **argv) { // 1: caminho da imagem, 2: Caminho do resultado
    if (argc != 3) {
        printf("Uso: %s <imagem_entrada.pgm> <imagem_saida.pgm>\n", argv[0]);
        return -1;
    }

    Image img = loadPGM(argv[1]);
    int width = img.width, height = img.height;

    // Criando um array de números complexos para a FFT
    Complex *imageFFT = (Complex *)malloc(width * height * sizeof(Complex));
    for (int i = 0; i < width * height; i++) {
        imageFFT[i].real = img.data[i];
        imageFFT[i].imag = 0.0;
    }

    // Aplicando a FFT 2D
    fft2D(imageFFT, width, height);

    // Criando uma imagem com a magnitude da FFT
    Image fftMagnitude;
    fftMagnitude.width = width;
    fftMagnitude.height = height;
    fftMagnitude.data = (unsigned char *)malloc(width * height);

    double maxMagnitude = 0.0;
    for (int i = 0; i < width * height; i++) {
        double magnitude = sqrt(imageFFT[i].real * imageFFT[i].real + imageFFT[i].imag * imageFFT[i].imag); //sqrt(real² + imag²)
        fftMagnitude.data[i] = (unsigned char)(log(1 + magnitude) * 10);
        if (magnitude > maxMagnitude)
            maxMagnitude = magnitude;
    }

    // Normalizando para 0-255
    // for (int i = 0; i < width * height; i++) {
    //     fftMagnitude.data[i] = (unsigned char)((fftMagnitude.data[i] / log(1 + maxMagnitude)) * 255);
    // }

    // Salvando a imagem
    savePGM(argv[2], fftMagnitude);

    printf("FFT salva como %s\n", argv[2]);

    // Liberando memória
    free(img.data);
    free(imageFFT);
    free(fftMagnitude.data);

    return 0;
}
