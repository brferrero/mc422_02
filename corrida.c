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

typedef struct args {
    int id;
    int posicao;
    int v;
    int d;
} args_ciclistas;

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
    
    

    args_ciclistas arg[6];
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
        arg[i].id = i;
        arg[i].posicao = 0;
        arg[i].v = v;
        arg[i].d = d; 
        /* PASSAR v (voltas), id dos ciclistas, d (tamanho da pista), posicao na pista como argumento */
        if ( pthread_create( &threads[i], NULL, ciclista, (void*)&arg[i]) ) {
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

    /* FAZER JOIN NAS THREADS */

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
    int speed = 30;
    int volta = 0;
    int dt = 20;

    int id = 0;
    int tamanho = 250;
    int voltas = 80;
    int posicao = 0;

    args_ciclistas *parametros = arg;

    fprintf(stderr, "%d | %d | %d \n",parametros->id,parametros->posicao,parametros->v);

    /* FAZER A STRUCT CERTA */
    /*id = *arg;
    posicao = *arg;
    voltas = *arg;
    tamanho = *arg;
    */
    while (1) {
        /* BARREIRA de 60ms */
        if (relogio%(3*dt) == 0) {
            if (speed == 60 && (relogio%(3*dt) == 0)) printf("");/* ATUALIZA A PISTA 1m */
            if (speed == 30 && (relogio%(6*dt) == 0)) printf("");/* ATUALIZA A PISTA 1m */

            /* ULTRAPASSAGEM */


            /*pthread_barrier_wait(pthread_barrier_t *barrier);*/
        }

        else if ((voltas - volta <= 2) && relogio%dt == 0) {
            if (speed == 30 && relogio%(6*dt) == 0) printf("");/* ATUALIZA A PISTA 1m */
            if (speed == 60 && (relogio%(3*dt) == 0)) printf("");/* ATUALIZA A PISTA 1m */
            if (speed == 60 && (relogio%(2*dt) == 0)) printf("");/* ATUALIZA A PISTA 1m */
            

            /* ULTRAPASSAGEM */
            

            /*pthread_barrier_wait(pthread_barrier_t *barrier);*/
        }

        if (posicao == tamanho) {
            /*atualiza status e escreve em PISTA 
              e espera o proximo passo de tempo*/
            if (speed == 30)
                if (speed_lottery (relogio, 70)) speed = 60;
            
            else if (speed == 60)
                if (speed_lottery (relogio, 50)) speed = 30;
        

            /* VALIDO APENAS PARA 1 CICLISTA !!! */
            if(voltas - volta <= 2)
                if (speed_lottery (relogio, 10)) speed = 90;

            volta++;
            posicao = 0;
        }

        if(volta%15 == 0) /* PODE QUEBRAR */

        if(volta == voltas)
            break;

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
