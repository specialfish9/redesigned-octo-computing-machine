#include "asl.h"
//DESCRIZIONI SOMMARIE DELLE FUNZIONI, PER QUELLE COMPLETE VEDERE FILE asl.h

int insertBlocked(int *semAdd,pcb_t *p){    //p punta al PCB dalla coda dei processi bloccati, semAdd è la chiave per il SEMD associato a quella coda. Return TRUE se non è possibile allocare un SEMD perchè la lista dei liberi è vuota, FALSE altrimenti

}

pcb_t* removeBlocked(int *semAdd){  //rimuovere il primo PCB nella lista dei bloccati associati al SEMD tramite semAdd. Return del PCB, altrimenti null se non esiste

}

pcb_t* outBlocked(pcb_t *p){    //rimuovere PCB puntato da p->p_semAdd dalla coda del semaforo su cui è bloccato. Return null se il PCB non compare, p altrimenti

}

pcb_t* headBlocked(int *semAdd){    //restituire senza rimuovere il puntatore del PCB in testa alla coda dei processi del SEMD identificato da semAdd, null se il SEMD non compare o se compare ma la sua lista dei processi è vuota

}

void initASL(){ //inizializzare lista di semdFree per contenere tutti gli elementi di semdTable

}