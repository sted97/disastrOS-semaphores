#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

//SEMCLOSE:
//La syscall semClose(semnum) rilascia il semaforo con quello specifico semnum.
//In caso di successo ritorna 0;
//In caso di errore, cioè se il semaforo non è gestito dall'applicazione, ritorna uno specifico codice di errore.

void internal_semClose(){
	int id = running->syscall_args[0]; //mi prendo l'id
	
	SemDescriptor* desc = SemDescriptorList_byFd(&running-> sem_descriptors, id); //mi prendo il descrittore del semaforo a partire dall'id attraverso la funzione SemDescriptorList_byFd (vedi disastrOS_semdescriptor.c)

    GESTORE_ERRORI(desc, DSOS_ESEMCLOSE); //gestisco eventuali errori in caso la semClose fallisca

    List_detach(&running->sem_descriptors, (ListItem*)desc); //elimino l'elemento con descrittore 'desc' dalla lista 'sem_descriptors' del processo running attraverso la funzione List_detach (vedi linked_list.c)

    Semaphore* semaforo = desc->semaphore; //semaforo associato al descrittore
    SemDescriptorPtr* desc_ptr = (SemDescriptorPtr*) List_detach(&semaforo->descriptors, (ListItem*) desc->ptr); //puntatore al descrittore

    if(semaforo->descriptors.size == 0 && semaforo->waiting_descriptors.size == 0){ //quando le due liste 'descriptors' e 'waiting_descriptors':
        List_detach(&semaphores_list, (ListItem*)semaforo); //"stacco" il semaforo dalla lista dei semafori;
        Semaphore_free(semaforo); //rilascio il semaforo
    }


    SemDescriptor_free(desc); //rilascio il descrittore
    
    SemDescriptorPtr_free(desc_ptr); //rilascio il puntatore al descrittore

    running->syscall_retvalue = 0; //valore di ritorno nel caso in cui la chiusura del semaforo sia andata a buon fine
    
    return;
}
