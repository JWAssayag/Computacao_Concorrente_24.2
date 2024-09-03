/*
 * Multiplicação de matrizes MxN e NxP de forma sequencial
 * Recebendo duas matrizes em formato de arquivo binário
 */

#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

#define TEMPO

typedef struct {
    int linhas;
    int colunas;
    float *elementos;
} Matriz;

Matriz *le_matriz_bin(const char *arquivo) {
    Matriz *matriz = (Matriz *) malloc(sizeof(Matriz));
    FILE * descritor_arquivo; //descritor do arquivo de entrada
    size_t ret; //retorno da funcao de leitura no arquivo de entrada

    if (!matriz) {
        fprintf(stderr, "Erro de alocação da matriz\n");
        return NULL;
    }
    
    descritor_arquivo = fopen(arquivo, "rb"); //abre o arquivo para leitura binária
    if (!descritor_arquivo) {
        fprintf(stderr, "Erro de abertura do arquivo\n");
        return NULL;
    }

    ret = fread(&matriz->linhas, sizeof(int), 1, descritor_arquivo);
    if (!ret) {
        fprintf(stderr, "Erro de leitura das dimensões da matriz\n");
        return NULL;
    }

    ret = fread(&matriz->colunas, sizeof(int), 1, descritor_arquivo);
    if (!ret) {
        fprintf(stderr, "Erro de leitura das dimensões da matriz\n");
        return NULL;
    }

    matriz->elementos = (float *) malloc(sizeof(float) * matriz->linhas * matriz->colunas);
    if (!matriz->elementos) {
        fprintf(stderr, "Erro de alocação da matriz\n");
        return NULL;
    }

    ret = fread(matriz->elementos, sizeof(float), matriz->linhas * matriz->colunas, descritor_arquivo); //leitura dos elementos da matriz
    if (ret < matriz->linhas * matriz->colunas) {
        fprintf(stderr, "Erro de leitura dos elementos da matriz\n");
        return NULL;
    }

    return matriz;
}

void escreve_matriz_bin(const char *arquivo, Matriz *matriz) {
    FILE * descritor_arquivo; //descritor do arquivo de saida
    size_t ret; //retorno da funcao de escrita no arquivo de saida

    descritor_arquivo = fopen(arquivo, "wb"); // abre o arquivo para escrita em binario
    if (!descritor_arquivo) {
        fprintf(stderr, "Erro de abertura do arquivo\n");
        return;
    }

    ret = fwrite(&matriz->linhas, sizeof(int), 1, descritor_arquivo);
    if (!ret) {
        fprintf(stderr, "Erro de escrita das dimensões da matriz\n");
        return;
    }

    ret = fwrite(&matriz->colunas, sizeof(int), 1, descritor_arquivo);
    if (!ret) {
        fprintf(stderr, "Erro de escrita das dimensões da matriz\n");
        return;
    }

    ret = fwrite(matriz->elementos, sizeof(float), matriz->linhas * matriz->colunas, descritor_arquivo); //escrita dos elementos da matriz
    if (ret < matriz->linhas * matriz->colunas) {
        fprintf(stderr, "Erro de escrita dos elementos da matriz\n");
        return;
    }
}
 
void imprime_matriz(Matriz *matriz) {
    for (int i = 0; i < matriz->linhas; i++) {
        for (int j = 0; j < matriz->colunas; j++)
            fprintf(stdout, "%.6f ", matriz->elementos[i * matriz->colunas + j]);
        fprintf(stdout, "\n");
    }
}

void *multiplica_matrizes(Matriz *A, Matriz *B, Matriz *C) {
    // A: MxN, B: NxP, C: MxP
    // C = A * B
    for (int i = 0; i < C->linhas; i++) {
        for (int j = 0; j < C->colunas; j++) {
            C->elementos[i * C->colunas + j] = 0; // C[i][j] = 0 (inicializa com zero)
            for (int k = 0; k < A->colunas; k++)
                C->elementos[i * C->colunas + j] += A->elementos[i * A->colunas + k] * B->elementos[k * B->colunas + j]; // C[i][j] += A[i][k] * B[k][j]
        }
    }
    return C;
}

int main(int argc, char *argv[]) {
    Matriz *A, *B, *C;
    double inicio_init, fim_init, inicio_mult, fim_mult, inicio_end, fim_end; // Tempos de execução

    if (argc < 4) {
        fprintf(stderr, "Digite: %s <matriz1> <matriz2> <matriz_saida>\n", argv[0]);
        return 1;
    }

    // Inicialização
    GET_TIME(inicio_init);
    A = le_matriz_bin(argv[1]);
    if (!A) return 2;

    B = le_matriz_bin(argv[2]);
    if (!B) return 3;

    if (A->colunas != B->linhas) {
        fprintf(stderr, "Erro: Matrizes incompatíveis para multiplicação\n");
        return 4;
    }

    C = (Matriz *) malloc(sizeof(Matriz));
    if (!C) {
        fprintf(stderr, "Erro de alocação da matriz\n");
        return 5;
    }
    // Inicializa matriz C
    C->linhas = A->linhas;
    C->colunas = B->colunas;
    C->elementos = (float *) malloc(sizeof(float) * C->linhas * C->colunas);
    if (!C->elementos) {
        fprintf(stderr, "Erro de alocação da matriz\n");
        return 6;
    }
    GET_TIME(fim_init);

    // Multiplicação ou processamento
    GET_TIME(inicio_mult);
    multiplica_matrizes(A, B, C);
    GET_TIME(fim_mult);

    // Finalização
    GET_TIME(inicio_end);
    // imprime_matriz(C);
    escreve_matriz_bin(argv[3], C);

    free(A->elementos);
    free(A);
    free(B->elementos);
    free(B);
    free(C->elementos);
    free(C);
    GET_TIME(fim_end);

    #ifdef TEMPO
    // Exporta os tempos de execução em formato CSV
    //fprintf(stdout, "inicializacao, multiplicacao, finalizacao, total, threads\n");
        fprintf(stdout, "%f, %f, %f, %f, seq\n", fim_init - inicio_init, fim_mult - inicio_mult, fim_end - inicio_end, fim_end - inicio_init);
    #endif

    return 0;
}
    