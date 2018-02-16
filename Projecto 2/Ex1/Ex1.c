/*-----------------------------------------------------------------------*/
/*                  Ex1.c                                                */
/*		Joao Antonio Rodrigues Ferreira   2013139657					 */
/*		Jose Pedro Soares Castanheira     2013139490					 */
/*																		 */
/*-----------------------------------------------------------------------*/
/*  Compilation: make                                                    */
/*-----------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>

/*-----------------------------------------------------------------------*/
/*                  macro definitions                                    */
/*-----------------------------------------------------------------------*/
#define NUM_WRITERS 22
#define NUM_READERS 20
#define LIST_SIZE 500

/*-----------------------------------------------------------------------*/
/*                  type definitions                                     */
/*-----------------------------------------------------------------------*/
typedef struct {
    sem_t mutex;
    sem_t stop_writers;
    int tail;
    int head;
    int readers;
    int slots[LIST_SIZE];
} mem_structure;

struct DadosArgumento {
    mem_structure *queue;
    int NumeroReader;
};

/*-----------------------------------------------------------------------*/
/*                  global variables definitions                         */
/*-----------------------------------------------------------------------*/
int shmfd;
int sem1;
int sem2;
pthread_t ThreadR[NUM_READERS];
pthread_t ThreadW[NUM_WRITERS];
mem_structure *ParaFechar; //este ponteiro vai ter os mesmos dados que a queue criada no main, para depois poder ser feito o cleanup com esta.
/*-----------------------------------------------------------------------*/
/*                  cleanup()                                            */
/*-----------------------------------------------------------------------*/
void cleanup(int signo) {
    pid_t FilhosTodos;
    FilhosTodos= 0; //FilhosTodos serve para identificar ambos os filhos (que teem PID = 0)
    int clean = 0;  //usei um inteiro para não ter de adicionar outra biblioteca para usar booleanos
    /*insert here code to terminate the POTW and POTR, and to wait for their termination*/

    kill(FilhosTodos, SIGTERM); //envia o sinaal SIGTERM ao processo com o ID 0(ou seja, os filhos)
    while( clean == 0) {

        sleep(1);
        if( waitpid(FilhosTodos,NULL,0) == 0) { //espera pelos filhos terem finalizado
            clean = 1;  //se esperou pelos filhos todos, está limpo 
        }
        else {
            kill(FilhosTodos, SIGKILL); //mata "abruptamente os processos com pid 0 (filhos)
            clean = 1;
        }


    }

    /*insert here code to unmap and remove the shared memory area, and to destroy the semaphores*/
    sem_destroy(&(ParaFechar->mutex));  //destroi o semaforo mutex
    sem_destroy(&(ParaFechar->stop_writers)); //destroi o semaforo stop_writers
    munmap((void *)ParaFechar, sizeof(mem_structure)); //faz o unmap da shared memory
    shm_unlink("/area"); //unlink (remove) da shared memory
    exit(0);
}


/*-----------------------------------------------------------------------*/
/*                  next()                                               */
/*-----------------------------------------------------------------------*/
int next(int pos) {

    return (pos + 1) % LIST_SIZE;
}

/*-----------------------------------------------------------------------*/
/*                  get_code()                                           */
/*-----------------------------------------------------------------------*/
int get_code() {

    return 1 + (int) ( random() % 1000 );
}

/*-----------------------------------------------------------------------*/
/*                  do_write()                                           */
/*-----------------------------------------------------------------------*/
void do_write(int n_writer, mem_structure *queue) {

    queue->slots[queue->head] = get_code();
    fprintf(stderr, "CENTRAL: %d received call %d at position %d.\n",
            n_writer, queue->slots[queue->head], queue->head);
    queue->head = next(queue->head);
}

/*-----------------------------------------------------------------------*/
/*                  writer()                                             */
/*-----------------------------------------------------------------------*/

void writer(void *Argumento)
{
    struct DadosArgumento *dados = (struct DadosArgumento *)Argumento;
    while(1) {


        /*#################################Os seguintes passos foram baseados no quadro fornecido pelo Professor no enunciado######### */

        sem_wait(&(dados->queue->stop_writers)); // reserva o semaforo e não permite que mais ninguem execute funções relativas a escrita sem ser o nosso processo


        do_write(dados->NumeroReader, dados->queue);  // operação de escrita



        sem_post(&(dados->queue->stop_writers)); // liberta o semaforo, deixando que outros processos executem operações de escrita


        sleep(random()%11);
    }
}
/*-----------------------------------------------------------------------*/
/*                  do_read()                                            */
/*-----------------------------------------------------------------------*/
void do_read(int pos, int n_reader, mem_structure *queue) {

    if (pos != queue->head)
        fprintf(stderr, "OPERATOR %d answered call %d from position %d.\n",
                n_reader, queue->slots[pos], pos);
    else
        fprintf(stderr, "OPERADOR %d - There are no calls.\n", n_reader);
}

/*-----------------------------------------------------------------------*/
/*                  reader()                                             */
/*-----------------------------------------------------------------------*/



struct DadosArgumento DadosArgumentos[NUM_READERS];

void *reader(void *Argumento) {


    struct DadosArgumento *dados = (struct DadosArgumento *)Argumento;

    int tailocal;

    while(1) {

        /*#################A lógica dos seguintes passos foi baseada no enunciado e na ajuda que o professor deu neles######################*/

        sem_wait(&(dados->queue->mutex)); // reserva o semaforo de leitura para garantir que enquanto obtemos valores da queue ou a actualizamos mais ninguem o faz
        tailocal = dados->queue->tail; // tail local = tail da queue
        (dados->queue->readers) = (dados->queue->readers) +1; //

        if (dados->queue->tail != dados->queue->head) { //  passa ao próximo na queue até lista estar preenchida
            dados->queue->tail = next((dados->queue->tail));
        }

        if (dados->queue->readers == 1) {
            sem_wait(&(dados->queue->stop_writers)); //quando existir um reader, bloqueamos o semaphoro para mais nenhum processo conseguir escrever.
        }

        sem_post(&(dados->queue->mutex)); // libertamos o semaforo de leitura pois enquanto se faz a leitura em si e se mada para o ecra a informação da chamada atendida pelo operador se possam fazer mais leituras
        do_read(tailocal, dados->NumeroReader, dados->queue); //executa o read com os valores agora passádos como parametros com a ajuda desta funcao


        sem_wait(&(dados->queue->mutex)); // reserva outra vez o semaforo pelo motivo anterior
        (dados->queue->readers)=(dados->queue->readers)-1;


        if (dados->queue->readers == 0) {
            sem_post(&(dados->queue->stop_writers)); // liberta o semaforo de escrita para se poder voltar a escrever pois a leitura ja foi toda feita
        }

        sem_post(&(dados->queue->mutex)); //liberta o semaforo da leitura

        sleep(random()%11);
    }

}
/*-----------------------------------------------------------------------*/
/*                  monitor()                                            */
/*-----------------------------------------------------------------------*/
int monitor(mem_structure *queue) {

    int aux;
    struct sigaction act;

    act.sa_handler = cleanup;
    act.sa_flags = 0;
    if ((sigemptyset(&act.sa_mask) == -1) ||
            (sigaction(SIGINT, &act, NULL) == -1))
        perror("Failed to set SIGINT to handle Ctrl-C.");

    while (1) {
        //1.6
        /*insert here code to make a wait operation on the stop_writers semaphore*/
        sem_wait(&(queue->stop_writers));

        aux = queue->tail == queue->head ? 0 :
              queue->tail  < queue->head ? queue->head - queue->tail :
              LIST_SIZE - (queue->tail - queue->head);
        fprintf(stderr, "\nThere are %d calls waiting.\n\n", aux);
        //1.6
        /*insert here code to make a signal operation on the stop_writers semaphore*/
        sem_post(&(queue->stop_writers));

        sleep(5);
    }
    return(0);
}





/*-----------------------------------------------------------------------*/
/*                  main()                                               */
/*-----------------------------------------------------------------------*/
int main(int argc, char *argv[]) {

    mem_structure *queue;

    /*insert here code to create the shared memory area*/

    //1.1

    shmfd = shm_open("/area", O_CREAT | O_RDWR, 00777);
    ftruncate(shmfd, sizeof( mem_structure));

    //1.2
    queue= mmap(NULL, sizeof(mem_structure), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    //1.3
    /*insert here code to create the semaphores*/
    sem_init(&(queue->mutex),1,1);
    //primeiro atributo é o endereço do semaphoro apontado por queue-> mutex, o segundo é a flag que vai decidir se este pode ser partilhado entre processos, e o terceiro é o valor inicial.
    sem_init(&(queue->stop_writers),1,1);


    pid_t pid;

    queue->tail = 0;
    queue->head = 0;
    queue->readers = 0;
    ParaFechar = queue;  //copia o ponteiro da queue para depois conseguir fechar na cleanup
    //1.4
    /*insert here code to create the "process of the writers" (POTW), and the writer threads within the POTW*/
    pid = fork();
    struct DadosArgumento TabelaArgumentos[NUM_WRITERS];
    if (pid  == 0) { //cria um filho para os writers
        srandom(getpid()); //faz uma nova seed para randomizar o tempo nas funcoes reader e writer

        for(int i=0; i<NUM_WRITERS; i++) {
            TabelaArgumentos[i].queue = queue;  //passa a queue que temos para a queue da tabela de argumentos, a ser passada à funcao write na thread
            TabelaArgumentos[i].NumeroReader = 1+i; //neste caso nao é reader, mas ficou declarado assim...
            pthread_create(&ThreadW[i], NULL, (void *)writer, (void *)&TabelaArgumentos[i]); //cria as threads de escrita

        }

        for(int k = 0; k < NUM_WRITERS; k++)
        {
            pthread_join(ThreadW[k], NULL); //espera que as threads de escrita terminem
        }



    }
    else {
        /*insert here code to create the "process of the readers" (POTR), and the reader threads within the POTR*/
        pid = fork(); //cria um filho para os readers

        if (pid  == 0) {
            srandom(getpid()); //faz uma nova seed para randomizar o tempo nas funcoes reader e writer

            for(int k=0; k<NUM_READERS; k++) {
                TabelaArgumentos[k].queue= queue; //passa a queue que temos para a tabela de argumentos, como feito em cima
                TabelaArgumentos[k].NumeroReader = 1+k; //aumenta o numero do reader atual
                pthread_create(&ThreadR[k], NULL, (void *)reader, (void *)&TabelaArgumentos); //cria as threads de leitura
            }
            for(int k = 0; k < NUM_READERS; k++)
            {
                pthread_join(ThreadR[k], NULL); //espera pelo fim das threads
            }


        }
        else {

            monitor(queue); //funcao que monitoriza quantas calls existem

        }
    }

    exit(0);
}
/*-----------------------------------------------------------------------*/


