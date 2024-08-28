// Disciplina: Programação Concorrente
// Aluno: João Assayag
// Código: Geração de vetores e cálculo de produto interno em threads, com saída em arquivo binário

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Função para gerar dois vetores de tamanho N com valores aleatórios entre 0 e 10
void gerar_vetores(int N, float* a, float* b) {
    for (int i = 0; i < N; i++) {
        a[i] = ((float)rand() / RAND_MAX) * 10.0;
        b[i] = ((float)rand() / RAND_MAX) * 10.0;
    }
}

// Função para calcular o produto interno de dois vetores de tamanho N
double produto_interno(int N, float* a, float* b) {
    double resultado = 0.0;
    for (int i = 0; i < N; i++) {
        resultado += a[i] * b[i];
    }
    return resultado;
}

int main(int argc, char *argv[]) {
    // Verifica se o número correto de argumentos foi passado
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <tamanho do vetor N> <nome do arquivo binário>\n", argv[0]);
        return 1;
    }

    // Converte o primeiro argumento para inteiro (tamanho N do vetor)
    int N = atoi(argv[1]);
    // Obtém o nome do arquivo binário do segundo argumento
    char *nome_arquivo = argv[2];

    // Aloca memória para os vetores
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    if (a == NULL || b == NULL) {
        fprintf(stderr, "Erro ao alocar memória para os vetores.\n");
        return 1;
    }

    // Inicializa o gerador de números aleatórios
    srand(time(NULL));
    // Gera os vetores com valores aleatórios
    gerar_vetores(N, a, b);

    // Calcula o produto interno dos vetores
    double resultado = produto_interno(N, a, b);

    // Abre o arquivo binário para escrita
    FILE *arquivo = fopen(nome_arquivo, "wb");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo para escrita");
        free(a);
        free(b);
        return 1;
    }

    // Escreve o tamanho do vetor
    fwrite(&N, sizeof(int), 1, arquivo);

    // Escreve os vetores
    fwrite(a, sizeof(float), N, arquivo);
    fwrite(b, sizeof(float), N, arquivo);

    // Escreve o resultado do produto interno
    fwrite(&resultado, sizeof(double), 1, arquivo);

    // Fecha o arquivo e libera a memória alocada
    fclose(arquivo);
    free(a);
    free(b);

    return 0;
}
