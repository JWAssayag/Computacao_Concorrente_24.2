// Disciplina: Programação Concorrente
// Aluno: João Assayag
/* Código: Programa que utiliza 3 threads para ler um arquivo de entrada,
 * formatar seu conteúdo e imprimir o resultado. A formatação inclui a adição
 * de quebras de linha após cada 2n + 1 caracteres, até n=10; depois, a quebra de linha
 * é adicionada a cada 10 caracteres.
 */
// Compilação: gcc -o atividade3.c -lpthread
// Execução: ./a entrada.txt

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define NTHREADS 3
#define MAX_BUFFER 10000

sem_t estado1, estado2; 
char buffer[MAX_BUFFER] = {0};
char aux_buffer[MAX_BUFFER] = {0};

// Função da Thread 1: Lê o conteúdo do arquivo
void *lerArquivo(void *arg) {
    FILE *file = fopen((char *)arg, "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        pthread_exit(NULL);
    }
    
    char temp_buffer[MAX_BUFFER];
    while (fgets(temp_buffer, sizeof(temp_buffer), file) != NULL) {
        if (strlen(buffer) + strlen(temp_buffer) < MAX_BUFFER) {
            strcat(buffer, temp_buffer);
        } else {
            printf("Erro: o buffer excedeu o tamanho máximo permitido.\n");
            break;
        }
    }
    fclose(file);
    
    sem_post(&estado1); // Libera a Thread 2
    pthread_exit(NULL);
}

// Função da Thread 2: Formata o buffer lido
void *formatarBuffer(void *arg) {
    sem_wait(&estado1); // Espera a Thread 1

    int in_idx = 0, out_idx = 0;
    int n = 0;

    while (in_idx < strlen(buffer) && out_idx < MAX_BUFFER - 1) {
        int limite = (n < 10) ? 2 * n + 1 : 10;

        for (int i = 0; i < limite && in_idx < strlen(buffer); i++) {
            aux_buffer[out_idx++] = buffer[in_idx++];
        }

        aux_buffer[out_idx++] = '\n';
        n++;
    }

    aux_buffer[out_idx] = '\0';
    sem_post(&estado2); // Libera a Thread 3
    pthread_exit(NULL);
}

// Função da Thread 3: Imprime o buffer formatado
void *imprimirBuffer(void *arg) {
    sem_wait(&estado2); // Espera a Thread 2
    printf("%s", aux_buffer); // Imprime o conteúdo formatado
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <nome_do_arquivo>\n", argv[0]);
        return 1;
    }
    
    pthread_t tid[NTHREADS];

    sem_init(&estado1, 0, 0);
    sem_init(&estado2, 0, 0);

    if (pthread_create(&tid[0], NULL, lerArquivo, argv[1])) { 
        printf("--ERRO: pthread_create()\n"); exit(-1); 
    }
    if (pthread_create(&tid[1], NULL, formatarBuffer, NULL)) { 
        printf("--ERRO: pthread_create()\n"); exit(-1); 
    }
    if (pthread_create(&tid[2], NULL, imprimirBuffer, NULL)) { 
        printf("--ERRO: pthread_create()\n"); exit(-1); 
    }

    for (int t = 0; t < NTHREADS; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join() \n"); exit(-1); 
        }
    }

    sem_destroy(&estado1);
    sem_destroy(&estado2);

    return 0;
}
