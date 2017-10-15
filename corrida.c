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
    int rank;
    int clock;
    int pontos;
} parametros_ciclistas;

/*pista de largura 10*/
int *PISTA[LARGURA];
int *quebrados;
/*guarda o id dos ciclistas por ordem de chegada*/
int *ranking_tempo;
int *ranking_pontos;
/**/
int **ranking_voltas;

/*vetor usado para controlar os 4 ciclistas que vao pontuar %10 volta*/
int *pontuacao;
int volta_pontuada = 0;

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
void clear_pontuacao (int k);
void clear_ranking_voltas (int v, int n);
void print_ranking_voltas (int v, int n);

/*imprime status da PISTA*/
void print_pista (int d);

/*recebe a colocacao do ciclista na volta e devolve o numero de pontos ganhado na volta*/
int pontua_sprint (int colocacao);

/* www.ime.usp.br/~pf */
void *mallocX (unsigned int nbytes);
/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    pthread_t *threads;
    int i;
    /*timestep da simulacao: 100000 -> 1 ms*/
    int timestep = 100;

    /* d metros, n ciclistas e v voltas*/
    int d = 250;
    int v = 80;
    
    /*paramentros dos ciclistas*/
    parametros_ciclistas *arg;
    
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
    n = 8;
    v = 60;
    d = 10;
    ciclistas = n;
    
    /* parametros de cada ciclista */
    arg = mallocX (n * sizeof (parametros_ciclistas));

    /* Ciclistas quebrados */
    quebrados = mallocX((n - 5) * sizeof(int));
    
    /* ranking por ordem de chegada */
    ranking_tempo = mallocX ((n) * sizeof(int));
    ranking_pontos = mallocX ((n) * sizeof(int));
    pontuacao = mallocX ((n) * sizeof(int));
    ranking_voltas = (int **) malloc (v*sizeof(int*));
    for (i = 0; i < v; i++) 
        ranking_voltas[i] = mallocX (n *sizeof(int));

    /*prepara a pista para a corrida*/
    for (i = 0; i < LARGURA; i++)
        PISTA[i] = mallocX((d) * sizeof(int));
    
    /*zerando pista*/
    clear_pista (d);
    clear_pontuacao (n);
    clear_ranking_voltas (v, n);

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
        arg[i].rank = -1;
        arg[i].clock = -1;
        arg[i].pontos = 0;
        if ( pthread_create( &threads[i], NULL, ciclista, (void*)&arg[i]) ) {
            fprintf(stderr,"Erro ao criar thread.");
            abort ();
        }
        ranking_tempo[i] = -1;
        ranking_pontos[i] = -1;
    }

    /* SIMULADOR: "cronometro" */
    while (1) {
        usleep (timestep);
        relogio_global += 1;
        /*TODOS OS CICLISTAS TERMINARAM A CORRIDA*/
        if (finished == n) 
            break;
    }
    /*
    for (i = 0; i < n; i++){
        pthread_join(threads[i], NULL);
        printf("ID %2d: | rank: %2d | clock: %6d | pontos: %3d\n",arg[i].id,arg[i].rank,arg[i].clock, arg[i].pontos);
    }
    */
    print_ranking_voltas (v, n);

    /*FREE*/
    pthread_barrier_destroy (&barreira_ciclo);
    for (i = 0; i < LARGURA; i++)
        free (PISTA[i]);
    
    for (i = 0; i < v; i++)
        free (ranking_voltas[i]);
    free(threads);
    free(arg);
    free(ranking_tempo);
    free(ranking_pontos);

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
    int tamanho_pista = parametros->d;
    int voltas = parametros->v;
    int posicao = parametros->posicao;
    int raia;

    /* COLOCA OS CICLISTAS NA PISTA */
    pthread_mutex_lock ( &mutex1 );
    for (j = 0; j < tamanho_pista && empty == 0; j++)
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
            /* CICLISTA VAI ANDAR */
            else {
                distancia++;
                dx = 0;
                /* SECAO CRITICA! */
                pthread_mutex_lock ( &mutex1 );
                if (PISTA[raia][(posicao+1)%tamanho_pista] == -1) {
                    PISTA[raia][posicao%tamanho_pista] = -1;
                    posicao++;
                    PISTA[raia][posicao%tamanho_pista] = id;
                }
                /* Verifica ultrapassagem */
                else {
                    if (speed == 60 && PISTA[raia][(posicao+1)%tamanho_pista] != -1) {
                        /* ultrapassou */
                        printf("\nPISTA[%d][%d] = %d\n",raia,posicao+1,id);
                        for (i = raia; i < 10; i++) {
                            if (PISTA[i][(posicao+1)%tamanho_pista] == -1) {
                                PISTA[raia][posicao%tamanho_pista] = -1;
                                posicao++;
                                PISTA[i][posicao%tamanho_pista] = id;
                                raia = i;
                                break;
                            }
                        }
                        /* Se nao conseguiu ultrapassar volta a 30km/h */
                        if (i == 10) speed = 30;
                    }
                }
                pthread_mutex_unlock ( &mutex1 );
                /* END SECAO CRITICA*/
            }
            pthread_barrier_wait(&barreira_ciclo);
            /*printf ("[ID %2d] relogio: %d \t| d: %4dm | lap: %3d | s: %2dkm/h | pontos : %d\n",id,relogio_global,distancia,volta,speed, parametros->pontos);*/
        }
        /* BARREIRA 20ms; e se alguem teve sucesso com 90km/h */
        else 
            if ((clock%dt == 0) && (clock != ultima_barreira) && (voltas - volta) <= 2) {
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
                    if (PISTA[raia][(posicao+1)%tamanho_pista] == -1) {
                        PISTA[raia][posicao%tamanho_pista] = -1;
                        posicao++;
                        PISTA[raia][posicao%tamanho_pista] = id;
                    }
                    /* Verifica ultrapassagem */
                    else {
                        if (speed == 60 && PISTA[raia][(posicao+1)%tamanho_pista] != -1) {
                            /* ultrapassou */
                            printf("\nPISTA[%d][%d] = %d\n",raia,posicao+1,id);
                            for (i = raia; i < 10; i++) {
                                if (PISTA[i][(posicao+1)%tamanho_pista] == -1) {
                                    PISTA[raia][posicao%tamanho_pista] = -1;
                                    posicao++;
                                    PISTA[i][posicao%tamanho_pista] = id;
                                    raia = i;
                                    break;
                                }
                            }
                            /* Se nao conseguiu ultrapassar volta a 30km/h */
                            if (i == 10) speed = 30;
                        }
                    }
                    pthread_mutex_unlock ( &mutex1 );
                    /* END SECAO CRITICA*/
                }
                pthread_barrier_wait(&barreira_ciclo);
                /*printf ("[ID %2d] relogio: %d \t| d: %4dm | lap: %3d | s: %2dkm/h | pontos : %d\n",id,relogio_global,distancia,volta,speed, parametros->pontos);*/
            }
        
        /* COMPLETOU UMA VOLTA */
        if (distancia == tamanho_pista) {
            distancia = 0;
            /*registra na matriz a classificacao da volta*/
            pthread_mutex_lock ( &mutex1 );
            for (i = 0; i < n; i++) {
                if (ranking_voltas[volta][i] == -1) {
                    ranking_voltas[volta][i] = id;
                    /*printf ("Ciclista %2d completou a volta %3d\n",id,volta);*/
                    break;
                }
            }
            pthread_mutex_unlock ( &mutex1 );
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
                fprintf(stderr,"\nCiclista [%d] quebrou na volta %d\n",id, volta);
                while(1) {
                    pthread_barrier_wait(&barreira_ciclo);
                    if (finished == n) return NULL;
                }
            }
        }
        /* ATUALIZA PONTUACAO */
        /* acho que nao vamos precisar desse if - da pra pontuar vendo a matrix ranking_voltas*/
        if (volta%10 == 0 && volta > 0) {
            pthread_mutex_lock ( &mutex1 );
            for (i = 0; i < n && pontuacao[i] != id; i++) {
                if (pontuacao[i] == -1) {
                    pontuacao[i] = id;
                    parametros->pontos += pontua_sprint(i);
                    break;
                }
            }
            /*volta foi pontuada*/
            if (i == n)
            {
                clear_pontuacao (n);
            }
            pthread_mutex_unlock ( &mutex1 );
        }

        /* TERMINOU A CORRIDA */
        if (volta == voltas) {
            pthread_mutex_lock ( &mutex1 );
            finished++;
            for (i = 0; i < n; i++){
                if (ranking_tempo[i] == -1) {
                    parametros->rank = i;
                    parametros->clock = clock;
                    ranking_tempo[i] = id;
                    break;
                    }
            }
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

/**/
void clear_pontuacao (int k)
{
    int i;
    for (i = 0; i < k; i++)
        pontuacao[i] = -1;
}

void clear_ranking_voltas (int v, int n)
{
    int i,j;
    for (i = 0; i < v; i++) 
        for (j = 0; j < n; j++) 
            ranking_voltas[i][j] = -1;
}

void print_ranking_voltas (int v, int n)
{
    int i,j;
    printf ("Classificacao por volta:");
    for (i = 0; i < v; i++) {
        printf ("\nVolta %3d: ", i+1);
        for (j = 0; j < n; j++)
            printf ("%2d | ", ranking_voltas[i][j]);
    }
    printf ("\n");
}

void print_pista(int d)
{
    int i,j;
    for (i = 0; i < LARGURA; i++) {
        for (j = 0; j < d; j++)
            if (PISTA[i][j] == -1)
                printf (".");
            else printf("%d",PISTA[j][j]);
        printf("\n");
    }
}

/**/
int pontua_sprint (int colocacao)
{
    int pontos;
    switch (colocacao) {
        case 0: pontos = 5; break;
        case 1: pontos = 3; break;
        case 2: pontos = 2; break;
        case 3: pontos = 1; break;
        default : pontos = 0;
    }
    return pontos;
}

/* ~pf */
void *mallocX (unsigned int nbytes) 
{
    void *ptr;
    ptr = malloc (nbytes);
    if (ptr == NULL) {
        fprintf (stderr,"Socorro! malloc devolveu NULL!\n");
        abort ();
    }
    return ptr;
}
