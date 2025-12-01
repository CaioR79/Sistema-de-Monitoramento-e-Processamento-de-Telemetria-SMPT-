// Define _DE, topo para habilitar usleep sem avisos
#define _DEFAULT_SOURCE

#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h> // Necessário para flags de modo (0666)

// Configurações Globais
#define QUEUE_NAME    "/smpt_queue"
#define MAX_MSG        10         // Capacidade da fila
#define MSG_SIZE       sizeof(int) // Tamanho da mensagem (apenas um inteiro)
#define NUM_CONSUMERS  3          // Número de threads consumidoras
#define TOTAL_MESSAGES 100        // Total de mensagens a serem enviadas

// Variáveis Globais de Sincronização e Comunicação
mqd_t queue_desc;                   // Descritor da Fila de Mensagens POSIX
pthread_mutex_t counter_mutex;      // Mutex para exclusão mútua do contador
long long processed_count = 0;      // Contador global (Seção Crítica)

// Threads Consumidoras
void* consumer_thread(void* arg) {
    long id = (long)arg;
    int msg_data;
    
    printf("Consumidor %ld iniciado. Aguardando mensagens...\n", id);

    while (1) {
        // IPC: mq_receive() - BLOQUEIA se a fila estiver vazia
        if (mq_receive(queue_desc, (char *)&msg_data, MSG_SIZE, 0) < 0) {
            perror("mq_receive erro");
            break;
        }

        // Simulação de processamento (Seção NÃO-Crítica)
        usleep(10000 + (rand() % 5000)); // 10ms - 15ms

        // SINCRONIZAÇÃO: Seção Crítica (Acesso ao contador)
        pthread_mutex_lock(&counter_mutex);
        
        processed_count++;  // Incremento do dado compartilhado
        
        printf("Consumidor %ld processou: %d (Total: %lld)\n", 
               id, msg_data, processed_count);
               
        pthread_mutex_unlock(&counter_mutex);
        // Fim da Seção Crítica
        
        if (processed_count >= TOTAL_MESSAGES) {
            break;
        }
    }
    
    printf("Consumidor %ld finalizado.\n", id);
    pthread_exit(NULL);
}

// Processo Principal (Produtor)
int main(void) {
    
    // 1. Inicializar Sincronização
    if (pthread_mutex_init(&counter_mutex, NULL) != 0) {
        perror("Mutex init falhou");
        return 1;
    }
    
    // 2. Configurar e Abrir Fila de Mensagens POSIX
    struct mq_attr attr = {
        .mq_maxmsg  = MAX_MSG,
        .mq_msgsize = MSG_SIZE,
        .mq_flags   = 0 // Fila bloqueante
    };

    // Remove fila antiga se existir (para evitar erros de execução anterior)
    mq_unlink(QUEUE_NAME);

    // O_CREAT: cria a fila; 0666: permissões; &attr: atributos
    queue_desc = mq_open(QUEUE_NAME, O_RDWR | O_CREAT, 0666, &attr);
    if (queue_desc == (mqd_t)-1) {
        perror("mq_open falhou");
        return 1;
    }
    printf("Sistema SMPT iniciado. Fila criada: %s\n", QUEUE_NAME);

    // 3. Criar Threads Consumidoras
    pthread_t consumers[NUM_CONSUMERS];
    for (long i = 0; i < NUM_CONSUMERS; i++) {
        if (pthread_create(&consumers[i], NULL, consumer_thread, (void*)i) != 0) {
            perror("pthread_create falhou");
            return 1;
        }
    }
    
    // 4. Processo Principal Atua como PRODUTOR
    printf("Iniciando produção de %d mensagens...\n", TOTAL_MESSAGES);
    for (int i = 1; i <= TOTAL_MESSAGES; i++) {
        int rc = mq_send(queue_desc, (const char *)&i, MSG_SIZE, 0);
        if (rc == -1) {
            perror("mq_send falhou");
            break;
        }
        usleep(100); // Pequena pausa na produção
    }
    printf("Produção finalizada. Aguardando consumidores terminarem...\n");

    // 5. Sincronização Final: Aguardar Threads (pthread_join)
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    // 6. Limpeza
    printf("\nRESULTADO FINAL: %lld mensagens processadas.\n", processed_count);
    
    pthread_mutex_destroy(&counter_mutex);
    mq_close(queue_desc);
    mq_unlink(QUEUE_NAME); // Remove a fila do sistema
    
    printf("SMPT encerrado com sucesso.\n");
    return 0;
}
