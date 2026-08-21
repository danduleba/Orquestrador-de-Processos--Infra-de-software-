#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_TAREFAS 100
#define MAX_ARGS 20
#define MAX_TEXTO 256

typedef struct {
    char nome[MAX_TEXTO];
    char programa[MAX_TEXTO];
    char argumentos[MAX_ARGS][MAX_TEXTO];
    int qtd_argumentos;
} Tarefa;


void cadastrar_tarefa(Tarefa tarefas[], int *qtd_tarefas) {

    char *nome = strtok(NULL, " \t");
    char *programa = strtok(NULL, " \t");

    if(nome == NULL || programa == NULL) {
        printf("uso: task <nome> <programa> [argumentos...]\n");
        return;
    }

    if(*qtd_tarefas >= MAX_TAREFAS) {
        printf("limite de tarefas atingido\n");
        return;
    }

    strcpy(tarefas[*qtd_tarefas].nome, nome);
    strcpy(tarefas[*qtd_tarefas].programa, programa);

    tarefas[*qtd_tarefas].qtd_argumentos = 0;

    char *arg;

    while((arg = strtok(NULL, " \t")) != NULL) {

        if(tarefas[*qtd_tarefas].qtd_argumentos < MAX_ARGS) {

            strcpy(tarefas[*qtd_tarefas].argumentos[tarefas[*qtd_tarefas].qtd_argumentos],arg);
            tarefas[*qtd_tarefas].qtd_argumentos++;
        }
    }

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

void executar_indice(Tarefa tarefas[], int indice){
    
    pid_t pid = fork();
    
    if(pid<0){
        printf("erro ao criar o processo\n");
    }
    else if(pid==0){
        char *args[MAX_ARGS+2];
        args[0]= tarefas[indice].programa;

        for(int i=0; i<tarefas[indice].qtd_argumentos; i++){
            args[i+1]=tarefas[indice].argumentos[i];
        }
        args[tarefas[indice].qtd_argumentos+1] = NULL;

        execvp(tarefas[indice].programa,args);
        perror("execvp");
        exit(1);
    }
    else{
        waitpid(pid,NULL,0);
    }
}

void executar_sequencial(Tarefa tarefas[], int qtd_tarefas){

    char *nome;
    int quantidade = 0;

    while((nome = strtok(NULL, " \t")) != NULL){

        quantidade++;

        int indice= buscar_tarefa(tarefas,qtd_tarefas,nome);

        if(indice == -1){
            printf("tarefa '%s' nao encontrada\n", nome);
        }
        executar_indice(tarefas,indice);
    }
    if(quantidade == 0){
        printf("uso: run sequential <tarefa1> <tarefa2> ...\n");
    }
}

void executar_paralelo(Tarefa tarefas[], int qtd_tarefas){
    char *nome;
    int quantidade =0;

    while ((nome = strtok(NULL, " \t")) != NULL){
        quantidade++;
        int indice=buscar_tarefa(tarefas,qtd_tarefas,nome);
        
        if (indice == -1){
            printf("tarefa '%s' nao encontrada\n", nome);
            return;
        }
        pid_t pid = fork();

        if(pid<0){
            printf("erro ao criar processo\n");
        }
        else if(pid == 0){
            char *args[MAX_ARGS+2];
            args[0]= tarefas[indice].programa;

            for(int i = 0; i< tarefas[indice].qtd_argumentos; i++){
                args[i + 1]= tarefas[indice].argumentos[i];
            }
            args[tarefas[indice].qtd_argumentos+1]= NULL;
            execvp(tarefas[indice].programa,args);

            perror("execvp");
            exit(1);
        }
        else{
            waitpid(pid,NULL,0);
        }

    }
    if(quantidade == 0){
        printf("uso: run parallel <tarefa1> <tarefa2> ...\n");
    }
}

void executar_pipe(Tarefa tarefas[], int qtd_tarefas){

    int indices[MAX_TAREFAS];
    pid_t pids[MAX_TAREFAS];

    int quantidade = 0;

    char *nome;

    while((nome = strtok(NULL, " \t")) != NULL){

        int indice = buscar_tarefa(tarefas,qtd_tarefas,nome);

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

                dup2(leitura_anterior,STDIN_FILENO);
            }

            if(i < quantidade - 1){

                dup2(
                    fd[1],
                    STDOUT_FILENO
                );
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

            for(int j = 0;
                j < tarefas[indice].qtd_argumentos; j++){

                args[j + 1] =tarefas[indice].argumentos[j];
            }

            args[tarefas[indice].qtd_argumentos + 1] = NULL;

            execvp(tarefas[indice].programa,args);
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

        waitpid(
            pids[i],
            NULL,
            0
        );
    }
}

void executar_tarefa(Tarefa tarefas[], int qtd_tarefas){
    char *nome = strtok(NULL, " \t");

    if(nome == NULL){
        printf("uso: run <nome>\n");
        return;
    }

    if(strcmp(nome, "sequential") ==0){
        executar_sequencial(tarefas, qtd_tarefas);
        return;
    }
    if(strcmp(nome, "parallel") ==0){
        executar_paralelo(tarefas, qtd_tarefas);
        return;
    }

    if(strcmp(nome, "pipe") == 0){

    executar_pipe(tarefas,qtd_tarefas);
    return;
}

    int indice = buscar_tarefa(tarefas,qtd_tarefas,nome);
    if (indice ==-1){
        printf("tarefa não encontrada\n");
        return;
    }
    executar_indice(tarefas,indice);    
}



int main(void) {

    char linha[1024];

    Tarefa tarefas[MAX_TAREFAS];
    int qtd_tarefas = 0;

    while(1) {

        printf("processflow> ");
        fflush(stdout);

        if(fgets(linha, sizeof(linha), stdin) == NULL) {
            break;
        }

        linha[strcspn(linha, "\n")] = '\0';

        if(strlen(linha) == 0) {
            continue;
        }

        char *comando = strtok(linha, " \t");

        if(strcmp(comando, "exit") == 0) {
            break;
        }

        if(strcmp(comando, "task") == 0) {

            cadastrar_tarefa(tarefas, &qtd_tarefas);

            continue;
        }

        if(strcmp(comando, "run") == 0){
            executar_tarefa(tarefas,qtd_tarefas);
            continue;
        }
    }


    return 0;
}