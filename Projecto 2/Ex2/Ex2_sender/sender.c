#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MAX_BYTES 10
#define PERMS 0777

struct message_structure {

    long type;
    char text[MAX_BYTES];

};


int main(int argc, char *argv[]) {

    int msgqid;// variavel que vai guardar o ID da message queue
    int key=125; // chave para ser associada à msg queue
    int fd;    // file descriptor do ficheiro aberto
    int sread=MAX_BYTES; // tamanho lido pela função read
    struct message_structure message;// estrutura com a mensagem a enviar


    message.type = 1; // tipo de mensagem

    if(argc != 2) { // verfica se o utilizador colocou o numero de argumentos certo
        printf("Error, invalid number of arguments\n");
        return-1;
    }

    // aqui é criada a queue com um ID msgqid e associada à uma key com as permissoes definidas no inicio
    if ( (msgqid=msgget(key, IPC_CREAT | PERMS)) == -1 ) {
        printf("Error trying to create/open the queue.\n");

    }

    if((fd = open(argv[1], O_RDONLY )) == -1) { // Aberto o ficheiro que se pretende ler com permissoes de apenas leitura, o nome do ficheiro é passado como argumento
        printf("Error opening the file\n");
        return(-1);
    }

    while(sread==MAX_BYTES) { // ciclo que trata de ler o ficheiro e  enviar as mensagens

        sread=read(fd,message.text , MAX_BYTES); // é lido o ficheiro que foi aberto anteriormente, sread é o numero de bytes lido



        if( msgsnd(msgqid, (struct msgbuf *) &message, sread*sizeof(char), 0) == -1) { // Adiciona a mensagem criada anteriormente à queue
            printf("Error sending the message to the queue.\n");
        }

    }


    printf("\nSender: file reading and sending is now complete.\n");




    close (fd);//  fecha o ficheiro a que o fd se refere

    return (0);

}
