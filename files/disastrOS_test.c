#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS_constants.h"
#include "disastrOS.h"


#define ID_SEMAFORO_PRODUTTORI 1
#define ID_SEMAFORO_CONSUMATORI 2
#define NUMERO_PROCESSI 10

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

  printf("Apro i semafori\n");
  disastrOS_semOpen(ID_SEMAFORO_PRODUTTORI, NUMERO_PROCESSI); //apro il semaforo per i produttori con id 1 e contatore a 10 (numero di processi)
  disastrOS_semOpen(ID_SEMAFORO_CONSUMATORI, 0); //apro il semaforo per i consumatori con id 2 e contatore a 0
  
  printf("Terminazione...\n");
  disastrOS_exit(disastrOS_getpid()+1); //figlio terminato
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("Ciao, Sono il processo Init! Ed ho appena iniziato :) \n");
  disastrOS_spawn(sleeperFunction, 0); //la spawn crea una nuova istanza del processo running e lo mette nella ready list eseguendo la funzione passata come primo parametro
  printf("Ora genero 10 nuovi processi...\n\n");
  int alive_children=0; //figli in vita inizialmente 0

  for (int i=0; i<10; ++i) { //creo 10 processi
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
	disastrOS_printStatus(); //stampo lo stato del sistema operativo
    printf("[INIT] Figlio %d terminato con valore di ritorno:%d, processi in vita: %d \n\n",
	   pid, retval, alive_children);
	alive_children--;  //decremento il numero di figli in vita
  }
	  
  disastrOS_printStatus(); //stampo lo stato del sistema operativo
  printf("SPEGNIMENTO.\n");
  disastrOS_shutdown(); //termino il programma
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("Il puntatore a funzione è: %p\n", childFunction); //puntatore alla funzione childFunction
  // spawn an init process
  printf("Si parte!!!\n");
  disastrOS_start(initFunction, 0, logfilename); //avvia il programma a partire dalla funzione initFunction
  return 0;
}
