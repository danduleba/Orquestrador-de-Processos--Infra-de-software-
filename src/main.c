#include <stdio.h>
#include <string.h>

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