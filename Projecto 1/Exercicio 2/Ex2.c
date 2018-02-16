#include  <stdio.h>
#include  <signal.h>
#include  <stdlib.h>
#include <unistd.h>
void     Pressiona(int );
void 	 Desliga (int );
void     TresSegs(int );


int  main(int argc, char *argv[])
{
	sigset_t set;  //cria-se um set de sinais que iremos bloquear
	sigemptyset(&set); //limpa-se esse set para ter a certeza que está vazio
	sigaddset(&set, SIGQUIT); //adiciona-se o SIGQUIT a esse set
	sigprocmask(SIG_BLOCK, &set,NULL); //bloqueia-se todo o set e sinais que estão incluídos nele

	printf("Interrupt twice with Ctrl-C to quit.\n");
    signal(SIGINT, Pressiona);  //Redireccionar o sinal para executar a funcao Pressiona
    signal(SIGTSTP, SIG_IGN);  //ignorar o SIGTSTP

     while (1){

		  pause();
            };


     return 0;
}

void  Pressiona(int sig)
{
	signal(sig,Desliga); //definir o que sigint faz se for carregado
    signal(SIGALRM,TresSegs); //definir o que faz o sinal que alarm irá lançar em 3 segundos
    printf( "\nInterrupt again to exit.\n ");

    alarm(3);   //começa um contador que apos tres segundos vai voltar a apontar SIGINT para esta funcao, do inicio


	return;

}

void     TresSegs(int sig ){
    signal(SIGINT, Pressiona);  //vai apontar outra vez sigint para a funcao inthandler(vai ter de carregar ctrl-c 2x again) ao final de 3 segundos
   return;

}


void Desliga(int sig){
    alarm(0);   //desliga o alarme quando se carrega ctrl+c dentro do tempo limite, senao o sinal ia ser mandado a mesma
    exit(0);
}
