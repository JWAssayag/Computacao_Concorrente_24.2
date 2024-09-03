/*
 * Multiplicação de matrizes MxN e NxP de forma concorrente
 * Recebe duas matrizes em arquivos binários e o número de threads
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

typedef struct {
    int linhas, colunas;
    float *elementos;
} Matriz;

typedef struct {
    int inicio, fim; // Índices de linhas que cada thread processa
    Matriz *A, *B, *C;
} t_Args;

Matriz *le_matriz_bin(const char *arquivo) {
    FILE *file = fopen(arquivo, "rb");
    if (!file) { perror("Erro ao abrir arquivo"); exit(1); }

    Matriz *matriz = malloc(sizeof(Matriz));
    fread(&matriz->linhas, sizeof(int), 1, file);
    fread(&matriz->colunas, sizeof(int), 1, file);
    matriz->elementos = malloc(matriz->linhas * matriz->colunas * sizeof(float));
    fread(matriz->elementos, sizeof(float), matriz->linhas * matriz->colunas, file);

    fclose(file);
    return matriz;
}

void escreve_matriz_bin(const char *arquivo, Matriz *matriz) {
    FILE *file = fopen(arquivo, "wb");
    if (!file) { perror("Erro ao abrir arquivo"); exit(1); }

    fwrite(&matriz->linhas, sizeof(int), 1, file);
    fwrite(&matriz->colunas, sizeof(int), 1, file);
    fwrite(matriz->elementos, sizeof(float), matriz->linhas * matriz->colunas, file);

    fclose(file);
}

void *multiplica_matriz_thread(void *arg) {
    t_Args *args = (t_Args *) arg;
    for (int i = args->inicio; i < args->fim; i++) {
        for (int j = 0; j < args->C->colunas; j++) {
            float soma = 0;
            for (int k = 0; k < args->A->colunas; k++) {
                soma += args->A->elementos[i * args->A->colunas + k] * args->B->elementos[k * args->B->colunas + j];
            }
            args->C->elementos[i * args->C->colunas + j] = soma;
        }
    }
    free(arg);
    pthread_exit(NULL);
}

void multiplica_matrizes(Matriz *A, Matriz *B, Matriz *C, int n_threads) {
    if (n_threads > C->linhas) n_threads = C->linhas; // Ajusta o número de threads

    pthread_t threads[n_threads];
    int linhas_por_thread = C->linhas / n_threads;
    int resto = C->linhas % n_threads;

    for (int i = 0; i < n_threads; i++) {
        t_Args *args = malloc(sizeof(t_Args));
        args->A = A;
        args->B = B;
        args->C = C;
        args->inicio = i * linhas_por_thread + (i < resto ? i : resto);
        args->fim = args->inicio + linhas_por_thread + (i < resto ? 1 : 0);

        pthread_create(&threads[i], NULL, multiplica_matriz_thread, (void *) args);
    }

    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Uso: %s <matriz1> <matriz2> <n_threads> <matriz_saida>\n", argv[0]);
        return 1;
    }

    int n_threads = atoi(argv[3]);
    if (n_threads < 1) { printf("Número de threads inválido\n"); return 2; }

    Matriz *A = le_matriz_bin(argv[1]);
    Matriz *B = le_matriz_bin(argv[2]);

    if (A->colunas != B->linhas) {
        printf("Erro: dimensões incompatíveis para multiplicação\n");
        return 3;
    }

    Matriz *C = malloc(sizeof(Matriz));
    C->linhas = A->linhas;
    C->colunas = B->colunas;
    C->elementos = malloc(C->linhas * C->colunas * sizeof(float));

    double inicio, fim;
    GET_TIME(inicio);
    multiplica_matrizes(A, B, C, n_threads);
    GET_TIME(fim);

    printf("Tempo de multiplicação: %.8lf\n", fim - inicio);

    escreve_matriz_bin(argv[4], C);

    free(A->elementos); free(A);
    free(B->elementos); free(B);
    free(C->elementos); free(C);

    return 0;
}
