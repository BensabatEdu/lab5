/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Codigo: Comunicação entre threads com variável compartilhada e sincronização condicional */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

#define INCREMENTOS 100000

long int soma = 0; 
pthread_mutex_t mutex; 
pthread_cond_t condicao_impressao; 

// Controla o estado
int impressao_pendente = 0; 
int threads_restantes = 0; 

// essa é a função executada pelas threads de trabalho
void *ExecutaTarefa (void *arg) {
    long int id = (long int) arg;
    printf("Thread : %ld esta executando...\n", id);

    for (int i = 0; i < INCREMENTOS; i++) {
        pthread_mutex_lock(&mutex);
                
        while (impressao_pendente) {
            pthread_cond_wait(&condicao_impressao, &mutex);
        }
        soma++;
                
        if (soma % 1000 == 0) {
            impressao_pendente = 1; 
            pthread_cond_signal(&condicao_impressao); 
                      
            while (impressao_pendente) {
                pthread_cond_wait(&condicao_impressao, &mutex);
            }
        }
        
        pthread_mutex_unlock(&mutex);
    }
    
    pthread_mutex_lock(&mutex);
    threads_restantes--;

    if (threads_restantes == 0) {
        pthread_cond_signal(&condicao_impressao);
    }
    pthread_mutex_unlock(&mutex);
    
    printf("Thread : %ld terminou!\n", id);
    pthread_exit(NULL);
}

// essa é função executada pela thread de log
void *extra (void *args) {
    pthread_mutex_lock(&mutex);
    
    while (threads_restantes > 0 || impressao_pendente) {
        while (!impressao_pendente && threads_restantes > 0) {
            pthread_cond_wait(&condicao_impressao, &mutex);
        }
        
        if (impressao_pendente) {
            printf("soma = %ld\n", soma);
            impressao_pendente = 0;
            pthread_cond_broadcast(&condicao_impressao); // libera todas as threads
        }
    }
    
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

// fluxo principal da atividade
int main(int argc, char *argv[]) {
    pthread_t *identificadores_threads; 
    int numero_threads; 
    
    if(argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    numero_threads = atoi(argv[1]);
    if (numero_threads <= 0) {
        printf("Numero de threads deve ser positivo.\n");
        return 1;
    }

    identificadores_threads = (pthread_t*) malloc(sizeof(pthread_t) * (numero_threads + 1));
    if(identificadores_threads == NULL) {
        puts("ERRO--malloc");
        return 2;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condicao_impressao, NULL);
    
    threads_restantes = numero_threads;

    for(long int t = 0; t < numero_threads; t++) {
        if (pthread_create(&identificadores_threads[t], NULL, ExecutaTarefa, (void *)t)) {
            printf("--ERRO: pthread_create()\n");
            exit(-1);
        }
    }

    if (pthread_create(&identificadores_threads[numero_threads], NULL, extra, NULL)) {
        printf("--ERRO: pthread_create()\n");
        exit(-1);
    }

    for (int t = 0; t < numero_threads + 1; t++) {
        if (pthread_join(identificadores_threads[t], NULL)) {
            printf("--ERRO: pthread_join() \n");
            exit(-1);
        }
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condicao_impressao);
    free(identificadores_threads);
    
    printf("Valor de 'soma' = %ld\n", soma);

    return 0;
}