#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

int *vetor; // Vetor de inteiros que será modificado pelas threads
int *vetor_original; // Cópia do vetor original para verificação
int N, M;   // N = número de elementos no vetor, M = número de threads

// Função que inicializa o vetor com valores de 1 até N
void inicializar_vetor() {
    for (int i = 0; i < N; i++) {
        vetor[i] = i + 1; // A inicialização dos vetor é preenchida de forma sequeeincial 
    }
}

// Função executada por cada thread para somar 1 a uma parte do vetor
void *soma_um(void *arg) {
    int id = *(int *)arg;          // Identificador da thread
    int base = N / M;              // Quantidade base de elementos para cada thread
    int resto = N % M;             // Quantidade de elementos restantes após a divisão

    // Calculo de início e fim da faixa de elementos do vetor que a thread deve processar 
    int inicio = id * base + (id < resto ? id : resto);
    int fim = inicio + base + (id < resto ? 1 : 0);
    //printf("Thread %d executando: processando elementos de %d até %d\n", id, inicio+1, fim);

    // Cada thread soma 1 a cada elemento na faixa designada no vetor 
    for (int i = inicio; i < fim; i++) {
        int valor_original = vetor[i];  // Armazena o valor original do na variavel Vetor_original para verificação
        vetor[i] += 1;
        //printf("Thread %d incrementou vetor[%d] de %d para %d\n", id, i, valor_original, vetor[i]); 
    }

    pthread_exit(NULL); 
}

// Função que verifica se todos os elementos do vetor foram incrementados corretamente
void verificar_resultado() {
    int correto = 1;
    for (int i = 0; i < N; i++) {
        if (vetor[i] != vetor_original[i] + 1) {
            printf("Erro no elemento %d: esperado %d, encontrado %d\n", i, vetor_original[i] + 1, vetor[i]);
            correto = 0;
        }
    }
    if (correto) {
        printf("Todos os elementos foram incrementados corretamente!\n");
    } else {
        printf("O vetor não foi incrementado corretamente.\n");
    }
}

int main(int argc, char *argv[]) {
    // Verifica se o usuário forneceu os valores de N e M
    if (argc != 3) {
        printf("ERRO: Informe o tamanho do vetor e a quantidade de threads %s <N> <M> \n", argv[0]);
        return -1;
    }

    N = atoi(argv[1]); // Número de elementos no vetor
    M = atoi(argv[2]); // Número de threads

    // Ajusta o número de threads se for maior que o número de elementos
    if (M > N) {
        printf("Número de threads maior que o número de elementos. Ajustando número de threads para %d.\n", N);
        M = N;
    }

    vetor = (int *)malloc(N * sizeof(int));           // Aloca memória para o vetor
    vetor_original = (int *)malloc(N * sizeof(int));  // Aloca memória para a cópia do vetor original

    inicializar_vetor(); // Inicializa o vetor com valores de 1 a N

    // Cria uma cópia do vetor original
    for (int i = 0; i < N; i++) {
        vetor_original[i] = vetor[i];
    }

    pthread_t threads[M]; // Array para armazenar os identificadores das threads
    int thread_id[M];      // Array para armazenar os IDs das threads

    // Cria as threads e faz com que cada uma execute a função soma_um
    for (int i = 0; i < M; i++) {
        thread_id[i] = i;
        //printf("Criando thread %d\n", i);
        if (pthread_create(&threads[i], NULL, soma_um, &thread_id[i])) {
            printf("Erro ao criar a thread\n");
            return -1;
        }
    }

    // Espera todas as threads terminarem a execução
    for (int i = 0; i < M; i++) {
        if (pthread_join(threads[i], NULL)) {
            printf("Erro ao esperar a thread terminar\n");
            return -1;
        }
    }

    // Imprime o vetor original e o vetor final após todas as threads terem terminado
    printf("Vetor Original:\n");
    for (int i = 0; i < N; i++) {
        printf("%d ", vetor_original[i]);
    }
    printf("\n");

    printf("Vetor Final:\n");
    for (int i = 0; i < N; i++) {
        printf("%d ", vetor[i]);
    }
    printf("\n");

    // Verifica o resultado comparando com o vetor original
    verificar_resultado();

    free(vetor);           // Libera a memória alocada para o vetor
    free(vetor_original);  // Libera a memória alocada para a cópia do vetor original
    return 0;
}
