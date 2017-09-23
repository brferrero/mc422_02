/*
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX_STRING 32
#define EPSILON 0.0000000001

int cpu = 0;
char processo_atual[MAX_STRING];
int *PISTA;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread1;

/*gera um numero aleatorio entre 0 e 99*/
int lottery (int clock);
/* recebe uma seed e uma chance (0-99)
 e devolve 1 se o sorteio teve sucesso com a chance dada*/
int speed_lottery (int clock, int chance);

/* www.ime.usp.br/~pf */
void *mallocX (unsigned int nbytes);
/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    /*relógio arbitrário: milisegundo*/
    int clock = 0;

    /*timestep da simulacao: 100000 -> 1 ms*/
    int timestep = 100000;
    int sucess;
    
    /* d metros, n ciclistas e v voltas*/
    int d = 250;
    int n = 6;
    int v = 80;
    /* velocidade */
    int speed = 30;
    int chance = 1;

    PISTA = mallocX ((d)*sizeof(int));

    /* argumentos */
    if (argc  < 4) {
        printf ("\n");
        /*exit(1);*/
    }
    else {
        if (argc == 5) printf ("debug\n");
    }

    /*condicoes da corrida*/
    if ( (d <= 249) || (n <= 5) || (n > 5*d) || (v%20 != 0)) {
        fprintf (stderr,"Erro nas condicoes de entrada\n");
        exit(1);
    }


    /* SIMULADOR */
    while (1) {

        usleep (timestep);
        clock += 1;
        
        /*testando*/
        sucess = speed_lottery (clock, chance);
        if (sucess)
            speed = 90;
        else speed = 30;
        fprintf (stderr,"clock: | : %d \t loteria: | %d\t%d\t%d\n", clock, chance,sucess,speed);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void* processo (void* slep)
{
    double sleep = *((double*) slep);
    pthread_mutex_lock ( &mutex1 );
    cpu = 1;
    pthread_mutex_unlock ( &mutex1 );

    sleep = (int)sleep*1000000;
    usleep (sleep);

    pthread_mutex_lock( &mutex1 );
    cpu = 0;
    pthread_mutex_unlock( &mutex1 );
    return NULL;
}

/*gera aleatorios entre 0 e 99*/
int lottery (int clock)
{
    srand(clock+time(NULL));
    return rand()%100;
}

int speed_lottery (int clock, int chance)
{   
    int random;
    random = lottery (clock);
    if (random < chance)
        return 1;
    else return 0;
}

/* ~pf */
void *mallocX (unsigned int nbytes) 
{
    void *ptr;
    ptr = malloc (nbytes);
    if (ptr == NULL) {
	printf ("Socorro! malloc devolveu NULL!\n");
	exit (EXIT_FAILURE);
    }
    return ptr;
}
