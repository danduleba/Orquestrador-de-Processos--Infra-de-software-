#ifndef PROCESSFLOW_H
#define PROCESSFLOW_H

#include <sys/types.h>

#define MAX_TAREFAS 100
#define MAX_ARGS 20
#define MAX_TEXTO 256
#define MAX_JOBS 100

typedef struct {
    char nome[MAX_TEXTO];
    char programa[MAX_TEXTO];
    char argumentos[MAX_ARGS][MAX_TEXTO];
    int qtd_argumentos;
    char entrada[MAX_TEXTO];
    char saida[MAX_TEXTO];
    int usar_append;
} Tarefa;

typedef struct {
    int id;
    pid_t pid;
    char tarefa[MAX_TEXTO];
    int ativo;
} Job;

int buscar_tarefa(Tarefa tarefas[], int qtd_tarefas, char nome[]);
void cadastrar_tarefa(Tarefa tarefas[], int *qtd_tarefas);
void configurar_redirecionamento(Tarefa tarefas[], int qtd_tarefas, char tipo[]);
void aplicar_redirecionamentos(Tarefa tarefas[], int indice);
void executar_indice(Tarefa tarefas[], int indice);
void executar_sequencial(Tarefa tarefas[], int qtd_tarefas);
void executar_paralelo(Tarefa tarefas[], int qtd_tarefas);
void executar_pipe(Tarefa tarefas[], int qtd_tarefas);
void executar_tarefa(Tarefa tarefas[], int qtd_tarefas);
void alterar_workdir(void);
void iniciar_background(Tarefa tarefas[], int qtd_tarefas, Job jobs[], int *qtd_jobs, int *proximo_job_id);
void atualizar_jobs(Job jobs[], int qtd_jobs);
void listar_jobs(Job jobs[], int qtd_jobs);
int buscar_job(Job jobs[], int qtd_jobs, int id);
void esperar_job(Job jobs[], int qtd_jobs);

#endif
