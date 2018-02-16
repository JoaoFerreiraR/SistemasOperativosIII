#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

#define MAX_BYTES 10

#define PERMS 0777
#define FLAGS MSG_NOERROR | IPC_NOWAIT

struct message_structure {

    long type;
    char text[MAX_BYTES];

};

int main(int argc, char *argv[]) {

    int msgqid;// variavel que vai guardar o ID da message queue
    int key=125; // chave para ser associada à msg queue
    int fd;    // file descriptor do ficheiro aberto
    int swrote=MAX_BYTES; // tamanho da menssagem recebida
    struct message_structure message;
    long mtype=1; // tipo de mensagem que vamos ler

    if(argc != 2) { // verfica se o utilizador colocou o numero de argumentos certo
        printf("Error, invalid number of arguments\n");
        return-1;
    }


    if((msgqid=msgget(key, 0)) == -1 ) { // "liga-se" a uma queue caso já exista dá erro
        printf("Error trying to open the queue (that should already exist).\n");
    }

    if((fd = open(argv[1],O_RDWR| O_TRUNC | O_APPEND | O_CREAT, PERMS))== -1) { //Aberto um Abre o ficheiro com permissoes de read and write, apaga o que lá
        printf("Error opening the file\n");									//estiver escrito, se o ficheiro não existir cria-o com as permissoes definidas no inicio
        return(-1);
    }


    while(swrote!=-1 ) { // ciclo que trata da recepção das mensagens e da escrita das mesmas no ficheiro referido anteriormente

        if( (swrote=msgrcv(msgqid, (struct msgbuf *) &message,  sizeof(message), mtype, FLAGS )) != -1) {// recebe a mensagem do tipo mtype e guarda o  lido



            if ((  write(fd, message.text,swrote)) == -1) {//escreve a mensagem no ficheiro aberto anteriormente, swrote é o o numero de bytes lido
                printf( "Error: writing to output.txt \n");// caso nada tenha sido escrito no ficheiro   e escrito a mensagem de erro e retorna -1
            }
        }
        else {
            printf("\nReceiver: file reception and saving is now complete.\n" );
        }
    }


    msgctl(msgqid,IPC_RMID,NULL); // fecha a queue

    close(fd); //echa o ficheiro a que o fd se refere

    return (0);
}


