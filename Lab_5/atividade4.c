#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

long int soma = 0; // Variável compartilhada entre as threads
pthread_mutex_t mutex; // Variável de lock para exclusão mútua

// Função executada pelas threads
void *ExecutaTarefa(void *arg) {
    long int id = (long int)arg;
    printf("Thread : %ld está executando...\n", id);

    for (int i = 0; i < 100000; i++) {
        // Entrada na seção crítica
        pthread_mutex_lock(&mutex);
        // Seção crítica
        soma++; // Incrementa a variável compartilhada
        // Saída da seção crítica
        pthread_mutex_unlock(&mutex);
    }
    printf("Thread : %ld terminou!\n", id);
    pthread_exit(NULL);
}

// Função executada pela thread de log
void *extra(void *args) {
    printf("Extra : está executando...\n");
    int count = 0;
    long int last_printed_soma = -1; // Armazena o último valor de 'soma' impresso

    while (count < 20) { // Limita a 20 impressões
        // Entrada na seção crítica
        pthread_mutex_lock(&mutex);
        // Verifica se 'soma' é múltiplo de 10 e diferente do último valor impresso
        if (soma % 10 == 0 && soma != last_printed_soma) {
            printf("soma = %ld\n", soma);
            last_printed_soma = soma; // Atualiza o último valor impresso
            count++;
        }
        // Saída da seção crítica
        pthread_mutex_unlock(&mutex);
    }

    printf("Extra : terminou!\n");
    pthread_exit(NULL);
}

// Fluxo principal
int main(int argc, char *argv[]) {
    pthread_t *tid; // Identificadores das threads no sistema
    int nthreads; // Qtde de threads (passada linha de comando)

    // Lê e avalia os parâmetros de entrada
    if (argc < 2) {
        printf("Digite: %s <número de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    // Aloca as estruturas
    tid = (pthread_t *)malloc(sizeof(pthread_t) * (nthreads + 1));
    if (tid == NULL) {
        puts("ERRO--malloc");
        return 2;
    }

    // Inicializa o mutex (lock de exclusão mútua)
    pthread_mutex_init(&mutex, NULL);

    // Cria as threads
    for (long int t = 0; t < nthreads; t++) {
        if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
            printf("--ERRO: pthread_create()\n");
            exit(-1); 
        }
    }

    // Cria thread de log
    if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
        printf("--ERRO: pthread_create()\n");
        exit(-1);
    }

    // Espera todas as threads terminarem
    for (int t = 0; t < nthreads + 1; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join() \n");
            exit(-1);
        }
    }

    // Finaliza o mutex
    pthread_mutex_destroy(&mutex);

    printf("Valor de 'soma' = %ld\n", soma);

    // Libera memória
    free(tid);

    return 0;
}
