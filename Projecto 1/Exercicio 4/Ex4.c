#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    int Filepath = open("E4.txt" , O_RDWR| O_TRUNC | O_APPEND | O_CREAT, 00777 );//Abre o ficheiro E4 com permissoes de read and write, apaga o que lá estiver escrito, se o ficheiro não existir cria-o com permissoes para todos (00777)

    if (write(Filepath, "This will be the text writen in the filen", 41) != 41) {//escreve a mensagem no ficheiro aberto anteriormente, 36 é o numero de bytes que grava
        write(2, "Error: writing to E4.txtn", 25);// caso nada tenha sido escrito no ficheiro .txt  e escrito a mensagem de erro e retorna -1
        return -1;
    }

    char buffer[41];// cria tabela para guardar dados

    if(lseek(Filepath,-41,SEEK_CUR) < 0) return 1;//como o ponteiro ficou no final do ficheiro temos de andar para trás, o lseek retorna a distancia em bytes do inicio do ficheiro,SEEK_CUR significa que conta a partir do sitio onde está o ponteiro

    if(read(Filepath, buffer, 10)<0){ // lê 10 chars e grava-os no buffer do ficheiro escrito no inicio do programa
        write(2, "Error: redding E4.txtn", 22);// caso não consiga ler nada escreve a menssagem de erro apresentada e retorna -1
        return -1;
    }
    printf("%sn",buffer);// faz print de "This will" ( os primeiros 10 caracteres desde o ponteiro)


    if(lseek(Filepath,7,SEEK_CUR) < 0) return 1;// estou agora no no 10º byte do ficheiro ando 6 para a frente. 16 byte

    if(read(Filepath, buffer, 15)<0){ // lê 15 chars do mesmo sitio e grava-os no buffer
        write(2, "Error: redding E4.txtn", 22);//caso não consiga ler nada escreve a menssagem de erro apresentada e retorna -1
        return -1;
    }
    printf("%sn",buffer);// faz print de "the text writen in" ( os primeiros 15 caracteres desde o ponteiro)

    close(Filepath);// fecha o ficheiro E4.txt
    return 0;
}
