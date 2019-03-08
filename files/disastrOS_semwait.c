#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

//SEMWAIT:
//La semWait(semnum) decrementa il valore del semaforo con numero semnum:
//se il valore del semaforo è minore di 0 il processo corrente viene bloccato
//Ritorna 0 in caso di successo e un valore negativo in caso di errore

void internal_semWait(){
    int fd = running->syscall_args[0]; //mi prendo l'id [running è una struttura dati di tipo PCB (vedi pcb.h)]


    SemDescriptor* desc = SemDescriptorList_byFd(&running->sem_descriptors, fd); //mi prendo il descrittore del semaforo a partire dall'id attraverso la funzione SemDescriptorList_byFd (vedi disastrOS_semdescriptor.c)
    GESTORE_ERRORI(desc, DSOS_ESEMWAIT); //gestisco eventuali errori in caso la semWait fallisca

    SemDescriptorPtr* desc_ptr = desc->ptr; //dichiaro un puntatore al descrittore e gli assegno come valore il campo ptr di desc

    Semaphore* semaforo = desc->semaphore; //dichiaro un semaforo e gli assegno come valore il campo semaphore di desc

    PCB* pcb; //dichiaro una struttura dati di tipo PCB
    semaforo->count--; //decremento il contatore del semaforo

    
    if (semaforo->count < 0){ //se il contatore è minore di 0
        List_detach(&semaforo->descriptors, (ListItem*)desc_ptr); //rimuovo il descrittore del processo dalla lista dei descrittori
        
        List_insert(&semaforo->waiting_descriptors, semaforo->waiting_descriptors.last, (ListItem*) desc->ptr); //inserisco il descrittore del processo nella lista di waiting
        running->status = Waiting; //imposto lo stato del processo a Waiting

        List_insert(&waiting_list, waiting_list.last, (ListItem*) running); //inserisco il processo in esecuzione nella coda di attesa
       
        pcb = (PCB*) List_detach(&ready_list, (ListItem*)ready_list.first); //prendo il primo elemento dalla ready list
        running = (PCB*)pcb; //mando in esecuzione il primo elemento della lista di ready appena preso
    }

    running->syscall_retvalue = 0; //valore di ritorno 0 in caso di successo
    

    return;

}
