#include <stdio.h>
#include <stdlib.h>

void convertP5toP2(const char *inputFile, const char *outputFile) {
    FILE *fin = fopen(inputFile, "rb");
    if (!fin) {
        printf("Erro ao abrir o arquivo %s\n", inputFile);
        return;
    }

    // Lendo o cabeçalho do PGM P5
    char magic[3];
    int width, height, maxVal;
    fscanf(fin, "%2s", magic);
    if (magic[0] != 'P' || magic[1] != '5') {
        printf("Erro: O arquivo não está no formato P5\n");
        fclose(fin);
        return;
    }

    // Pulando comentários
    char c;
    while ((c = fgetc(fin)) == '#') {
        while (fgetc(fin) != '\n'); // Ignora a linha do comentário
    }
    ungetc(c, fin); // Devolve o último caractere lido para o fluxo

    // Lendo largura, altura e maxVal
    fscanf(fin, "%d %d %d", &width, &height, &maxVal);
    fgetc(fin); // Consome o caractere de nova linha após maxVal

    // Criando o arquivo de saída P2
    FILE *fout = fopen(outputFile, "w");
    if (!fout) {
        printf("Erro ao criar o arquivo %s\n", outputFile);
        fclose(fin);
        return;
    }

    // Escrevendo cabeçalho P2
    fprintf(fout, "P2\n%d %d\n%d\n", width, height, maxVal);

    // Lendo os valores binários e escrevendo como ASCII
    for (int i = 0; i < width * height; i++) {
        unsigned char pixel;
        fread(&pixel, sizeof(unsigned char), 1, fin);
        fprintf(fout, "%d ", pixel);
        if ((i + 1) % width == 0) fprintf(fout, "\n"); // Quebra de linha para manter o formato
    }

    // Fechando arquivos
    fclose(fin);
    fclose(fout);

    printf("Conversão concluída! Arquivo salvo como %s\n", outputFile);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <input.pgm> <output.pgm>\n", argv[0]);
        return 1;
    }

    // Definir a subpasta onde será salvo
    const char *path_out = "P2/";
    const char *path_in = "pgm/";

    // Criar o caminho completo do arquivo de saída
    char caminho_saida[256]; // Ajuste o tamanho se necessário
    char caminho_entrada[256]; // Ajuste o tamanho se necessário
    snprintf(caminho_saida, sizeof(caminho_saida), "%s%s", path_out, argv[1]);
    snprintf(caminho_entrada, sizeof(caminho_entrada), "%s%s", path_in, argv[1]);

    convertP5toP2(caminho_entrada, caminho_saida);
    return 0;
}
