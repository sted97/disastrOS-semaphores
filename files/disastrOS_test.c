#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS_constants.h"
#include "disastrOS.h"
#include <errno.h>
#include <stdlib.h>


#define ID_SEMAFORO_VUOTO 1
#define ID_SEMAFORO_PIENO 2
#define ID_SEMAFORO_CONDIVISO 3
#define NUMERO_PROCESSI 7
#define ITERAZIONI 10

#define N 10 // numero di thread
#define M 2 // number di iterazioni per thread
#define V 1 // valore aggiunto da ogni thread ad ogni iterazione

/* MACRO DEI COLORI */
#define COL(x) "\033[" #x ";1m"
#define COL_RED COL(31)
#define COL_GREEN COL(32)
#define COL_YELLOW COL(33)
#define COL_BLUE COL(34)
#define COL_VIOLET COL(35)
#define COL_WHITE COL(37)
#define COL_GRAY "\033[0m"


unsigned long int shared_variable;
int n = N, m = M, v = V;
int num_processi=NUMERO_PROCESSI;
int iterazioni=ITERAZIONI;
int semaforo_condiviso;

#ifdef PC
int buffer_size=32;
static int buf[32];
int in=0;
int out=0;
int semaforo_pieno;
int semaforo_vuoto;
int semaforo_cs;

void produttore(){
  printf("%s[PROCESSO %d] Ora divento un PRODUTTORE%s\n\n",COL_YELLOW, disastrOS_getpid(), COL_GRAY);

  int i, dato;
  for (i = 0; i < iterazioni; i++){
    dato=i;
    printf("%s[PROCESSO %d] Faccio una WAIT%s\n", COL_GREEN,disastrOS_getpid(), COL_GRAY);    
    disastrOS_semWait(semaforo_vuoto);
    disastrOS_semWait(semaforo_cs);
    
    buf[in]=dato;
    printf("%s[PROCESSO %d] Scritto %d nella posizione buf[%d]%s \n",COL_GREEN, disastrOS_getpid(), dato, in, COL_GRAY);
    in=(in+1)%buffer_size;
   
    printf("%s[PROCESSO %d] Faccio una POST\n\n%s", COL_GREEN, disastrOS_getpid(), COL_GRAY);    
    disastrOS_semPost(semaforo_cs);
    disastrOS_semPost(semaforo_pieno);
  }
  

}


void consumatore(){
  printf("%s[PROCESSO %d] Ora divento un CONSUMATORE%s\n\n",COL_YELLOW, disastrOS_getpid(), COL_GRAY);
  int i, dato;
  for (i = 0; i <iterazioni; i++){
    printf("%s[PROCESSO %d] Faccio una WAIT%s\n", COL_BLUE, disastrOS_getpid(), COL_GRAY);    
    disastrOS_semWait(semaforo_pieno);
    disastrOS_semWait(semaforo_cs);
    

    dato=buf[out];
    printf("%s[PROCESSO %d] Letto %d dalla posizione buf[%d]%s \n",COL_BLUE, disastrOS_getpid(), dato, out, COL_GRAY);
    out=(out+1)%buffer_size;
    
    printf("%s[PROCESSO %d] Faccio una POST%s\n\n",COL_BLUE, disastrOS_getpid(), COL_GRAY);    
    disastrOS_semPost(semaforo_cs);
    disastrOS_semPost(semaforo_vuoto);
  }

}


// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("%sCiao, sono lo sleeper e dormo %d%s\n\n",COL_YELLOW, disastrOS_getpid(), COL_GRAY);
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction(void* args){
  printf("%sCiao, sono il figlio con PID %d%s\n",COL_YELLOW, disastrOS_getpid(), COL_GRAY);

  printf("%s[PROCESSO %d] Apro i semafori%s\n\n",COL_YELLOW, disastrOS_getpid(), COL_GRAY);
  semaforo_vuoto=disastrOS_semOpen(ID_SEMAFORO_VUOTO, buffer_size); //apro il semaforo per i vuoto con id 1 e contatore a 10 (numero di processi)
  semaforo_pieno=disastrOS_semOpen(ID_SEMAFORO_PIENO, 0); //apro il semaforo per i pieno con id 2 e contatore a 0
  semaforo_cs=disastrOS_semOpen(ID_SEMAFORO_CONDIVISO, 1); //apro il semaforo per la critical section

  disastrOS_printStatus();

  if (disastrOS_getpid() % 2 == 0){
    produttore();
    printf("\n");
  }

  else{
    consumatore();
    printf("\n");

  }

  disastrOS_printStatus();


  printf("%s[PROCESSO %d] Terminazione...%s\n",COL_YELLOW, disastrOS_getpid(), COL_GRAY);
  
  
  printf("%s[PROCESSO %d] Chiudo i semafori%s\n",COL_YELLOW, disastrOS_getpid(), COL_GRAY);
  disastrOS_semClose(semaforo_vuoto);
  disastrOS_semClose(semaforo_pieno);
  disastrOS_semClose(semaforo_cs);

  disastrOS_exit(disastrOS_getpid()); //figlio terminato
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("%s[INIT] Ho appena iniziato :)%s \n\n", COL_RED, COL_GRAY);
  disastrOS_spawn(sleeperFunction, 0); //la spawn crea una nuova istanza del processo running e lo mette nella ready list eseguendo la funzione passata come primo parametro

  printf("%s>>>>>>>>>>>>>>>>>>>> STATO INIZIALE DEL BUFFER <<<<<<<<<<<<<<<<<<<<%s\n", COL_YELLOW, COL_GRAY);
  printf("%s[ %s", COL_YELLOW, COL_GRAY);
  for(int i=0; i<buffer_size; i++){
    printf("%s%d %s",COL_YELLOW, buf[i], COL_GRAY);
  }
  printf("%s]%s\n\n", COL_YELLOW, COL_GRAY);

  printf("%s[INIT] Ora genero %d nuovi processi...%s\n\n",COL_RED, num_processi, COL_GRAY);
  int alive_children=0; //figli in vita inizialmente 0

  for (int i=0; i<num_processi; ++i) { //creo 10 processi
    disastrOS_spawn(childFunction, 0); //eseguo la funzione childFunction
    printf("%s[INIT] Creato figlio %d%s\n\n",COL_RED, i+1, COL_GRAY);
    alive_children++; //incremento il numero dei figli in vita
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ //finchè c'è almeno un processo in vita o il main sta attendendo la terminazione per un figlio
    printf("%s[INIT] Figlio %d terminato con valore di ritorno:%d, processi in vita: %d%s \n\n",
    COL_RED, pid, retval, alive_children, COL_GRAY);
    disastrOS_printStatus(); //stampo lo stato del sistema operativo

    alive_children--;  //decremento il numero di figli in vita
  }

  printf("%s>>>>>>>>>>>>>>>>>>>>> STATO FINALE DEL BUFFER <<<<<<<<<<<<<<<<<<<<<%s\n", COL_YELLOW, COL_GRAY);
  printf("%s[ %s", COL_YELLOW, COL_GRAY);
  for(int i=0; i<buffer_size; i++){
    printf("%s%d %s",COL_YELLOW, buf[i], COL_GRAY);
  }
  printf("%s]%s\n\n", COL_YELLOW, COL_GRAY);

  printf("%sSPEGNIMENTO...%s\n", COL_RED, COL_GRAY);

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

  int ok=0;
  printf("\n%sHAI SCELTO LA VERSIONE 'PRODUTTORE-CONSUMATORE'%s\n\n", COL_WHITE, COL_GRAY);
  printf("\n%sISTRUZIONI PER LEGGERE AL MEGLIO IL PROGRAMMA:%s\n", COL_WHITE, COL_GRAY);
  printf("-I messaggi mandati dai PRODUTTORI sono   %sVERDI%s;\n",COL_GREEN, COL_GRAY);
  printf("-I messaggi mandati dai CONSUMATORI sono  %sBLU%s;\n",COL_BLUE, COL_GRAY);
  printf("-I messaggi mandati da INIT sono          %sROSSI%s;\n",COL_RED, COL_GRAY);
  printf("-I messaggi mandati da DISASTROS sono     %sBIANCHI%s;\n",COL_WHITE, COL_GRAY);
  printf("-I messaggi di tipo GENERALE sono         %sGIALLI%s;\n\n",COL_YELLOW, COL_GRAY);

  printf("Inserire '1' per eseguire il programma: ");
  scanf("%d", &ok);
  while(ok!=1);

  // spawn an init process
  printf("\n%sSi parte!!!%s\n", COL_YELLOW, COL_GRAY);
  disastrOS_start(initFunction, 0, logfilename); //avvia il programma a partire dalla funzione initFunction
  return 0;
}
#endif

#ifdef ME
void childFunction(void* args){
    printf("%s[Processo %d] Apro il semaforo%s\n",COL_GREEN, disastrOS_getpid(), COL_GRAY);
    semaforo_condiviso=disastrOS_semOpen(ID_SEMAFORO_CONDIVISO, 1);

    int i;
    for (i = 0; i < m; i++) {
      disastrOS_semWait(semaforo_condiviso);;
      printf("%s[Processo %d] Faccio una semWait sul semaforo%s\n",COL_GREEN, disastrOS_getpid(), COL_GRAY);
      disastrOS_printStatus();

      shared_variable += v;
      printf("%s[Processo %d] Sommo %d alla variabile condivisa%s\n\n",COL_GREEN, disastrOS_getpid(), v, COL_GRAY);
      printf("%s<SHARED VARIABLE = %lu>%s\n\n", COL_VIOLET, shared_variable, COL_GRAY);


      disastrOS_semPost(semaforo_condiviso);
      printf("%s[Processo %d] Faccio una semPost sul semaforo%s\n",COL_GREEN, disastrOS_getpid(), COL_GRAY);
      disastrOS_printStatus();
    }
  
  printf("%s[Processo %d] Chiudo il semaforo%s\n",COL_GREEN, disastrOS_getpid(), COL_GRAY);
  disastrOS_semClose(semaforo_condiviso);

  printf("%s[Processo %d] Terminazione...%s\n", COL_GREEN, disastrOS_getpid(), COL_GRAY);
  disastrOS_exit(disastrOS_getpid()); //figlio terminato
}


void initFunction(void* args) {
  printf("%s[INIT] Ho appena iniziato :)%s \n", COL_YELLOW, COL_GRAY);
  disastrOS_printStatus();

  printf("%s[INIT]Ora genero %d nuovi processi...%s\n\n",COL_YELLOW, n, COL_GRAY);
  int alive_children=0; //figli in vita inizialmente 0

  for (int i=0; i<n; ++i) { //creo 10 processi
    disastrOS_spawn(childFunction, 0); //eseguo la funzione childFunction
    printf("%s[INIT] Creato figlio %d\n\n%s",COL_YELLOW, i+1, COL_GRAY);
    alive_children++; //incremento il numero dei figli in vita
  }

  printf("%s<SHARED VARIABLE = %lu>%s\n", COL_VIOLET, shared_variable, COL_GRAY);

  disastrOS_printStatus();

  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ //finchè c'è almeno un processo in vita o il main sta attendendo la terminazione per un figlio
    printf("%s[INIT] Figlio %d terminato con valore di ritorno:%d, processi in vita: %d%s \n\n",
    COL_YELLOW, pid, retval, alive_children, COL_GRAY);
    disastrOS_printStatus();
 	  alive_children--;  //decremento il numero di figli in vita
  }

  unsigned long int expected_value = (unsigned long int)n*m*v;
  if(shared_variable==expected_value) printf("%s|ESITO DELLA COMPUTAZIONE: %sPOSITIVO%s|%s\n", COL_YELLOW, COL_WHITE, COL_YELLOW, COL_GRAY);
  else printf("%s|ESITO DELLA COMPUTAZIONE: %sNEGATIVO%s|%s\n", COL_YELLOW, COL_WHITE, COL_YELLOW, COL_GRAY);
  
  printf("%s-Il valore della variabile condivisa è:%s %lu\n%s-Il valore atteso è:%s                    %lu\n\n",COL_YELLOW, COL_WHITE, shared_variable, COL_YELLOW, COL_WHITE, expected_value);

  printf("%sSPEGNIMENTO...%s\n", COL_VIOLET, COL_GRAY);

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

  int ok=0;
  printf("\n%sHAI SCELTO LA VERSIONE 'MUTUA ESCLUSIONE'%s\n\n", COL_WHITE, COL_GRAY);
  printf("\n%sISTRUZIONI PER LEGGERE AL MEGLIO IL PROGRAMMA:%s\n", COL_WHITE, COL_GRAY);
  printf("-I messaggi mandati dai PROCESSI sono     %sVERDI%s;\n",COL_GREEN, COL_GRAY);
  printf("-I messaggi mandati da INIT sono          %sGIALLI%s;\n",COL_YELLOW, COL_GRAY);
  printf("-I messaggi mandati da DISASTROS sono     %sBIANCHI%s;\n",COL_WHITE, COL_GRAY);
  printf("-I messaggi di tipo GENERALE sono         %sVIOLA%s;\n\n",COL_VIOLET, COL_GRAY);

  printf("Inserire '1' per eseguire il programma: ");
  scanf("%d", &ok);
  while(ok!=1);
  printf("\n%sSi parte!!!\n%s", COL_VIOLET, COL_GRAY);

  disastrOS_start(initFunction, 0, logfilename); //avvia il programma a partire dalla funzione initFunction
  disastrOS_semClose(semaforo_condiviso);

  return 0;
}
#endif

