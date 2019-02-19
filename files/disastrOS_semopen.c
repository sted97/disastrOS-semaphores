#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"
#include "disastrOS_constants.h"


void internal_semOpen(){
	int id = running->syscall_args[0]; //running è una struttura dati di tipo PCB (vedi pcb.h)
    
    int contatore = running->syscall_args[1];

    Semaphore* semaforo = SemaphoreList_byId(&semaphores_list, id); //(vedi disastrOS_semaphores.c )SemaphoreList_byId è una funzione che scandisce la lista dei semafori e restituisce il semaforo con id uguale a "id" se presente
			
																	 //inoltre semaphores_list è una variabile globale dichiarata in globals.h ed inizializzata in disastrOS.c
    if (semaforo==0) { //se non c'è
        semaforo = Semaphore_alloc(id, contatore); //lo alloco attraverso uno SLAB allocator(vedi disastrOS_semaphores.c)
        if(semaforo==0) printf("Impossibile allocare il semaforo/n");
        List_insert( &semaphores_list, semaphores_list.last, (ListItem*)semaforo); //lo inserisco nella lista dei semafori
    }

    SemDescriptor* desc = SemDescriptor_alloc(running->last_sem_fd, semaforo, running); //alloco un descrittore attraverso uno SLAB allocator (vedi disastrOS_descriptor.c)
    if(desc==0) printf("Impossibile allocare il descrittore/n");

    running->last_sem_fd++; //incremento il contatore dell'ultimo descrittore di semaforo

    List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) desc); //inserisco il descrittore nella lista dei descrittori

    SemDescriptorPtr* sem_desc_ptr = SemDescriptorPtr_alloc(desc); //puntatore al descrittore

    desc->ptr = sem_desc_ptr; //setto il puntatore bella struct del descrittore
    List_insert(&semaforo->descriptors, semaforo->descriptors.last, (ListItem*) sem_desc_ptr); //inserisco il puntatore nella lista dei puntatori dei descrittori

    running->syscall_retvalue = desc->fd; //imposto come valore di ritorno del processo running il fd(File Descriptor) di desc
    
    
    return;
}
