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

// Transformada Inversa 1D (IFFT)
void ifft1D(Complex *data, int n) {
    if (n <= 1) return;

    for (int i = 0; i < n; i++)
        data[i].imag = -data[i].imag;

    fft1D(data, n);

    for (int i = 0; i < n; i++) {
        data[i].real = data[i].real / n;
        data[i].imag = -data[i].imag / n;  // Conjugado de volta
    }
}

// Transformada Inversa 2D (IFFT)
void ifft2D(Complex *image, int width, int height) {
    for (int y = 0; y < height; y++)
        ifft1D(&image[y * width], width);

    for (int x = 0; x < width; x++) {
        Complex *column = (Complex *)malloc(height * sizeof(Complex));
        if (!column) {
            printf("Erro ao alocar memória para a coluna na IFFT!\n");
            exit(1);
        }

        for (int y = 0; y < height; y++)
            column[y] = image[y * width + x];

        ifft1D(column, height);

        for (int y = 0; y < height; y++)
            image[y * width + x] = column[y];

        free(column);
    }
}


void fftShift(Image fftImage) {
    int w = fftImage.width;
    int h = fftImage.height;

    int halfW = w / 2;
    int halfH = h / 2;

    // Trocar quadrantes da FFT
    for (int y = 0; y < halfH; y++) {
        for (int x = 0; x < halfW; x++) {
            int idx1 = y * w + x;
            int idx2 = (y + halfH) * w + (x + halfW);
            int idx3 = y * w + (x + halfW);
            int idx4 = (y + halfH) * w + x;

            // Swap entre quadrantes
            unsigned char temp = fftImage.data[idx1];
            fftImage.data[idx1] = fftImage.data[idx2];
            fftImage.data[idx2] = temp;

            temp = fftImage.data[idx3];
            fftImage.data[idx3] = fftImage.data[idx4];
            fftImage.data[idx4] = temp;
        }
    }
}

// Função principal
int main(int argc, char **argv) { // 1: caminho da imagem, 2: Caminho do resultado
    if (argc != 2) {
        printf("Uso: %s <imagem_entrada.pgm>\n", argv[0]);
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

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;
            imageFFT[index].real *= pow(-1, x + y);
            imageFFT[index].imag *= pow(-1, x + y);
        }
    }

    // Aplicando a FFT 2D
    fft2D(imageFFT, width, height);

    // Criando uma imagem com a magnitude da FFT
    Image fftMagnitude;
    fftMagnitude.width = width;
    fftMagnitude.height = height;
    fftMagnitude.data = (unsigned char *)malloc(width * height);

    // Criando uma imagem com a fase da FFT
    Image fftPhase;
    fftPhase.width = width;
    fftPhase.height = height;
    fftPhase.data = (unsigned char *)malloc(width * height);

    // double maxMagnitude = 0.0;
    for (int i = 0; i < width * height; i++) {
        double magnitude = sqrt(imageFFT[i].real * imageFFT[i].real + imageFFT[i].imag * imageFFT[i].imag); //sqrt(real² + imag²)
        double phase = atan2(imageFFT[i].imag, imageFFT[i].real);
        fftMagnitude.data[i] = (unsigned char)(log(1 + magnitude) * 10);
        fftPhase.data[i] = phase;
        // fftMagnitude.data[i] = (unsigned char)((log(1 + magnitude) / log(1 + 1000)) * 255);
        // fftPhase.data[i] = (unsigned char)(((phase + PI) / (2 * PI)) * 255);
        // fftMagnitude.data[i] = (unsigned char)(log(1 + magnitude) * 10);
        // if (magnitude > maxMagnitude)
        //     maxMagnitude = magnitude;
    }

    // fftShift(fftMagnitude);

    // Normalizando para 0-255
    // for (int i = 0; i < width * height; i++) {
    //     fftMagnitude.data[i] = (unsigned char)((fftMagnitude.data[i] / log(1 + maxMagnitude)) * 255);
    // }


    // double maxPhase = 2 * PI;
    // for (int i = 0; i < width * height; i++) {
    //     double phase = atan2(imageFFT[i].imag, imageFFT[i].real);
    //     fftPhase.data[i] = (unsigned char)(((phase + PI) / maxPhase) * 255); // Normalizando fase
    // }

    // Salvando a imagem
    savePGM("magnitude.pgm", fftMagnitude);
    // savePGM("phase.pgm", fftPhase);

    printf("FFT salva\n");
    
    //---------------------------------------------------

    // Reconstrução da FFT usando magnitude e fase
    for (int i = 0; i < width * height; i++) {
        // double magnitude = exp(((double)fftMagnitude.data[i] / 255.0) * log(1 + 1000)) - 1;
        // double phase = ((double)fftPhase.data[i] / 255.0) * (2 * PI) - PI;
        double magnitude = fftMagnitude.data[i];
        double phase = fftPhase.data[i];
        imageFFT[i].real = magnitude * cos(phase);
        imageFFT[i].imag = magnitude * sin(phase);
    }

    ifft2D(imageFFT, width, height);

    // Criando imagem reconstruída
    Image reconstructed = {width, height, (unsigned char *)malloc(width * height)};
    for (int i = 0; i < width * height; i++){
        reconstructed.data[i] = (unsigned char)fmax(0, fmin(255, imageFFT[i].real));
    }

    savePGM("reconstructed.pgm", reconstructed);

    printf("IFFT salva\n");

    // Liberando memória
    free(img.data);
    free(imageFFT);
    free(fftMagnitude.data);
    free(fftPhase.data);
    free(reconstructed.data);

    return 0;
}