#include <stdio.h>
#include <string.h>

#define MAX_TAREFAS 100
#define MAX_ARGS 20
#define MAX_TEXTO 256

typedef struct {
    char nome[MAX_TEXTO];
    char programa[MAX_TEXTO];
    char argumentos[MAX_ARGS][MAX_TEXTO];
    int qtd_argumentos;
}Tarefa;


int main(void){
    char linha[1024];

    while(1){
        printf("processflow");
        fflush(stdout);
        if(fgets(linha, sizeof(linha), stdin) == NULL){
            break;
        }
        linha[strcspn(linha, "\n")] = '\0';
        if(strcmp(linha, "exit") ==0){
            break;
        }
        if (strlen(linha) == 0){
                continue;
        }
        printf("comando nao implementado: %s\n", linha);
    }
    return 0;
}