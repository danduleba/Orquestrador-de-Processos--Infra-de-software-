PROCESSFLOW - ORQUESTRADOR DE PROCESSOS

Autor: Danilo Duleba

Descrição

O ProcessFlow é um programa desenvolvido em C para gerenciamento e execução de processos em ambientes Linux/Unix.

O programa permite cadastrar tarefas e executá-las de diferentes formas, utilizando recursos do sistema operacional como fork(), execvp(), waitpid(), pipe(), dup2() e redirecionamento de entrada e saída.

Compilação

Para compilar o projeto, execute:

make

Para remover o executável gerado:

make clean

Execução

Modo interativo:

./processflow

No modo interativo será exibido o prompt:

processflow>

Modo workflow:

./processflow arquivo.pf

Nesse modo, os comandos são lidos diretamente do arquivo informado.

Comandos disponíveis

task <nome> <programa> [argumentos...]

Cadastra uma nova tarefa. O nome da tarefa deve ser único.

run <tarefa>

Executa uma tarefa cadastrada.

run sequential <tarefa1> <tarefa2> ...

Executa as tarefas em sequência, aguardando o término de cada uma antes de iniciar a próxima.

run parallel <tarefa1> <tarefa2> ...

Executa as tarefas em paralelo.

run pipe <tarefa1> <tarefa2> ...

Executa tarefas conectando a saída de uma tarefa à entrada da próxima através de pipes.

input <tarefa> <arquivo>

Configura um arquivo como entrada padrão da tarefa.

output <tarefa> <arquivo>

Configura um arquivo como saída padrão da tarefa, substituindo o conteúdo anterior.

append <tarefa> <arquivo>

Configura um arquivo como saída da tarefa, adicionando o novo conteúdo ao final do arquivo.

workdir <diretorio>

Altera o diretório de trabalho atual do ProcessFlow.

start <tarefa>

Inicia uma tarefa em background e exibe o identificador do job e o PID do processo.

jobs

Exibe os jobs iniciados em background e seus estados.

wait <jobId>

Aguarda a finalização de um determinado job.

exit

Encerra o ProcessFlow.

Arquivos do projeto

main.c
Contém o fluxo principal do programa e o processamento dos comandos.

processflow.c
Contém a implementação das funções do ProcessFlow.

processflow.h
Contém as estruturas, constantes e protótipos das funções.

Makefile
Responsável pela compilação do projeto.

evidencias.log
Contém registros de compilação, execução, testes e correções realizadas durante o desenvolvimento.

Observações

O programa foi desenvolvido para ambientes compatíveis com chamadas de sistema POSIX.

A execução de programas é feita utilizando fork() e execvp(), sem utilização de system() ou popen().