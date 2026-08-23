#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

#include "processflow.h"


int copiar_texto(char destino[], const char origem[]) {

    size_t tamanho = strlen(origem);

    if(tamanho >= MAX_TEXTO){
        printf("texto muito longo\n");
        return 0;
    }

    memcpy(destino, origem, tamanho + 1);

    return 1;
}


void cadastrar_tarefa(Tarefa tarefas[], int *qtd_tarefas) {

    char *nome = strtok(NULL, " \t");
    char *programa = strtok(NULL, " \t");

    if(nome == NULL || programa == NULL) {
        printf("uso: task <nome> <programa> [argumentos...]\n");
        return;
    }

    if(buscar_tarefa(tarefas, *qtd_tarefas, nome) != -1){
        printf("tarefa '%s' ja cadastrada\n", nome);
        return;
    }

    if(*qtd_tarefas >= MAX_TAREFAS) {
        printf("limite de tarefas atingido\n");
        return;
    }

    Tarefa nova_tarefa = {0};

    if(!copiar_texto(nova_tarefa.nome, nome) ||
       !copiar_texto(nova_tarefa.programa, programa)){
        return;
    }

    char *arg;

    while((arg = strtok(NULL, " \t")) != NULL) {

        if(nova_tarefa.qtd_argumentos >= MAX_ARGS) {
            printf("limite de argumentos atingido\n");
            return;
        }

        if(!copiar_texto(
            nova_tarefa.argumentos[nova_tarefa.qtd_argumentos],
            arg
        )){
            return;
        }

        nova_tarefa.qtd_argumentos++;
    }

    tarefas[*qtd_tarefas] = nova_tarefa;

    printf("tarefa '%s' cadastrada\n", nome);

    (*qtd_tarefas)++;
}


int buscar_tarefa(Tarefa tarefas[], int qtd_tarefas, char nome[]) {

    for(int i = 0; i < qtd_tarefas; i++) {

        if(strcmp(tarefas[i].nome, nome) == 0) {
            return i;
        }
    }

    return -1;
}


void configurar_redirecionamento(Tarefa tarefas[], int qtd_tarefas, char tipo[]) {

    char *nome = strtok(NULL, " \t");
    char *arquivo = strtok(NULL, " \t");

    if(nome == NULL || arquivo == NULL){
        printf("uso %s <tarefa> <arquivo>\n", tipo);
        return;
    }

    int indice = buscar_tarefa(tarefas, qtd_tarefas, nome);

    if(indice == -1){
        printf("tarefa %s nao encontrada\n", nome);
        return;
    }

    if(strcmp(tipo, "input") == 0){
        copiar_texto(tarefas[indice].entrada, arquivo);
    }

    else if(strcmp(tipo, "output") == 0){

        if(copiar_texto(tarefas[indice].saida, arquivo)){
            tarefas[indice].usar_append = 0;
        }
    }

    else if(strcmp(tipo, "append") == 0){

        if(copiar_texto(tarefas[indice].saida, arquivo)){
            tarefas[indice].usar_append = 1;
        }
    }
}

void aplicar_redirecionamentos(Tarefa tarefas[], int indice) {

    int fd;

    if(tarefas[indice].entrada[0] != '\0'){

        fd = open(tarefas[indice].entrada, O_RDONLY);

        if(fd < 0){
            perror("open");
            exit(1);
        }

        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if(tarefas[indice].saida[0] != '\0'){

        if(tarefas[indice].usar_append){

            fd = open(
                tarefas[indice].saida,
                O_WRONLY | O_CREAT | O_APPEND,
                0644
            );
        }

        else{

            fd = open(
                tarefas[indice].saida,
                O_WRONLY | O_CREAT | O_TRUNC,
                0644
            );
        }

        if(fd < 0){
            perror("open");
            exit(1);
        }

        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}


void executar_indice(Tarefa tarefas[], int indice) {

    pid_t pid = fork();

    if(pid < 0){
        printf("erro ao criar o processo\n");
    }

    else if(pid == 0){

        char *args[MAX_ARGS + 2];

        args[0] = tarefas[indice].programa;

        for(int i = 0; i < tarefas[indice].qtd_argumentos; i++){
            args[i + 1] = tarefas[indice].argumentos[i];
        }

        args[tarefas[indice].qtd_argumentos + 1] = NULL;

        aplicar_redirecionamentos(tarefas, indice);

        execvp(tarefas[indice].programa, args);

        perror("execvp");
        exit(1);
    }

    else{
        waitpid(pid, NULL, 0);
    }
}


void executar_sequencial(Tarefa tarefas[], int qtd_tarefas) {

    char *nome;
    int quantidade = 0;

    while((nome = strtok(NULL, " \t")) != NULL){

        quantidade++;

        int indice = buscar_tarefa(tarefas, qtd_tarefas, nome);

        if(indice == -1){
            printf("tarefa '%s' nao encontrada\n", nome);
            return;
        }

        executar_indice(tarefas, indice);
    }

    if(quantidade == 0){
        printf("uso: run sequential <tarefa1> <tarefa2> ...\n");
    }
}


void executar_paralelo(Tarefa tarefas[], int qtd_tarefas) {

    char *nome;
    int quantidade = 0;

    int indices[MAX_TAREFAS];
    pid_t pids[MAX_TAREFAS];

    while((nome = strtok(NULL, " \t")) != NULL){

        int indice = buscar_tarefa(tarefas, qtd_tarefas, nome);

        if(indice == -1){
            printf("tarefa '%s' nao encontrada\n", nome);
            return;
        }

        indices[quantidade] = indice;
        quantidade++;
    }

    if(quantidade == 0){
        printf("uso: run parallel <tarefa1> <tarefa2> ...\n");
        return;
    }

    for(int i = 0; i < quantidade; i++){

        int indice = indices[i];

        pid_t pid = fork();

        if(pid < 0){
            printf("erro ao criar processo\n");
            return;
        }

        else if(pid == 0){

            char *args[MAX_ARGS + 2];

            args[0] = tarefas[indice].programa;

            for(int j = 0; j < tarefas[indice].qtd_argumentos; j++){
                args[j + 1] = tarefas[indice].argumentos[j];
            }

            args[tarefas[indice].qtd_argumentos + 1] = NULL;

            aplicar_redirecionamentos(tarefas, indice);

            execvp(tarefas[indice].programa, args);

            perror("execvp");
            exit(1);
        }

        else{
            pids[i] = pid;
        }
    }

    for(int i = 0; i < quantidade; i++){
        waitpid(pids[i], NULL, 0);
    }
}



void executar_pipe(Tarefa tarefas[], int qtd_tarefas) {

    int indices[MAX_TAREFAS];
    pid_t pids[MAX_TAREFAS];

    int quantidade = 0;

    char *nome;

    while((nome = strtok(NULL, " \t")) != NULL){

        int indice = buscar_tarefa(tarefas, qtd_tarefas, nome);

        if(indice == -1){
            printf("tarefa '%s' nao encontrada\n", nome);
            return;
        }

        indices[quantidade] = indice;
        quantidade++;
    }

    if(quantidade < 2){
        printf("uso: run pipe <tarefa1> <tarefa2> ...\n");
        return;
    }

    int leitura_anterior = -1;

    for(int i = 0; i < quantidade; i++){

        int fd[2];

        if(i < quantidade - 1){

            if(pipe(fd) < 0){
                perror("pipe");
                return;
            }
        }

        pid_t pid = fork();

        if(pid < 0){
            perror("fork");
            return;
        }

        else if(pid == 0){

            if(leitura_anterior != -1){
                dup2(leitura_anterior, STDIN_FILENO);
            }

            if(i < quantidade - 1){
                dup2(fd[1], STDOUT_FILENO);
            }

            if(leitura_anterior != -1){
                close(leitura_anterior);
            }

            if(i < quantidade - 1){
                close(fd[0]);
                close(fd[1]);
            }

            int indice = indices[i];

            char *args[MAX_ARGS + 2];

            args[0] = tarefas[indice].programa;

            for(int j = 0; j < tarefas[indice].qtd_argumentos; j++){
                args[j + 1] = tarefas[indice].argumentos[j];
            }

            args[tarefas[indice].qtd_argumentos + 1] = NULL;

            aplicar_redirecionamentos(tarefas, indice);

            execvp(tarefas[indice].programa, args);

            perror("execvp");
            exit(1);
        }

        else{

            pids[i] = pid;

            if(leitura_anterior != -1){
                close(leitura_anterior);
            }

            if(i < quantidade - 1){
                close(fd[1]);
                leitura_anterior = fd[0];
            }
        }
    }

    for(int i = 0; i < quantidade; i++){
        waitpid(pids[i], NULL, 0);
    }
}


void executar_tarefa(Tarefa tarefas[], int qtd_tarefas) {

    char *nome = strtok(NULL, " \t");

    if(nome == NULL){
        printf("uso: run <nome>\n");
        return;
    }

    if(strcmp(nome, "sequential") == 0){
        executar_sequencial(tarefas, qtd_tarefas);
        return;
    }

    if(strcmp(nome, "parallel") == 0){
        executar_paralelo(tarefas, qtd_tarefas);
        return;
    }

    if(strcmp(nome, "pipe") == 0){
        executar_pipe(tarefas, qtd_tarefas);
        return;
    }

    int indice = buscar_tarefa(tarefas, qtd_tarefas, nome);

    if(indice == -1){
        printf("tarefa não encontrada\n");
        return;
    }

    executar_indice(tarefas, indice);
}


void alterar_workdir(void) {

    char *diretorio = strtok(NULL, " \t");

    if(diretorio == NULL){
        printf("uso: workdir <diretorio>\n");
        return;
    }

    if(chdir(diretorio) != 0){
        perror("chdir");
        return;
    }
}


void iniciar_background(Tarefa tarefas[], int qtd_tarefas, Job jobs[], int *qtd_jobs, int *proximo_job_id) {

    char *nome = strtok(NULL, " \t");

    if(nome == NULL){
        printf("uso: start <tarefa>\n");
        return;
    }

    int indice = buscar_tarefa(tarefas, qtd_tarefas, nome);

    if(indice == -1){
        printf("tarefa nao encontrada\n");
        return;
    }

    if(*qtd_jobs >= MAX_JOBS){
        printf("limite de jobs atingido\n");
        return;
    }

    pid_t pid = fork();

    if(pid < 0){
        printf("erro ao criar processo\n");
        return;
    }

    else if(pid == 0){

        char *args[MAX_ARGS + 2];

        args[0] = tarefas[indice].programa;

        for(int i = 0; i < tarefas[indice].qtd_argumentos; i++){
            args[i + 1] = tarefas[indice].argumentos[i];
        }

        args[tarefas[indice].qtd_argumentos + 1] = NULL;

        aplicar_redirecionamentos(tarefas, indice);

        execvp(tarefas[indice].programa, args);

        perror("execvp");
        exit(1);
    }

    else{

        jobs[*qtd_jobs].id = *proximo_job_id;
        jobs[*qtd_jobs].pid = pid;

        strcpy(jobs[*qtd_jobs].tarefa, tarefas[indice].nome);

        jobs[*qtd_jobs].ativo = 1;

        printf("[%d] %d\n", jobs[*qtd_jobs].id, (int)pid);

        (*qtd_jobs)++;
        (*proximo_job_id)++;
    }
}


void atualizar_jobs(Job jobs[], int qtd_jobs) {

    for(int i = 0; i < qtd_jobs; i++){

        if(jobs[i].ativo == 1){

            pid_t resultado = waitpid(jobs[i].pid, NULL, WNOHANG);

            if(resultado == jobs[i].pid){
                jobs[i].ativo = 0;
            }
        }
    }
}


void listar_jobs(Job jobs[], int qtd_jobs) {

    atualizar_jobs(jobs, qtd_jobs);

    for(int i = 0; i < qtd_jobs; i++){

        printf(
            "[%d] %d %s %s\n",
            jobs[i].id,
            (int)jobs[i].pid,
            jobs[i].tarefa,
            jobs[i].ativo ? "executando" : "finalizado"
        );
    }
}


int buscar_job(Job jobs[], int qtd_jobs, int id) {

    for(int i = 0; i < qtd_jobs; i++){

        if(jobs[i].id == id){
            return i;
        }
    }

    return -1;
}


void esperar_job(Job jobs[], int qtd_jobs) {

    char *texto_id = strtok(NULL, " \t");

    if(texto_id == NULL){
        printf("uso: wait <jobId>\n");
        return;
    }

    char *fim;
    long valor;

    errno = 0;
    valor = strtol(texto_id, &fim, 10);

    if(errno == ERANGE || *fim != '\0' || valor < 1 || valor > INT_MAX){
        printf("jobId invalido\n");
        return;
    }

    int id = (int)valor;

    int indice = buscar_job(jobs, qtd_jobs, id);

    if(indice == -1){
        printf("job nao encontrado\n");
        return;
    }

    if(jobs[indice].ativo == 0){
        printf("job %d ja finalizado\n", id);
        return;
    }

    waitpid(jobs[indice].pid, NULL, 0);

    jobs[indice].ativo = 0;
}
