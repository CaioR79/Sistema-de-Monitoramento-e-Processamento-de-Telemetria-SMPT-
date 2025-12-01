# Sistema de Monitoramento e Processamento de Telemetria (SMPT)

## Objetivo

O SMPT foi desenvolvido como uma aplicação prática para demonstrar o uso de **programação concorrente** e mecanismos essenciais de **sincronização** e **Comunicação Interprocessos (IPC)** em um ambiente C/POSIX. O projeto simula a ingestão e processamento concorrente de dados de telemetria em um sistema embarcado.

## Arquitetura e Mecanismos Utilizados

Este sistema implementa o **Modelo Produtor-Consumidor** usando:

1.  **Programação Concorrente:**
    * **Processo Principal (Produtor):** Responsável por gerar os dados (mensagens).
    * **Threads POSIX (Consumidores):** Três threads (`pthread_create`, `pthread_join`) executam concorrentemente para processar as mensagens.

2.  **Comunicação Interprocessos (IPC):**
    * **Filas de Mensagens POSIX (`mqueue.h`):** Utilizada como o *buffer* de comunicação entre o Produtor e os Consumidores. A fila garante a transferência de dados **assíncrona** e **segura**.

3.  **Sincronização:**
    * **Mutex (`pthread_mutex_t`):** Utilizado para proteger o **Contador Global (`processed_count`)**, que representa a **Seção Crítica**. O `pthread_mutex_lock()` garante a **Exclusão Mútua** durante o incremento do contador, prevenindo uma *race condition*.

## Como Executar

O projeto requer um ambiente Linux ou WSL (Windows Subsystem for Linux) com o compilador GCC.

1.  **Compilação:** Certifique-se de vincular com as bibliotecas `pthread` e `rt`.
    ```bash
    gcc smpt_main.c -o smpt_main -pthread -lrt
    ```
2.  **Execução:**
    ```bash
    ./smpt_main
    ```
3.  **Limpeza:** O código utiliza `mq_unlink()` para remover a fila do sistema ao final.
