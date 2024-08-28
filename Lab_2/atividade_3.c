// Disciplina: Programação Concorrente
// Aluno: João Assayag
// Código: Leitura de vetores de um arquivo .bin para cálculo de produto interno em threads em C

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

// Estrutura para armazenar os dados necessários para cada thread
typedef struct {
    int inicio;          // Índice inicial para a thread
    int fim;             // Índice final (não inclusivo) para a thread
    float *a;            // Ponteiro para o vetor a
    float *b;            // Ponteiro para o vetor b
    double soma_parcial; // Somatório parcial calculado pela thread
} DadosThread;

// Função executada por cada thread para calcular o somatório parcial do produto interno
void* produto_interno_thread(void* arg) {
    DadosThread *dados = (DadosThread*)arg;
    dados->soma_parcial = 0.0;

    for (int i = dados->inicio; i < dados->fim; i++) {
        dados->soma_parcial += dados->a[i] * dados->b[i];
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <número de threads> <nome do arquivo binário>\n", argv[0]);
        return 1;
    }

    int T = atoi(argv[1]);    // Número de threads
    char *nome_arquivo = argv[2]; // Nome do arquivo binário

    FILE *arquivo = fopen(nome_arquivo, "rb");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    int N;
    fread(&N, sizeof(int), 1, arquivo);  // Ler a dimensão N do arquivo

    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));

    fread(a, sizeof(float), N, arquivo); // Ler o vetor a do arquivo
    fread(b, sizeof(float), N, arquivo); // Ler o vetor b do arquivo

    double resultado_esperado;
    fread(&resultado_esperado, sizeof(double), 1, arquivo); // Ler o resultado esperado do arquivo

    fclose(arquivo); // Fecha o arquivo

    pthread_t threads[T];
    DadosThread dados_thread[T];
    int tamanho_faixa = N / T; // Tamanho da faixa de índices para cada thread

    // Inicializa e cria as threads
    for (int i = 0; i < T; i++) {
        dados_thread[i].inicio = i * tamanho_faixa;
        dados_thread[i].fim = (i == T - 1) ? N : (i + 1) * tamanho_faixa;
        dados_thread[i].a = a;
        dados_thread[i].b = b;
        pthread_create(&threads[i], NULL, produto_interno_thread, (void*)&dados_thread[i]);
    }

    double resultado = 0.0;

    
    for (int i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
        resultado += dados_thread[i].soma_parcial;
    }

    // Exibe os resultados
    printf("Produto interno calculado: %.6lf\n", resultado);
    printf("Produto interno esperado: %.6lf\n", resultado_esperado);
    printf("Erro relativo: %.6lf\n", fabs((resultado_esperado - resultado) / resultado_esperado));

    free(a);
    free(b);

    return 0;
}
