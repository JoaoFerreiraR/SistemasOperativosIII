#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/socket.h>
int main(int argc, char *argv[]){
	//primeiro tipo de IPC (PIPE)
	int fd[2]; //numpipe[1] == output || numpipe[0] == input
	pipe(fd); //cria o mecanismo de pipe com fd[0] e fd[1]
	//segundo(named pipe)
	unlink("FIFO1"); //se existir, apaga
	mkfifo("FIFO1", 0644);// FIFO
	int FIFOID;
	//sockets
	unlink("SOCKFILE"); //se existir, apaga
	//File descriptor do ficheiro(só usado na parte final do codigo)
	int Filepath ;
	
	if( fork() ==0){ //filho n1  FICHEIRO-> PIPE
		close(fd[0]); //fechar fd leitura
		dup2(fd[1],STDOUT_FILENO); //o novo stdoutput vai ser o pipe
		execlp("cat", "cat" , "/etc/passwd" ,"/etc/passwd", (char *)NULL);// comando cat
	}
	
	if(fork() == 0){ // filho n2 PIPE-> NAMED PIPE
		FIFOID=open("FIFO1", O_WRONLY); // Abre o named pipe
		close(fd[1]); // fecho o pipe fd para escrita
		dup2(FIFOID, STDOUT_FILENO); //escreve no named Pipe FIFO1 com o ID de FIFOID e substitiu o standard output com o fd da named pipe
		dup2(fd[0],STDIN_FILENO); //coloca o fd da parte de input da pipe como substituicao de standard input
		execlp("cut","cut", "-d:", "-f", "1", (char *) NULL); // comando cut 
	}
	
	if(fork()==0){// filho n3 NAMED PIPE -> SOCKET
		wait(0);
		//setup da socket de envio de dados
		int sock;
		sock = socket(AF_UNIX, SOCK_STREAM, 0); 
		struct sockaddr_un server, client; //declaracao dos endereços que vamos utilizar
		unsigned int size=sizeof(server.sun_family)+sizeof(server.sun_path); //tamanho da estrutura de endereços
		server.sun_family = AF_UNIX;
		strcpy(server.sun_path,"SOCKFILE");
		
		bind(sock, (struct sockaddr *)&server, size); //faz bind da socket sock ao endereço "server"
		listen(sock, 0); //listen() necessario para a socket receber pedidos
		
			if(fork()==0){ //filho n4  SOCKET-> FILE so vai ser corrido aquando o accept no outro filho;
				//setup da socket de receccao de dados###
				int sock1 = socket(AF_UNIX, SOCK_STREAM, 0);
				struct sockaddr_un client;
				client.sun_family = AF_UNIX;
				strcpy(client.sun_path,"SOCKFILE"); //path do client vai ser "sockfile", o mesmo que o do servidor
				unsigned int size=sizeof(client.sun_family)+strlen(client.sun_path); //tamanho da estrutura de endereços
				
				connect(sock1,(struct sockaddr *)&client, size); //conecta sock1 ao endereço client, que vai receber após o accept(e comando) no filho anterior
				
				int Ficheiro=open("file.txt", O_WRONLY | O_TRUNC | O_CREAT, 0666);
				close(1); //fecha partes de IPC's desnecessários
				close(0);
				close(fd[1]);
				close(fd[0]);
				dup2(sock1,STDIN_FILENO); //coloca o fd da socket de receccao como substituicao de standard input
				dup2(Ficheiro,STDOUT_FILENO);//coloca o fd de ficheiro como substituicao de standard output
				execlp("uniq","uniq", (char *) NULL);   //corre uniq e grava em file.txt
			}
			
		int fd2=accept(sock, (struct sockaddr *)&client, &size); //é criado o descriptor fd2 para enviar dados pela socket sock e endereço client
		FIFOID =open("FIFO1", O_RDONLY); //abre o named pipe para receber dados com fd FIFOID
		close(fd[1]);   //fecha partes de IPC's desnecessários
		close(fd[0]);   
		dup2(FIFOID, STDIN_FILENO); //coloca o fd da named pipe de receccao como substituicao de standard input
		dup2(fd2,STDOUT_FILENO); //coloca o fd da socket de envio como substituicao de standard output
		execlp("sort","sort",(char *) NULL ) ; // faz o comando sort
	}
	
	wait(NULL);    // espera por todos os filhos e pelo fim de qualquer um deles
	close(0);    //fecha partes de IPC's desnecessários
	close(fd[1]);
	close(fd[0]);
	wait(0); //espera duplamente para mais segurança
	unlink("FIFO1"); //apaga os dois ficheiros já não necessários
	unlink("SOCKFILE"); //^
	

	Filepath = open("file.txt" , O_RDWR);
	dup2(Filepath,STDIN_FILENO);
		
	execlp("less","less",(char*)NULL); //comando less usando o ficheiro file.txt
	close(Filepath);
	return (0);
}
