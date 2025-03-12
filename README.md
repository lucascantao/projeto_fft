
# Aplicação de FFT

Este projeto adapta uma versão do algoritmo de Cooley-Tukey para aplicar a FFT a uma imagem .pgm do tipo P2, aplicar um filtro de passa alta e então aplicar IFFT para reconstruir a imagem.
## Compilando o programa

Abra um terminal na raíz do projeto e rode

```bash
  gcc -Wall -o fft -lm -std=c99 cshift.c pgm.c fft_serial.c
```

Execute o programa

```bash
  fft p2/nome_da_imagem.pgm
```

### Use o tipo de imagem adequado

O programa suporta apenas imagens .pgm sem compressão (P2). Se sua imagem .pgm for do tipo P5 (comprimido), use o comando abaixo para converter a imagem para o formato certo

```bash
  convertere <caminho_da_imagem.pgm>
```

A imagem convertida será armazenada na pasta P2/.


## Rodando a versão paralela

Rode o arquivo com a implementacao paralela, abra o terminal e rode

```bash
  gcc -Wall -o fft_omp -lm -std=c99 cshift.c pgm.c fft_omp.c -fopenmp
```