#include <stdio.h>
#include <string.h>

#include "processflow.h"


int main(int argc, char *argv[]) {

    char linha[1024];

    Tarefa tarefas[MAX_TAREFAS];
    int qtd_tarefas = 0;

    Job jobs[MAX_JOBS];
    int qtd_jobs = 0;
    int proximo_job_id = 1;

    FILE *entrada = stdin;
    int modo_workflow = 0;


    if(argc > 2){

        printf("uso: %s [workflowFile]\n", argv[0]);

        return 1;
    }


    if(argc == 2){

        entrada = fopen(argv[1], "r");

        if(entrada == NULL){

            perror("fopen");

            return 1;
        }

        modo_workflow = 1;
    }


    while(1) {

        if(!modo_workflow){

            printf("processflow> ");
            fflush(stdout);
        }


        if(fgets(linha, sizeof(linha), entrada) == NULL){

            break;
        }


        linha[strcspn(linha, "\n")] = '\0';


        if(modo_workflow){

            printf("%s\n", linha);
        }


        if(strlen(linha) == 0){

            continue;
        }


        char *comando = strtok(linha, " \t");


        if(strcmp(comando, "input") == 0){

            configurar_redirecionamento(tarefas, qtd_tarefas, "input");

            continue;
        }


        if(strcmp(comando, "output") == 0){

            configurar_redirecionamento(tarefas, qtd_tarefas, "output");

            continue;
        }


        if(strcmp(comando, "append") == 0){

            configurar_redirecionamento(tarefas, qtd_tarefas, "append");

            continue;
        }


        if(strcmp(comando, "jobs") == 0){

            listar_jobs(jobs, qtd_jobs);

            continue;
        }


        if(strcmp(comando, "exit") == 0){

            break;
        }


        if(strcmp(comando, "start") == 0){

            iniciar_background(tarefas, qtd_tarefas, jobs, &qtd_jobs, &proximo_job_id);

            continue;
        }


        if(strcmp(comando, "wait") == 0){

            esperar_job(jobs, qtd_jobs);

            continue;
        }


        if(strcmp(comando, "workdir") == 0){

            alterar_workdir();

            continue;
        }


        if(strcmp(comando, "task") == 0){

            cadastrar_tarefa(tarefas, &qtd_tarefas);

            continue;
        }


        if(strcmp(comando, "run") == 0){

            executar_tarefa(tarefas, qtd_tarefas);

            continue;
        }


        printf("comando nao reconhecido\n");
    }


    if(modo_workflow){

        fclose(entrada);
    }


    return 0;
}
