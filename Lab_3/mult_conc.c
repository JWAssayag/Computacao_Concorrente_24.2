/*
 * Multiplicação de matrizes MxN e NxP de forma concorrente
 * Recebendo duas matrizes em formato de arquivo binário e o número de threads
 */

#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include <pthread.h>

// #define TEST
#define TEMPO

typedef struct {
    int linhas;
    int colunas;
    float *elementos;
} Matriz;

typedef struct {
    int block_size; // Tamanho do bloco
    int id; // Identificador da thread
    int n_threads; // Número de threads
    Matriz *A, *B, *C;
} t_Args;

Matriz *le_matriz_bin(const char *arquivo) {
    Matriz *matriz = (Matriz *) malloc(sizeof(Matriz));
    FILE * descritor_arquivo; // descritor do arquivo de entrada
    size_t ret; // retorno da função de leitura no arquivo de entrada

    if (!matriz) {
        fprintf(stderr, "Erro de alocação da matriz\n");
        return NULL;
    }
    
    descritor_arquivo = fopen(arquivo, "rb");
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

    ret = fread(matriz->elementos, sizeof(float), matriz->linhas * matriz->colunas, descritor_arquivo);
    if (ret < matriz->linhas * matriz->colunas) {
        fprintf(stderr, "Erro de leitura dos elementos da matriz\n");
        return NULL;
    }

    fclose(descritor_arquivo);
    return matriz;
}

int escreve_matriz_bin(const char *arquivo, Matriz *matriz) {
    FILE * descritor_arquivo; // descritor do arquivo de saída
    size_t ret; // retorno da função de escrita no arquivo de saída

    descritor_arquivo = fopen(arquivo, "wb");  // abre o arquivo para escrita em binário
    if (!descritor_arquivo) {
        fprintf(stderr, "Erro de abertura do arquivo\n");
        return -1;
    }

    ret = fwrite(&matriz->linhas, sizeof(int), 1, descritor_arquivo);
    if (!ret) {
        fprintf(stderr, "Erro de escrita das dimensões da matriz\n");
        return -2;
    }

    ret = fwrite(&matriz->colunas, sizeof(int), 1, descritor_arquivo);
    if (!ret) {
        fprintf(stderr, "Erro de escrita das dimensões da matriz\n");
        return -3;
    }

    ret = fwrite(matriz->elementos, sizeof(float), matriz->linhas * matriz->colunas, descritor_arquivo); // escreve a matriz no arquivo
    if (ret < matriz->linhas * matriz->colunas) {
        fprintf(stderr, "Erro de escrita dos elementos da matriz\n");
        return -4;
    }

    fclose(descritor_arquivo);
    return 0;
}
 
void imprime_matriz(Matriz *matriz) {
    for (int i = 0; i < matriz->linhas; i++) {
        for (int j = 0; j < matriz->colunas; j++)
            fprintf(stdout, "%.6f ", matriz->elementos[i * matriz->colunas + j]);
        fprintf(stdout, "\n");
    }
}

void *task_multiplica_matrizes(void *arg) {
    t_Args *args = (t_Args *) arg;
    int start = args->id * args->block_size; // Início do bloco de linhas que a thread irá calcular
    int end = start + args->block_size; // Bloco de linhas que a thread irá calcular

    // Ajusta o final do bloco para a última linha da matriz se for a última thread
    if (args->id == args->n_threads - 1) {
        end = args->C->linhas;
    }

    #ifdef TEST
        printf("Thread %d: %d - %d\n", args->id, start, end);
    #endif

    for (int i = start; i < end; i++) {
        for (int j = 0; j < args->C->colunas; j++) {
            args->C->elementos[i * args->C->colunas + j] = 0; // C[i][j] = 0 (inicialização)
            for (int k = 0; k < args->A->colunas; k++) {
                args->C->elementos[i * args->C->colunas + j] += args->A->elementos[i * args->A->colunas + k] * args->B->elementos[k * args->B->colunas + j]; // C[i][j] += A[i][k] * B[k][j]
            }
        }
    }
    free(arg);
    pthread_exit(NULL);
}

void *multiplica_matrizes(Matriz *A, Matriz *B, Matriz *C, int n_threads) {

    // Ajusta o número de threads para não ser maior que o número de linhas
    if (n_threads > C->linhas) {
        n_threads = C->linhas;
        printf("Número de threads ajustado para %d (máximo de linhas da matriz)\n", n_threads);
    }

    pthread_t *tid = (pthread_t *) malloc(sizeof(pthread_t) * n_threads);
    if (!tid) {
        fprintf(stderr, "Erro de alocação das threads\n");
        return NULL;
    }

    // Divisão do trabalho entre as threads (cada thread calcula um bloco de linhas)
    int block_size = C->linhas / n_threads;

    for (int i = 0; i < n_threads; i++) {
        t_Args *arg = malloc(sizeof(t_Args));
        if (!arg) {
            fprintf(stderr, "Erro de alocação dos argumentos\n");
            return NULL;
        }

        // Inicializa os argumentos da thread i com os valores necessários e a matriz
        arg->block_size = block_size;
        arg->id = i;
        arg->n_threads = n_threads;
        arg->A = A;
        arg->B = B;
        arg->C = C;

        if (pthread_create(tid + i, NULL, task_multiplica_matrizes, (void *) arg)) {
            fprintf(stderr, "Erro: pthread_create()\n");
            return NULL;
        }
    }

    // Verifica se há linhas restantes para serem processadas pela thread principal
    if (C->linhas % n_threads) {
        #ifdef TEST
            printf("Thread principal: %d - %d\n", block_size * n_threads, C->linhas);
        #endif

        for (int i = block_size * n_threads; i < C->linhas; i++) {
            for (int j = 0; j < C->colunas; j++) {
                C->elementos[i * C->colunas + j] = 0; // C[i][j] = 0 (inicialização)
                for (int k = 0; k < A->colunas; k++) {
                    C->elementos[i * C->colunas + j] += A->elementos[i * A->colunas + k] * B->elementos[k * B->colunas + j]; // C[i][j] += A[i][k] * B[k][j]
                }
            }
        }
    }

    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(tid[i], NULL)) {
            fprintf(stderr, "Erro: pthread_join()\n");
            return NULL;
        }
    }

    free(tid);
    return NULL;
}

int main(int argc, char *argv[]) {
    Matriz *A, *B, *C;  // Matrizes de entrada e saída
    double inicio_init, fim_init, inicio_mult, fim_mult, inicio_end, fim_end; // Tempos de execução

    if (argc < 5) {
        fprintf(stderr, "Uso: %s <matriz1> <matriz2> <n_threads> <matriz_saida>\n", argv[0]);
        return 1;
    }

    int n_threads = atoi(argv[3]);
    if (n_threads < 1) {
        fprintf(stderr, "Número de threads inválido\n");
        return 2;
    }

    // Inicialização
    GET_TIME(inicio_init);
    A = le_matriz_bin(argv[1]);
    B = le_matriz_bin(argv[2]);

    if (!A || !B || A->colunas != B->linhas) {
        fprintf(stderr, "Erro de leitura das matrizes ou dimensão inválida\n");
        return 3;
    }

    C = (Matriz *) malloc(sizeof(Matriz));
    if (!C) {
        fprintf(stderr, "Erro de alocação da matriz C\n");
        return 4;
    }
    C->linhas = A->linhas;
    C->colunas = B->colunas;
    C->elementos = (float *) malloc(sizeof(float) * C->linhas * C->colunas);
    if (!C->elementos) {
        fprintf(stderr, "Erro de alocação dos elementos da matriz C\n");
        return 5;
    }
    GET_TIME(fim_init);

    // Multiplicação das Matrizes
    GET_TIME(inicio_mult);
    multiplica_matrizes(A, B, C, n_threads);
    GET_TIME(fim_mult);

    // Finalização
    GET_TIME(inicio_end);
    escreve_matriz_bin(argv[4], C);
    GET_TIME(fim_end);

    #ifdef TEMPO
        printf("Tempo de inicialização: %.8lf\n", fim_init - inicio_init);
        printf("Tempo de multiplicação: %.8lf\n", fim_mult - inicio_mult);
        printf("Tempo de finalização: %.8lf\n", fim_end - inicio_end);
        printf("Tempo total: %.8lf\n", fim_end - inicio_init);
    #endif

    #ifdef TEST
        printf("Matriz A:\n");
        imprime_matriz(A);
        printf("Matriz B:\n");
        imprime_matriz(B);
        printf("Matriz C:\n");
        imprime_matriz(C);
    #endif

    free(A->elementos);
    free(A);
    free(B->elementos);
    free(B);
    free(C->elementos);
    free(C);

    return 0;
}
