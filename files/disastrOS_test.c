#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS_constants.h"
#include "disastrOS.h"
#include <errno.h>
#include <stdlib.h>


#define ID_SEMAFORO_PRODUTTORI 1
#define ID_SEMAFORO_CONSUMATORI 2
#define ID_SEMAFORO_CONDIVISO 3
#define NUMERO_PROCESSI 7
#define ITERAZIONI 7

#define N 10 // numero di thread
#define M 2 // number di iterazioni per thread
#define V 1 // valore aggiunto da ogni thread ad ogni iterazione

unsigned long int shared_variable;
int n = N, m = M, v = V;
int num_processi=NUMERO_PROCESSI;
int iterazioni=ITERAZIONI;
int semaforo_condiviso;

#ifdef PC
void produttore(int semaforo_produttori, int semaforo_consumatori){
  int i;
  for (i = 0; i < iterazioni; i++){
    disastrOS_semWait(semaforo_produttori);
    printf("[PRODOTTO]\n");
    disastrOS_semPost(semaforo_consumatori);
  }
}


void consumatore(int semaforo_produttori, int semaforo_consumatori){
  int i;
  for (i = 0; i <iterazioni; i++){
    disastrOS_semWait(semaforo_consumatori);
    printf("[CONSUMATO]\n");
    disastrOS_semPost(semaforo_produttori);
  }
}


// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Ciao, sono lo sleeper e dormo %d\n\n", disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction(void* args){
  printf("Ciao, sono il figlio con PID %d\n", disastrOS_getpid());
  int type=0;
  int mode=0;
  int fd=disastrOS_openResource(disastrOS_getpid(), type, mode);
  printf("Apro risorsa con File Descriptor fd=%d\n", fd);

  printf("Apro i semafori\n\n");
  int semaforo_produttori=disastrOS_semOpen(ID_SEMAFORO_PRODUTTORI, NUMERO_PROCESSI); //apro il semaforo per i produttori con id 1 e contatore a 10 (numero di processi)
  int semaforo_consumatori=disastrOS_semOpen(ID_SEMAFORO_CONSUMATORI, 0); //apro il semaforo per i consumatori con id 2 e contatore a 0


  if (disastrOS_getpid() % 2 == 0){
    produttore(semaforo_produttori, semaforo_consumatori);
    printf("\n");
  }

  else{
    consumatore(semaforo_produttori, semaforo_consumatori);
    printf("\n");

  }


  disastrOS_printStatus();

  printf("Terminazione...\n");
  
  
  printf("Chiudo i semafori\n");
  disastrOS_semClose(semaforo_produttori);
  disastrOS_semClose(semaforo_consumatori);
  

  disastrOS_exit(disastrOS_getpid()); //figlio terminato
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("Ciao, Sono il processo Init! Ed ho appena iniziato :) \n");
  disastrOS_spawn(sleeperFunction, 0); //la spawn crea una nuova istanza del processo running e lo mette nella ready list eseguendo la funzione passata come primo parametro

  printf("Ora genero %d nuovi processi...\n\n", num_processi);
  int alive_children=0; //figli in vita inizialmente 0

  for (int i=0; i<num_processi; ++i) { //creo 10 processi
    int type=0;
    int mode=DSOS_CREATE;
    printf("Apro la risorsa %d con type=%d e mode=%d (e la creo se necessario)\n", i, type, mode);
    int fd=disastrOS_openResource(i,type,mode);
    printf("Aperta risorsa %d con file Descriptor fd=%d\n", i, fd);
    disastrOS_spawn(childFunction, 0); //eseguo la funzione childFunction
    printf("Creato figlio %d\n\n", i+1);
    alive_children++; //incremento il numero dei figli in vita
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ //finchè c'è almeno un processo in vita o il main sta attendendo la terminazione per un figlio
    printf("[INIT] Figlio %d terminato con valore di ritorno:%d, processi in vita: %d \n\n",
     pid, retval, alive_children);
    disastrOS_printStatus(); //stampo lo stato del sistema operativo

    alive_children--;  //decremento il numero di figli in vita
  }

  disastrOS_printStatus(); //stampo lo stato del sistema operativo
  printf("SPEGNIMENTO.\n");

  disastrOS_shutdown(); //termino il programma
}

int main(int argc, char** argv){
  if (argc > 1) num_processi = atoi(argv[1]);
  if (argc > 2) iterazioni = atoi(argv[2]);

  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue

  // spawn an init process
  printf("Si parte!!!\n");
  disastrOS_start(initFunction, 0, logfilename); //avvia il programma a partire dalla funzione initFunction
  return 0;
}
#endif

#ifdef ME
void childFunction(void* args){
    semaforo_condiviso=disastrOS_semOpen(ID_SEMAFORO_CONDIVISO, 1);

    int i;
    for (i = 0; i < m; i++) {
      disastrOS_semWait(semaforo_condiviso);;
      printf("[Processo %d] Faccio una semWait sul semaforo\n", disastrOS_getpid());
      disastrOS_printStatus();

      shared_variable += v;
      printf("[Processo %d] Sommo %d alla variabile condivisa\n\n", disastrOS_getpid(), v);
      printf("<SHARED VARIABLE = %lu>\n\n", shared_variable);


      disastrOS_semPost(semaforo_condiviso);
      printf("[Processo %d] Faccio una semPost sul semaforo\n", disastrOS_getpid());
      disastrOS_printStatus();
    }

  printf("Terminazione...\n");

  disastrOS_exit(disastrOS_getpid()); //figlio terminato
}


void initFunction(void* args) {
  printf("Ciao, Sono il processo Init! Ed ho appena iniziato :) \n");
  disastrOS_printStatus();

  printf("Ora genero %d nuovi processi...\n\n", n);
  int alive_children=0; //figli in vita inizialmente 0

  for (int i=0; i<n; ++i) { //creo 10 processi
    disastrOS_spawn(childFunction, 0); //eseguo la funzione childFunction
    printf("Creato figlio %d\n\n", i+1);
    alive_children++; //incremento il numero dei figli in vita
  }

  printf("<SHARED VARIABLE = %lu>\n", shared_variable);

  disastrOS_printStatus();

  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ //finchè c'è almeno un processo in vita o il main sta attendendo la terminazione per un figlio
    printf("[INIT] Figlio %d terminato con valore di ritorno:%d, processi in vita: %d \n\n",
    pid, retval, alive_children);
    disastrOS_printStatus();
 	  alive_children--;  //decremento il numero di figli in vita
  }

  unsigned long int expected_value = (unsigned long int)n*m*v;
  if(shared_variable==expected_value) printf("|ESITO DELLA COMPUTAZIONE: POSITIVO|\n");
  else printf("|ESITO DELLA COMPUTAZIONE: NEGATIVO|\n");
  
  printf("-Il valore della variabile condivisa è: %lu\n-Il valore atteso è:                    %lu\n\n", shared_variable, expected_value);

  printf("SPEGNIMENTO.\n");

  disastrOS_shutdown(); //termino il programma
}

int main(int argc, char** argv){
  if (argc > 1) n = atoi(argv[1]);
  if (argc > 2) m = atoi(argv[2]);
  if (argc > 3) v = atoi(argv[3]);

  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }

  shared_variable=0;
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue

  // spawn an init process
  printf("Si parte!!!\n");

  disastrOS_start(initFunction, 0, logfilename); //avvia il programma a partire dalla funzione initFunction
  disastrOS_semClose(semaforo_condiviso);

  return 0;
}
#endif

