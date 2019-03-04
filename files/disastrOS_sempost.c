#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

//SEMPOST:
//La syscall semPost(semnum) sblocca il semaforo con numero "semnum", cioè:
//-Incrementa di 1 il valore del contatore del semaforo
//-Se il valore diventa > 0  qualche altro thread bloccato sulla sem_wait puo' continuare l'esecuzione
//Restituisce 0 in caso di successo e -1 in caso di errore

void internal_semPost(){
    int id = running->syscall_args[0]; //mi prendo l'id [running è una struttura dati di tipo PCB (vedi pcb.h)]

    SemDescriptor* desc = SemDescriptorList_byFd(&running->sem_descriptors, id); //mi prendo il descrittore del semaforo a partire dall'id attraverso la funzione SemDescriptorList_byFd (vedi disastrOS_semdescriptor.c)

    Semaphore* semaforo = desc->semaphore; //semaforo associato al descrittore

    SemDescriptorPtr* desc_ptr; //puntatore al descrittore
    
    semaforo->count++; //incremento il contatore del semaforo

    
    if (semaforo->count <= 0) { //se il contatore del semaforo ha valore minore o uguale a zero
        List_insert(&ready_list, ready_list.last, (ListItem*) running); //aggiungo il processo running nella ready list che è una lista di tipo ListHead dichiarata in disastrOs_globals.h
        
        desc_ptr = (SemDescriptorPtr*) List_detach(&semaforo->waiting_descriptors, (ListItem*) semaforo->waiting_descriptors.first); //rimuovo il primo descrittore del semaforo dalla lista dei descrittori in attesa
        List_insert(&semaforo->descriptors, semaforo->descriptors.last, (ListItem*) desc_ptr);
        List_detach(&waiting_list, (ListItem*) desc_ptr->descriptor->pcb); //rimuovo il processo corrispondente a desc_ptr dalla lista di attesa
        
        running->status = Ready; //metto il processo running nello stato ready
        running = desc_ptr->descriptor->pcb; //metto in esecuzione il processo appena levato dalla waiting_list
    }

    running->syscall_retvalue = 0; //valore di ritorno 0 in caso di successo
   
    return;


}
