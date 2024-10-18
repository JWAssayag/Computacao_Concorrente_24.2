/*
Programa Concorrente - Leitores e Escritores com Prioridade para Escrita

Este programa para controle em operações de escrita prioritaria. Varios leitores podem ler simultaneamente,mas quando há escritores aguardando, escritor recebe prioridade.
Compilação: `gcc -o ativ3 atividade3.c list_int.c -lpthread`
Execução: `./ativ3 <numero_de_threads>`
*/



#include <stdio.h>
#include <stdlib.h>
#include "list_int.h"
#include <pthread.h>
#include "timer.h"

#define QTDE_OPS 10000000 //quantidade de operacoes sobre a lista (insercao, remocao, consulta)
#define QTDE_INI 100 //quantidade de insercoes iniciais na lista
#define MAX_VALUE 100 //valor maximo a ser inserido

//lista compartilhada iniciada 
struct list_node_s* head_p = NULL; 
//qtde de threads no programa
int nthreads;

//rwlock modificado para prioridade de escrita
pthread_mutex_t mutex; // protege o contador de leitores
pthread_cond_t cond_leitores; // condição para leitores
pthread_cond_t cond_escritores; // condição para escritores
int num_leitores = 0; // número de leitores ativos
int esperando_escritores = 0; // escritores aguardando
int escritor_ativo = 0; // indica se há um escritor ativo

// Função para inicializar o rwlock modificado
void init() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_leitores, NULL);
    pthread_cond_init(&cond_escritores, NULL);
}

// Função de lock para leitura com prioridade para escritores
void read_lock(long int id) {
    pthread_mutex_lock(&mutex);
    while (escritor_ativo || esperando_escritores > 0) {
        printf("Thread %ld bloqueada esperando escritores\n", id);
        pthread_cond_wait(&cond_leitores, &mutex);
    }
    num_leitores++;
    printf("Thread %ld iniciou leitura (Leitores ativos: %d)\n", id, num_leitores);
    pthread_mutex_unlock(&mutex);
}

// Função de unlock para leitura
void read_unlock(long int id) {
    pthread_mutex_lock(&mutex);
    num_leitores--;
    printf("Thread %ld terminou leitura (Leitores ativos: %d)\n", id, num_leitores);
    if (num_leitores == 0) {
        printf("Leitores liberaram escritores\n");
        pthread_cond_signal(&cond_escritores);
    }
    pthread_mutex_unlock(&mutex);
}

// Função de lock para escrita
void write_lock(long int id) {
    pthread_mutex_lock(&mutex);
    esperando_escritores++;
    printf("Thread %ld solicitou escrita (Escritores esperando: %d)\n", id, esperando_escritores);
    while (num_leitores > 0 || escritor_ativo) {
        pthread_cond_wait(&cond_escritores, &mutex);
    }
    esperando_escritores--;
    escritor_ativo = 1;
    printf("Thread %ld iniciou escrita\n", id);
    pthread_mutex_unlock(&mutex);
}

// Função de unlock para escrita
void write_unlock(long int id) {
    pthread_mutex_lock(&mutex);
    escritor_ativo = 0;
    printf("Thread %ld terminou escrita\n", id);
    if (esperando_escritores > 0) {
        pthread_cond_signal(&cond_escritores);
    } else {
        pthread_cond_broadcast(&cond_leitores);
        printf("Escritores liberaram leitores\n");
    }
    pthread_mutex_unlock(&mutex);
}

//tarefa das threads
void* tarefa(void* arg) {
   long int id = (long int) arg;
   int op;
   int in, out, read; 
   in=out=read = 0; 

   //realiza operacoes de consulta (98%), insercao (1%) e remocao (1%)
   for(long int i=id; i<QTDE_OPS; i+=nthreads) {
      op = rand() % 100;
      if(op<98) {
         read_lock(id); /* lock de LEITURA */    
         Member(i%MAX_VALUE, head_p);   /* Ignore return value */
         read_unlock(id);     
         read++;
      } else if(98<=op && op<99) {
         write_lock(id); /* lock de ESCRITA */    
         Insert(i%MAX_VALUE, &head_p);  /* Ignore return value */
         write_unlock(id);     
         in++;
      } else if(op>=99) {
         write_lock(id); /* lock de ESCRITA */     
         Delete(i%MAX_VALUE, &head_p);  /* Ignore return value */
         write_unlock(id);     
         out++;
      }
   }
   //registra a qtde de operacoes realizadas por tipo
   printf("Thread %ld: in=%d out=%d read=%d\n", id, in, out, read);
   pthread_exit(NULL);
}

/*-----------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   pthread_t *tid;
   double ini, fim, delta;
   
   //verifica se o numero de threads foi passado na linha de comando
   if(argc<2) {
      printf("Digite: %s <numero de threads>\n", argv[0]); return 1;
   }
   nthreads = atoi(argv[1]);

   //insere os primeiros elementos na lista
   for(int i=0; i<QTDE_INI; i++)
      Insert(i%MAX_VALUE, &head_p);  /* Ignore return value */
   

   //aloca espaco de memoria para o vetor de identificadores de threads no sistema
   tid = malloc(sizeof(pthread_t)*nthreads);
   if(tid==NULL) {  
      printf("--ERRO: malloc()\n"); return 2;
   }

   //tomada de tempo inicial
   GET_TIME(ini);
   //inicializa a variavel rwlock modificado
   init();
   
   //cria as threads
   for(long int i=0; i<nthreads; i++) {
      if(pthread_create(tid+i, NULL, tarefa, (void*) i)) {
         printf("--ERRO: pthread_create()\n"); return 3;
      }
   }
   
   //aguarda as threads terminarem
   for(int i=0; i<nthreads; i++) {
      if(pthread_join(*(tid+i), NULL)) {
         printf("--ERRO: pthread_join()\n"); return 4;
      }
   }

   //tomada de tempo final
   GET_TIME(fim);
   delta = fim-ini;
   printf("Tempo: %lf\n", delta);

   //libera o mutex e a condição
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&cond_leitores);
   pthread_cond_destroy(&cond_escritores);
   
   //libera o espaco de memoria do vetor de threads
   free(tid);
   //libera o espaco de memoria da lista
   Free_list(&head_p);

   return 0;
}  /* main */
