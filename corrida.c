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

/*largura da pista: 10 raias*/
#define LARGURA 10

typedef struct param_ciclistas {
    int id;
    int posicao;
    int v;
    int d;
} parametros_ciclistas;

/*pista de largura 10*/
int *PISTA[LARGURA];
int *quebrados;

int relogio_global = 0;
int finished = 0;
int lottery_90 = 0;
int ciclistas;
int n;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barreira_ciclo;

/*gera um numero aleatorio entre 0 e 99*/
int lottery (int semente);
/* recebe uma seed e uma chance (0-99)
   e devolve 1 se o sorteio teve sucesso com a chance dada*/
int speed_lottery (int semente, int chance);

void *ciclista (void *arg);

/* zera matriz PISTA */ 
void clear_pista (int d);

/* www.ime.usp.br/~pf */
void *mallocX (unsigned int nbytes);
/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    pthread_t *threads;
    int i;
    /*timestep da simulacao: 100000 -> 1 ms*/
    int timestep = 10000;

    /* d metros, n ciclistas e v voltas*/
    int d = 250;
    int v = 80;
    
    /*paramentros dos ciclistas*/
    parametros_ciclistas *arg;
    
    /* testes */
    /*int speed = 30;
    int chance = 30;
    int success;*/

    /* checando argumentos de entrada */
    if (argc  == 4) {
        d = atoi (argv[1]);
        n = atoi (argv[2]);
        v = atoi (argv[3]);
        printf ("Pista tamanho: %d | n ciclistas: %d| v voltas: %d \n", d, n, v);
    }
    else {
        if (argc == 5) printf ("modo debug\n");
        else {
            fprintf (stderr,"Erro de entrada!\n");
            exit (EXIT_FAILURE);
        }
    }

    /* checando condicoes da corrida*/
    if ( (d <= 249) || (n <= 5) || (n > 5*d) || (v%20 != 0)) {
        fprintf (stderr,"Erro nas condicoes de entrada\n");
        exit (EXIT_FAILURE);
    }

    /*teste*/
    n = 15;
    v = 3;
    d = 10;
    ciclistas = n;
    
    /* parametros de cada ciclista */
    arg = mallocX (n * sizeof (parametros_ciclistas));

    /* Ciclistas quebrados */
    quebrados = mallocX((n - 5) * sizeof(int));

    /*prepara a pista para a corrida*/
    for (i = 0; i < LARGURA; i++)
        PISTA[i] = mallocX((d) * sizeof(int));

    /*zerando pista*/
    clear_pista (d);

    /* Inicializa a barreira */
    pthread_barrier_init (&barreira_ciclo, NULL, n);

    /*vetor de n threads*/
    threads = mallocX (n * sizeof(pthread_t));
    
    /*dispara os ciclistas (threads)*/
    for (i = 0; i < n; i++) {
        arg[i].id = i+1;
        arg[i].posicao = 0;
        arg[i].v = v;
        arg[i].d = d; 
        if ( pthread_create( &threads[i], NULL, ciclista, (void*)&arg[i]) ) {
            printf("Erro ao criar thread.");
            abort ();
        }
    }

    /* SIMULADOR: "cronometro" */
    while (1) {
        usleep (timestep);
        relogio_global += 1;
        /*TODOS OS CICLISTAS TERMINARAM A CORRIDA*/
        if (finished == n) 
            break;
    }

    /**/
    for (i = 0; i < n; i++)
        pthread_join(threads[i], NULL);

    /*FREE*/
    pthread_barrier_destroy (&barreira_ciclo);
    for (i = 0; i < LARGURA; i++)
        free (PISTA[i]);
    free(threads);
    free(arg);
    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *
 *  cada thread/ciclista controlara sua corrida dentro do loop
 *  a sincronizacao eh feita a cada ...
 *
 **/

void *ciclista(void *arg) {
    
    int i, j, clock;
    int ultima_barreira = 0;
    int speed = 30;
    int volta = 0;
    int distancia = 0;

    int dt = 20;
    int dx = 0;
    int empty = 0;

    parametros_ciclistas *parametros = arg;
    int id = parametros->id;
    int tamanho = parametros->d;
    int voltas = parametros->v;
    int posicao = parametros->posicao;
    int raia;

    /* COLOCA OS CICLISTAS NA PISTA */
    pthread_mutex_lock ( &mutex1 );
    for (j = 0; j < tamanho && empty == 0; j++)
        for (i = 0; i < LARGURA && empty == 0; i++) {
            if (PISTA[i][j] == -1) {
                raia = i;
                posicao = j;
                empty = 1;
            }
        }
    /* Primeiro slot livre */
    PISTA[raia][posicao] = id;
    printf ("pista[%d][%d] = %d\n",raia,posicao,PISTA[raia][posicao]);
    pthread_mutex_unlock ( &mutex1 );
    pthread_barrier_wait(&barreira_ciclo);
    
    /* Espera a LARGADA e comeca a corrida*/
    while (relogio_global == 0) continue;
    while (1) {
        clock = relogio_global;

        /* BARREIRA de 60ms */
        if ((clock%(3*dt) == 0) && (clock != ultima_barreira) && (voltas - volta) > 2) {
            ultima_barreira = clock;
            
            if (speed == 30 && (dx < 1)) dx++;

            /* Ciclista ANDA */
            else {
                distancia++;
                dx = 0;

                /* SECAO CRITICA! */
                pthread_mutex_lock ( &mutex1 );
                if (PISTA[raia][(posicao+1)%tamanho] == -1) {
                    PISTA[raia][posicao%tamanho] = -1;
                    posicao++;
                    PISTA[raia][posicao%tamanho] = id;
                }

                /* Verifica ultrapassagem */
                else if (speed == 60 && PISTA[raia][(posicao+1)%tamanho] != -1) {
                    /* ultrapassou */
                    fprintf(stderr, "\nPISTA[%d][%d] = %d\n",raia,posicao+1,id);
                    for (i = raia; i < 10; i++) {
                        if (PISTA[i][(posicao+1)%tamanho] == -1) {
                            PISTA[raia][posicao%tamanho] = -1;
                            posicao++;
                            PISTA[i][posicao%tamanho] = id;
                            raia = i;
                            break;
                        }
                    }

                    /* Se nao conseguiu ultrapassar volta a 30km/h */
                    if (i == 10) speed = 30;
                }
                pthread_mutex_unlock ( &mutex1 );
                /* END SECAO CRITICA*/
            }
            pthread_barrier_wait(&barreira_ciclo);

            fprintf (stderr,"[ID %d] relogio: %d | d: %dm | lap: %d | s: %dkm/h \n",id,relogio_global,distancia,volta,speed);
        }

        /* BARREIRA 20ms */
        else if ((clock%dt == 0) && (clock != ultima_barreira) && (voltas - volta) <= 2) {
            ultima_barreira = clock;
            
            if (speed == 30 && (dx < 5)) dx++;
            else if (speed == 60 && (dx < 2)) dx++;
            else if (speed == 90 && (dx < 1)) dx++;

            /* Ciclista ANDA */
            else {
                distancia++;
                dx = 0;

                /* SECAO CRITICA! */
                pthread_mutex_lock ( &mutex1 );
                if (PISTA[raia][(posicao+1)%tamanho] == -1) {
                    PISTA[raia][posicao%tamanho] = -1;
                    posicao++;
                    PISTA[raia][posicao%tamanho] = id;
                }

                /* Verifica ultrapassagem */
                else if (speed == 60 && PISTA[raia][(posicao+1)%tamanho] != -1) {
                    /* ultrapassou */
                    fprintf(stderr, "\nPISTA[%d][%d] = %d\n",raia,posicao+1,id);
                    for (i = raia; i < 10; i++) {
                        if (PISTA[i][(posicao+1)%tamanho] == -1) {
                            PISTA[raia][posicao%tamanho] = -1;
                            posicao++;
                            PISTA[i][posicao%tamanho] = id;
                            raia = i;
                            break;
                        }
                    }

                    /* Se nao conseguiu ultrapassar volta a 30km/h */
                    if (i == 10) speed = 30;
                }
                pthread_mutex_unlock ( &mutex1 );
                /* END SECAO CRITICA*/
            }
            pthread_barrier_wait(&barreira_ciclo);

            fprintf (stderr,"[ID %d] relogio: %d | d: %dm | lap: %d | s: %dkm/h \n",id,relogio_global,distancia,volta,speed);
        }

        /* Completou uma volta */
        if (distancia == tamanho) {
            distancia = 0;
            volta++;

            /* Sorteia velocidade 90 */
            if ((voltas - volta) <= 2 && lottery_90 == 0 && speed_lottery(id, 10)) {
                speed = 90;
                pthread_mutex_lock ( &mutex1 );
                lottery_90 = 1;
                pthread_mutex_unlock ( &mutex1 );
            }

            /* Sorteia nova velocidade */
            if (speed == 30 && speed_lottery(id, 70)) speed = 60;
            else if (speed == 60 && speed_lottery(id, 50)) speed = 30;

            /* verifica chance de quebrar */
            if (volta%15 == 0 && ciclistas > 5 && speed_lottery(id, 1)) {
                finished++;
                pthread_mutex_lock ( &mutex1 );
                quebrados[n - ciclistas] = id;
                ciclistas--;
                pthread_mutex_unlock ( &mutex1 );
                printf("\nCiclista [%d] quebrou\n",id);
                while(1) {
                    pthread_barrier_wait(&barreira_ciclo);
                    if (finished == n) return NULL;
                }
            }
        }

     /*       
            if (speed == 60 && (relogio_global%(3*dt) == 0) && PISTA[raia][posicao+1] == 0) {
                PISTA[raia][posicao] = 0;
                PISTA[raia][++posicao] = id;
            }
            if (speed == 30 && (relogio_global%(6*dt) == 0) && PISTA[raia][posicao+1] == 0) {
                PISTA[raia][posicao] = 0;
                PISTA[raia][++posicao] = id;
            }
            posicao++;
            printf ("id %d: Esperando em 60 : relogio_global: %d volta : %d\n", id, relogio_global,volta);

            pthread_barrier_wait(&barreira_ciclo);
            if (posicao == tamanho) { volta++;posicao=0;}
            if (volta >= voltas) break;
        }
    */
        /* BARREIRA de 20ms */
        /*
        else if ((voltas - volta <= 2) && relogio_global%dt == 0) {
            if (speed == 30 && (relogio_global%(6*dt) == 0) && PISTA[raia][posicao+1] == 0) {
                PISTA[raia][posicao] = 0;
                PISTA[raia][++posicao] = id;
            }
            if (speed == 60 && (relogio_global%(3*dt) == 0) && PISTA[raia][posicao+1] == 0) {
                PISTA[raia][posicao] = 0;
                PISTA[raia][++posicao] = id;
            }
            if (speed == 60 && (relogio_global%(2*dt) == 0) && PISTA[raia][posicao+1] == 0) {
                PISTA[raia][posicao] = 0;
                PISTA[raia][++posicao] = id;
            }

            printf ("id %d: Esperando em 20 : relogio_global: %d volta: %d\n", id, relogio_global, volta);
            pthread_barrier_wait(&barreira_ciclo);
        }
        */
        
        /*
        if (posicao == tamanho) {
            if (speed == 30) {
                if (speed_lottery (relogio_global, 70)) speed = 60;
            }
            else if (speed == 60) {
                if (speed_lottery (relogio_global, 50)) speed = 30;
            }

             //VALIDO APENAS PARA 1 CICLISTA !!!
            if (voltas - volta <= 2)
                if (speed_lottery (relogio_global, 10)) speed = 90;
            
            volta++;
            printf ("id %d: Esperando em ultima volta : relogio_global: %d\n", id, relogio_global);
            posicao = 0;
            if (volta >= voltas) break;
        }
        */
        /*
        if(volta%15 == 0) break;
        */

        /*terminou a corrida*/
        if (volta == voltas) {
            pthread_mutex_lock ( &mutex1 );
            finished++;
            pthread_mutex_unlock ( &mutex1 );

            /* Aguarda outros ciclistas terminarem */
            while(1) {
                pthread_barrier_wait(&barreira_ciclo);
                if (finished == n) return NULL;
            }
        }
    }
    return NULL;
}

/*gera aleatorios entre 0 e 99*/
int lottery (int semente)
{
    srand(semente+time(NULL));
    return rand()%100;
}

int speed_lottery (int semente, int chance)
{   
    int random;
    random = lottery (semente);
    if (random < chance)
        return 1;
    else return 0;
}

/* esvazia a PISTA*/
void clear_pista (int d)
{ 
    int i,j;
    for (i = 0; i < LARGURA; i++)
        for (j = 0; j < d; j++)
            PISTA[i][j] = -1;
}

/* ~pf */
void *mallocX (unsigned int nbytes) 
{
    void *ptr;
    ptr = malloc (nbytes);
    if (ptr == NULL) {
        printf ("Socorro! malloc devolveu NULL!\n");
        abort ();
    }
    return ptr;
}
