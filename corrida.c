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

/*pista de largura 10*/
int *PISTA[10];

/*relogio global: acertar o nome das variaveis clock*/
int relogio = 0;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

/*gera um numero aleatorio entre 0 e 99*/
int lottery (int clock);
/* recebe uma seed e uma chance (0-99)
 e devolve 1 se o sorteio teve sucesso com a chance dada*/
int speed_lottery (int clock, int chance);

void *ciclista (void *arg);

/* www.ime.usp.br/~pf */
void *mallocX (unsigned int nbytes);
/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    pthread_t *threads;
    int i;
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
    int chance = 30;
    
    /*PISTA de comprimento d e largura 10*/
    for (i = 0; i < 10; i++)
         PISTA[i] = mallocX((d) * sizeof(int));

    /* argumentos */
    if (argc  == 4) {
        d = atoi (argv[1]);
        n = atoi (argv[2]);
        v = atoi (argv[3]);
        printf ("Pista tamanho: %d | n ciclistas: %d| v voltas: %d \n", d, n, v);
        /*exit(1);*/
    }
    else {
        if (argc == 5) printf ("debug\n");
        else {
            fprintf (stderr,"Erro de entrada!\n");
            exit (EXIT_FAILURE);
        }
    }

    /*condicoes da corrida*/
    if ( (d <= 249) || (n <= 5) || (n > 5*d) || (v%20 != 0)) {
        fprintf (stderr,"Erro nas condicoes de entrada\n");
        exit (EXIT_FAILURE);
    }
    
    /*criar vetor de n threads*/
    /*pthread_t  thread_ciclistas[n];*/
    threads = mallocX (n * sizeof(pthread_t));
    /*dispara os ciclistas (threads)*/
    for (i = 0; i < n; i++) {
        if ( pthread_create( &threads[i], NULL, ciclista, NULL) ) {
            printf("Erro ao criar thread.");
            abort ();
        }
    }

    /* SIMULADOR */
    /* esse while só vai ser usado para incrementar o relogio global*/
    while (1) {

        usleep (timestep);
        clock += 1;
        
        /*testando*/
        /*os sorteios vao pra dentro da funcao ciclista*/
        sucess = speed_lottery (clock, chance);
        if (sucess)
            speed = 90;
        else speed = 30;
        fprintf (stderr,"clock: | : %d \t loteria: | %d\t%d\t%d\n", clock, chance,sucess,speed);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *
 *  cada thread/ciclista controlara sua corrida dentro do loop
 *  a sincronizacao sera feita a cada dt = 120ms
 *
**/

void *ciclista(void *arg) {
    int id;
    int dt;
    id = 0;
    dt = 100000;
    while (1) {
        /*atualiza status e escreve em PISTA 
          e espera o proximo passo de tempo*/
        usleep (dt);
        pthread_mutex_lock ( &mutex1 );
        PISTA[0][1] = id;
        pthread_mutex_unlock ( &mutex1 );
    }
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
